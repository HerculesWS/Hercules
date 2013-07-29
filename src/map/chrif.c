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

static int check_connect_char_server(int tid, unsigned int tick, int id, intptr_t data);

static struct eri *auth_db_ers; //For reutilizing player login structures.
static DBMap* auth_db; // int id -> struct auth_node*

static const int packet_len_table[0x3d] = { // U - used, F - free
	60, 3,-1,27,10,-1, 6,-1,	// 2af8-2aff: U->2af8, U->2af9, U->2afa, U->2afb, U->2afc, U->2afd, U->2afe, U->2aff
	 6,-1,18, 7,-1,39,30, 10,	// 2b00-2b07: U->2b00, U->2b01, U->2b02, U->2b03, U->2b04, U->2b05, U->2b06, U->2b07
	 6,30, -1, 0,86, 7,44,34,	// 2b08-2b0f: U->2b08, U->2b09, U->2b0a, F->2b0b, U->2b0c, U->2b0d, U->2b0e, U->2b0f
	11,10,10, 0,11, 0,266,10,	// 2b10-2b17: U->2b10, U->2b11, U->2b12, F->2b13, U->2b14, F->2b15, U->2b16, U->2b17
	 2,10, 2,-1,-1,-1, 2, 7,	// 2b18-2b1f: U->2b18, U->2b19, U->2b1a, U->2b1b, U->2b1c, U->2b1d, U->2b1e, U->2b1f
	-1,10, 8, 2, 2,14,19,19,	// 2b20-2b27: U->2b20, U->2b21, U->2b22, U->2b23, U->2b24, U->2b25, U->2b26, U->2b27
};

struct chrif_interface chrif_s;

//Used Packets:
//2af8: Outgoing, chrif_connect -> 'connect to charserver / auth @ charserver'
//2af9: Incoming, chrif_connectack -> 'answer of the 2af8 login(ok / fail)'
//2afa: Outgoing, chrif_sendmap -> 'sending our maps'
//2afb: Incoming, chrif_sendmapack -> 'Maps received successfully / or not ..'
//2afc: Outgoing, chrif->scdata_request -> request sc_data for pc->authok'ed char. <- new command reuses previous one.
//2afd: Incoming, chrif->authok -> 'client authentication ok'
//2afe: Outgoing, send_usercount_tochar -> 'sends player count of this map server to charserver'
//2aff: Outgoing, chrif->send_users_tochar -> 'sends all actual connected character ids to charserver'
//2b00: Incoming, iMap->setusers -> 'set the actual usercount? PACKET.2B COUNT.L.. ?' (not sure)
//2b01: Outgoing, chrif->save -> 'charsave of char XY account XY (complete struct)'
//2b02: Outgoing, chrif->charselectreq -> 'player returns from ingame to charserver to select another char.., this packets includes sessid etc' ? (not 100% sure)
//2b03: Incoming, clif_charselectok -> '' (i think its the packet after enterworld?) (not sure)
//2b04: Incoming, chrif_recvmap -> 'getting maps from charserver of other mapserver's'
//2b05: Outgoing, chrif->changemapserver -> 'Tell the charserver the mapchange / quest for ok...'
//2b06: Incoming, chrif_changemapserverack -> 'awnser of 2b05, ok/fail, data: dunno^^'
//2b07: Outgoing, chrif->removefriend -> 'Tell charserver to remove friend_id from char_id friend list'
//2b08: Outgoing, chrif->searchcharid -> '...'
//2b09: Incoming, map_addchariddb -> 'Adds a name to the nick db'
//2b0a: Incoming/Outgoing, socket_datasync()
//2b0b: Outgoing, update charserv skillid2idx
//2b0c: Outgoing, chrif->changeemail -> 'change mail address ...'
//2b0d: Incoming, chrif_changedsex -> 'Change sex of acc XY'
//2b0e: Outgoing, chrif->char_ask_name -> 'Do some operations (change sex, ban / unban etc)'
//2b0f: Incoming, chrif_char_ask_name_answer -> 'answer of the 2b0e'
//2b10: Outgoing, chrif->updatefamelist -> 'Update the fame ranking lists and send them'
//2b11: Outgoing, chrif->divorce -> 'tell the charserver to do divorce'
//2b12: Incoming, chrif_divorceack -> 'divorce chars
//2b13: FREE
//2b14: Incoming, chrif_accountban -> 'not sure: kick the player with message XY'
//2b15: FREE
//2b16: Outgoing, chrif->ragsrvinfo -> 'sends base / job / drop rates ....'
//2b17: Outgoing, chrif->char_offline -> 'tell the charserver that the char is now offline'
//2b18: Outgoing, chrif->char_reset_offline -> 'set all players OFF!'
//2b19: Outgoing, chrif->char_online -> 'tell the charserver that the char .. is online'
//2b1a: Outgoing, chrif->buildfamelist -> 'Build the fame ranking lists and send them'
//2b1b: Incoming, chrif_recvfamelist -> 'Receive fame ranking lists'
//2b1c: Outgoing, chrif->save_scdata -> 'Send sc_data of player for saving.'
//2b1d: Incoming, chrif_load_scdata -> 'received sc_data of player for loading.'
//2b1e: Incoming, chrif_update_ip -> 'Reqest forwarded from char-server for interserver IP sync.' [Lance]
//2b1f: Incoming, chrif_disconnectplayer -> 'disconnects a player (aid X) with the message XY ... 0x81 ..' [Sirius]
//2b20: Incoming, chrif_removemap -> 'remove maps of a server (sample: its going offline)' [Sirius]
//2b21: Incoming, chrif_save_ack. Returned after a character has been "final saved" on the char-server. [Skotlex]
//2b22: Incoming, chrif_updatefamelist_ack. Updated one position in the fame list.
//2b23: Outgoing, chrif_keepalive. charserver ping.
//2b24: Incoming, chrif_keepalive_ack. charserver ping reply.
//2b25: Incoming, chrif_deadopt -> 'Removes baby from Father ID and Mother ID'
//2b26: Outgoing, chrif->authreq -> 'client authentication request'
//2b27: Incoming, chrif_authfail -> 'client authentication failed'

int char_fd = -1;
int srvinfo;
static char char_ip_str[128];
static uint32 char_ip = 0;
static uint16 char_port = 6121;
static char userid[NAME_LENGTH], passwd[NAME_LENGTH];
static int chrif_state = 0;

//Interval at which map server updates online listing. [Valaris]
#define CHECK_INTERVAL 3600000
//Interval at which map server sends number of connected users. [Skotlex]
#define UPDATE_INTERVAL 10000
//This define should spare writing the check in every function. [Skotlex]
#define chrif_check(a) { if(!chrif->isconnected()) return a; }


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
	if( auth_db->size(auth_db) > 0 )
		return;
	runflag = CORE_ST_STOP;
}

struct auth_node* chrif_search(int account_id) {
	return (struct auth_node*)idb_get(auth_db, account_id);
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
		
		if ( node->char_dat )
			aFree(node->char_dat);
		
		if ( node->sd )
			aFree(node->sd);
		
		ers_free(auth_db_ers, node);
		idb_remove(auth_db,account_id);
		
		return true;
	}
	return false;
}

//Moves the sd character to the auth_db structure.
static bool chrif_sd_to_auth(TBL_PC* sd, enum sd_state state) {
	struct auth_node *node;
	
	if ( chrif->search(sd->status.account_id) )
		return false; //Already exists?

	node = ers_alloc(auth_db_ers, struct auth_node);
	
	memset(node, 0, sizeof(struct auth_node));
	
	node->account_id = sd->status.account_id;
	node->char_id = sd->status.char_id;
	node->login_id1 = sd->login_id1;
	node->login_id2 = sd->login_id2;
	node->sex = sd->status.sex;
	node->fd = sd->fd;
	node->sd = sd;	//Data from logged on char.
	node->node_created = iTimer->gettick(); //timestamp for node timeouts
	node->state = state;

	sd->state.active = 0;
	
	idb_put(auth_db, node->account_id, node);
	
	return true;
}

static bool chrif_auth_logout(TBL_PC* sd, enum sd_state state) {
	
	if(sd->fd && state == ST_LOGOUT) { //Disassociate player, and free it after saving ack returns. [Skotlex]
		//fd info must not be lost for ST_MAPCHANGE as a final packet needs to be sent to the player.
		if ( session[sd->fd] )
			session[sd->fd]->session_data = NULL;
		sd->fd = 0;
	}
	
	return chrif_sd_to_auth(sd, state);
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
	memcpy(userid, id, NAME_LENGTH);
}

// sets char-server's password
void chrif_setpasswd(char *pwd) {
	memcpy(passwd, pwd, NAME_LENGTH);
}

// security check, prints warning if using default password
void chrif_checkdefaultlogin(void) {
	if (strcmp(userid, "s1")==0 && strcmp(passwd, "p1")==0) {
		ShowWarning("Using the default user/password s1/p1 is NOT RECOMMENDED.\n");
		ShowNotice("Please edit your 'login' table to create a proper inter-server user/password (gender 'S')\n");
		ShowNotice("and then edit your user/password in conf/map-server.conf (or conf/import/map_conf.txt)\n");
	}
}

// sets char-server's ip address
int chrif_setip(const char* ip) {
	char ip_str[16];
	
	if ( !( char_ip = host2ip(ip) ) ) {
		ShowWarning("Failed to Resolve Char Server Address! (%s)\n", ip);
		
		return 0;
	}
	
	safestrncpy(char_ip_str, ip, sizeof(char_ip_str));
	
	ShowInfo("Char Server IP Address : '"CL_WHITE"%s"CL_RESET"' -> '"CL_WHITE"%s"CL_RESET"'.\n", ip, ip2str(char_ip, ip_str));
	
	return 1;
}

// sets char-server's port number
void chrif_setport(uint16 port) {
	char_port = port;
}

// says whether the char-server is connected or not
int chrif_isconnected(void) {
	return (char_fd > 0 && session[char_fd] != NULL && chrif_state == 2);
}

/*==========================================
 * Saves character data.
 * Flag = 1: Character is quitting
 * Flag = 2: Character is changing map-servers
 *------------------------------------------*/
int chrif_save(struct map_session_data *sd, int flag) {
	nullpo_retr(-1, sd);

	pc->makesavestatus(sd);

	if (flag && sd->state.active) { //Store player data which is quitting
		//FIXME: SC are lost if there's no connection at save-time because of the way its related data is cleared immediately after this function. [Skotlex]
		if ( chrif->isconnected() )
			chrif->save_scdata(sd);
		if ( !chrif_auth_logout(sd,flag == 1 ? ST_LOGOUT : ST_MAPCHANGE) )
			ShowError("chrif_save: Failed to set up player %d:%d for proper quitting!\n", sd->status.account_id, sd->status.char_id);
	}

	chrif_check(-1); //Character is saved on reconnect.

	//For data sync
	if (sd->state.storage_flag == 2)
		gstorage->save(sd->status.account_id, sd->status.guild_id, flag);

	if (flag)
		sd->state.storage_flag = 0; //Force close it.

	//Saving of registry values. 
	if (sd->state.reg_dirty&4)
		intif->saveregistry(sd, 3); //Save char regs
	if (sd->state.reg_dirty&2)
		intif->saveregistry(sd, 2); //Save account regs
	if (sd->state.reg_dirty&1)
		intif->saveregistry(sd, 1); //Save account2 regs

	WFIFOHEAD(char_fd, sizeof(sd->status) + 13);
	WFIFOW(char_fd,0) = 0x2b01;
	WFIFOW(char_fd,2) = sizeof(sd->status) + 13;
	WFIFOL(char_fd,4) = sd->status.account_id;
	WFIFOL(char_fd,8) = sd->status.char_id;
	WFIFOB(char_fd,12) = (flag==1)?1:0; //Flag to tell char-server this character is quitting.
	memcpy(WFIFOP(char_fd,13), &sd->status, sizeof(sd->status));
	WFIFOSET(char_fd, WFIFOW(char_fd,2));

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

	return 0;
}

// connects to char-server (plaintext)
int chrif_connect(int fd) {
	ShowStatus("Logging in to char server...\n", char_fd);
	WFIFOHEAD(fd,60);
	WFIFOW(fd,0) = 0x2af8;
	memcpy(WFIFOP(fd,2), userid, NAME_LENGTH);
	memcpy(WFIFOP(fd,26), passwd, NAME_LENGTH);
	WFIFOL(fd,50) = 0;
	WFIFOL(fd,54) = htonl(clif->map_ip);
	WFIFOW(fd,58) = htons(clif->map_port);
	WFIFOSET(fd,60);

	return 0;
}

// sends maps to char-server
int chrif_sendmap(int fd) {
	int i;
	
	ShowStatus("Sending maps to char server...\n");
	
	// Sending normal maps, not instances
	WFIFOHEAD(fd, 4 + instance->start_id * 4);
	WFIFOW(fd,0) = 0x2afa;
	for(i = 0; i < instance->start_id; i++)
		WFIFOW(fd,4+i*4) = map[i].index;
	WFIFOW(fd,2) = 4 + i * 4;
	WFIFOSET(fd,WFIFOW(fd,2));

	return 0;
}

// receive maps from some other map-server (relayed via char-server)
int chrif_recvmap(int fd) {
	int i, j;
	uint32 ip = ntohl(RFIFOL(fd,4));
	uint16 port = ntohs(RFIFOW(fd,8));

	for(i = 10, j = 0; i < RFIFOW(fd,2); i += 4, j++) {
		iMap->setipport(RFIFOW(fd,i), ip, port);
	}
	
	if (battle_config.etc_log)
		ShowStatus("Received maps from %d.%d.%d.%d:%d (%d maps)\n", CONVIP(ip), port, j);

	chrif->other_mapserver_count++;
	
	return 0;
}

// remove specified maps (used when some other map-server disconnects)
int chrif_removemap(int fd) {
	int i, j;
	uint32 ip =  RFIFOL(fd,4);
	uint16 port = RFIFOW(fd,8);

	for(i = 10, j = 0; i < RFIFOW(fd, 2); i += 4, j++)
		iMap->eraseipport(RFIFOW(fd, i), ip, port);

	chrif->other_mapserver_count--;
	
	if(battle_config.etc_log)
		ShowStatus("remove map of server %d.%d.%d.%d:%d (%d maps)\n", CONVIP(ip), port, j);
	
	return 0;
}

// received after a character has been "final saved" on the char-server
static void chrif_save_ack(int fd) {
	chrif->auth_delete(RFIFOL(fd,2), RFIFOL(fd,6), ST_LOGOUT);
	chrif->check_shutdown();
}

// request to move a character between mapservers
int chrif_changemapserver(struct map_session_data* sd, uint32 ip, uint16 port) {
	nullpo_retr(-1, sd);

	if (chrif->other_mapserver_count < 1) {//No other map servers are online!
		clif->authfail_fd(sd->fd, 0);
		return -1;
	}

	chrif_check(-1);

	WFIFOHEAD(char_fd,35);
	WFIFOW(char_fd, 0) = 0x2b05;
	WFIFOL(char_fd, 2) = sd->bl.id;
	WFIFOL(char_fd, 6) = sd->login_id1;
	WFIFOL(char_fd,10) = sd->login_id2;
	WFIFOL(char_fd,14) = sd->status.char_id;
	WFIFOW(char_fd,18) = sd->mapindex;
	WFIFOW(char_fd,20) = sd->bl.x;
	WFIFOW(char_fd,22) = sd->bl.y;
	WFIFOL(char_fd,24) = htonl(ip);
	WFIFOW(char_fd,28) = htons(port);
	WFIFOB(char_fd,30) = sd->status.sex;
	WFIFOL(char_fd,31) = htonl(session[sd->fd]->client_addr);
	WFIFOL(char_fd,35) = sd->group_id;
	WFIFOSET(char_fd,39);
	
	return 0;
}

/// map-server change request acknowledgement (positive or negative)
/// R 2b06 <account_id>.L <login_id1>.L <login_id2>.L <char_id>.L <map_index>.W <x>.W <y>.W <ip>.L <port>.W
int chrif_changemapserverack(int account_id, int login_id1, int login_id2, int char_id, short map_index, short x, short y, uint32 ip, uint16 port) {
	struct auth_node *node;
	
	if ( !( node = chrif->auth_check(account_id, char_id, ST_MAPCHANGE) ) )
		return -1;

	if ( !login_id1 ) {
		ShowError("map server change failed.\n");
		clif->authfail_fd(node->fd, 0);
	} else
		clif->changemapserver(node->sd, map_index, x, y, ntohl(ip), ntohs(port));

	//Player has been saved already, remove him from memory. [Skotlex]
	chrif->auth_delete(account_id, char_id, ST_MAPCHANGE);

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int chrif_connectack(int fd) {
	static bool char_init_done = false;

	if (RFIFOB(fd,2)) {
		ShowFatalError("Connection to char-server failed %d.\n", RFIFOB(fd,2));
		exit(EXIT_FAILURE);
	}

	ShowStatus("Successfully logged on to Char Server (Connection: '"CL_WHITE"%d"CL_RESET"').\n",fd);
	chrif_state = 1;
	chrif->connected = 1;

	chrif_sendmap(fd);

	ShowStatus("Event '"CL_WHITE"OnInterIfInit"CL_RESET"' executed with '"CL_WHITE"%d"CL_RESET"' NPCs.\n", npc_event_doall("OnInterIfInit"));
	if( !char_init_done ) {
		char_init_done = true;
		ShowStatus("Event '"CL_WHITE"OnInterIfInitOnce"CL_RESET"' executed with '"CL_WHITE"%d"CL_RESET"' NPCs.\n", npc_event_doall("OnInterIfInitOnce"));
		guild->castle_map_init();
	}
	
	socket_datasync(fd, true);
	chrif->skillid2idx(fd);

	return 0;
}

/**
 * @see DBApply
 */
static int chrif_reconnect(DBKey key, DBData *data, va_list ap) {
	struct auth_node *node = DB->data2ptr(data);
	
	switch (node->state) {
		case ST_LOGIN:
			if ( node->sd && node->char_dat == NULL ) {//Since there is no way to request the char auth, make it fail.
				pc->authfail(node->sd);
				chrif->char_offline(node->sd);
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
			
			if( iMap->mapname2ipport(sd->mapindex,&ip,&port) == 0 )
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
	ShowStatus("Map Server is now online.\n");
	
	chrif_state = 2;
	
	chrif->check_shutdown();

	//If there are players online, send them to the char-server. [Skotlex]
	chrif->send_users_tochar();

	//Auth db reconnect handling
	auth_db->foreach(auth_db,chrif_reconnect);

	//Re-save any storages that were modified in the disconnection time. [Skotlex]
	storage->reconnect();

	//Re-save any guild castles that were modified in the disconnection time.
	guild->castle_reconnect(-1, 0, 0);
}


/*==========================================
 *
 *------------------------------------------*/
int chrif_sendmapack(int fd) {
	
	if (RFIFOB(fd,2)) {
		ShowFatalError("chrif : send map list to char server failed %d\n", RFIFOB(fd,2));
		exit(EXIT_FAILURE);
	}

	memcpy(iMap->wisp_server_name, RFIFOP(fd,3), NAME_LENGTH);
	
	chrif_on_ready();
	
	return 0;
}

/*==========================================
 * Request sc_data from charserver [Skotlex]
 *------------------------------------------*/
int chrif_scdata_request(int account_id, int char_id) {
	
#ifdef ENABLE_SC_SAVING
	chrif_check(-1);

	WFIFOHEAD(char_fd,10);
	WFIFOW(char_fd,0) = 0x2afc;
	WFIFOL(char_fd,2) = account_id;
	WFIFOL(char_fd,6) = char_id;
	WFIFOSET(char_fd,10);
#endif
	
	return 0;
}

/*==========================================
 * Request auth confirmation
 *------------------------------------------*/
void chrif_authreq(struct map_session_data *sd) {
	struct auth_node *node= chrif->search(sd->bl.id);

	if( node != NULL || !chrif->isconnected() ) {
		set_eof(sd->fd);
		return;
	}

	WFIFOHEAD(char_fd,19);
	WFIFOW(char_fd,0) = 0x2b26;
	WFIFOL(char_fd,2) = sd->status.account_id;
	WFIFOL(char_fd,6) = sd->status.char_id;
	WFIFOL(char_fd,10) = sd->login_id1;
	WFIFOB(char_fd,14) = sd->status.sex;
	WFIFOL(char_fd,15) = htonl(session[sd->fd]->client_addr);
	WFIFOSET(char_fd,19);
	chrif_sd_to_auth(sd, ST_LOGIN);
}

/*==========================================
 * Auth confirmation ack
 *------------------------------------------*/
void chrif_authok(int fd) {
	int account_id, group_id, char_id;
	uint32 login_id1,login_id2;
	time_t expiration_time;
	struct mmo_charstatus* status;
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
	status = (struct mmo_charstatus*)RFIFOP(fd,25);
	char_id = status->char_id;

	//Check if we don't already have player data in our server
	//Causes problems if the currently connected player tries to quit or this data belongs to an already connected player which is trying to re-auth.
	if ( ( sd = iMap->id2sd(account_id) ) != NULL )
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
		node->char_dat == NULL &&
		node->account_id == account_id &&
		node->char_id == char_id &&
		node->login_id1 == login_id1 )
	{ //Auth Ok
		if (pc->authok(sd, login_id2, expiration_time, group_id, status, changing_mapservers))
			return;
	} else { //Auth Failed
		pc->authfail(sd);
	}
	
	chrif->char_offline(sd); //Set him offline, the char server likely has it set as online already.
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
	
	if(DIFF_TICK(iTimer->gettick(),node->node_created)>60000) {
		switch (node->state) {
			case ST_LOGOUT:
				//Re-save attempt (->sd should never be null here).
				node->node_created = iTimer->gettick(); //Refresh tick (avoid char-server load if connection is really bad)
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

int auth_db_cleanup(int tid, unsigned int tick, int id, intptr_t data) {
	chrif_check(0);
	auth_db->foreach(auth_db, auth_db_cleanup_sub);
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int chrif_charselectreq(struct map_session_data* sd, uint32 s_ip) {
	nullpo_retr(-1, sd);

	if( !sd || !sd->bl.id || !sd->login_id1 )
		return -1;
	
	chrif_check(-1);

	WFIFOHEAD(char_fd,18);
	WFIFOW(char_fd, 0) = 0x2b02;
	WFIFOL(char_fd, 2) = sd->bl.id;
	WFIFOL(char_fd, 6) = sd->login_id1;
	WFIFOL(char_fd,10) = sd->login_id2;
	WFIFOL(char_fd,14) = htonl(s_ip);
	WFIFOSET(char_fd,18);

	return 0;
}

/*==========================================
 * Search Char trough id on char serv
 *------------------------------------------*/
int chrif_searchcharid(int char_id) {
	
	if( !char_id )
		return -1;
	
	chrif_check(-1);

	WFIFOHEAD(char_fd,6);
	WFIFOW(char_fd,0) = 0x2b08;
	WFIFOL(char_fd,2) = char_id;
	WFIFOSET(char_fd,6);

	return 0;
}

/*==========================================
 * Change Email
 *------------------------------------------*/
int chrif_changeemail(int id, const char *actual_email, const char *new_email) {
	
	if (battle_config.etc_log)
		ShowInfo("chrif_changeemail: account: %d, actual_email: '%s', new_email: '%s'.\n", id, actual_email, new_email);

	chrif_check(-1);

	WFIFOHEAD(char_fd,86);
	WFIFOW(char_fd,0) = 0x2b0c;
	WFIFOL(char_fd,2) = id;
	memcpy(WFIFOP(char_fd,6), actual_email, 40);
	memcpy(WFIFOP(char_fd,46), new_email, 40);
	WFIFOSET(char_fd,86);

	return 0;
}

/*==========================================
 * S 2b0e <accid>.l <name>.24B <type>.w { <year>.w <month>.w <day>.w <hour>.w <minute>.w <second>.w }
 * Send an account modification request to the login server (via char server).
 * type of operation:
 *   1: block, 2: ban, 3: unblock, 4: unban, 5: changesex (use next function for 5)
 *------------------------------------------*/
int chrif_char_ask_name(int acc, const char* character_name, unsigned short operation_type, int year, int month, int day, int hour, int minute, int second) {
	
	chrif_check(-1);

	WFIFOHEAD(char_fd,44);
	WFIFOW(char_fd,0) = 0x2b0e;
	WFIFOL(char_fd,2) = acc;
	safestrncpy((char*)WFIFOP(char_fd,6), character_name, NAME_LENGTH);
	WFIFOW(char_fd,30) = operation_type;
	
	if ( operation_type == 2 ) {
		WFIFOW(char_fd,32) = year;
		WFIFOW(char_fd,34) = month;
		WFIFOW(char_fd,36) = day;
		WFIFOW(char_fd,38) = hour;
		WFIFOW(char_fd,40) = minute;
		WFIFOW(char_fd,42) = second;
	}
	
	WFIFOSET(char_fd,44);
	return 0;
}

int chrif_changesex(struct map_session_data *sd) {
	chrif_check(-1);
	
	WFIFOHEAD(char_fd,44);
	WFIFOW(char_fd,0) = 0x2b0e;
	WFIFOL(char_fd,2) = sd->status.account_id;
	safestrncpy((char*)WFIFOP(char_fd,6), sd->status.name, NAME_LENGTH);
	WFIFOW(char_fd,30) = 5;
	WFIFOSET(char_fd,44);

	clif->message(sd->fd, msg_txt(408)); //"Need disconnection to perform change-sex request..."

	if (sd->fd)
		clif->authfail_fd(sd->fd, 15);
	else
		iMap->quit(sd);
	return 0;
}

/*==========================================
 * R 2b0f <accid>.l <name>.24B <type>.w <answer>.w
 * Processing a reply to chrif->char_ask_name() (request to modify an account).
 * type of operation:
 *   1: block, 2: ban, 3: unblock, 4: unban, 5: changesex
 * type of answer:
 *   0: login-server request done
 *   1: player not found
 *   2: gm level too low
 *   3: login-server offline
 *------------------------------------------*/
static void chrif_char_ask_name_answer(int acc, const char* player_name, uint16 type, uint16 answer) {
	struct map_session_data* sd;
	char action[25];
	char output[256];
	
	sd = iMap->id2sd(acc);
	
	if( acc < 0 || sd == NULL ) {
		ShowError("chrif_char_ask_name_answer failed - player not online.\n");
		return;
	}

	if( type > 0 && type <= 5 )
		snprintf(action,25,"%s",msg_txt(427+type)); //block|ban|unblock|unban|change the sex of
	else
		snprintf(action,25,"???");

	switch( answer ) {
		case 0 : sprintf(output, msg_txt(424), action, NAME_LENGTH, player_name); break;
		case 1 : sprintf(output, msg_txt(425), NAME_LENGTH, player_name); break;
		case 2 : sprintf(output, msg_txt(426), action, NAME_LENGTH, player_name); break;
		case 3 : sprintf(output, msg_txt(427), action, NAME_LENGTH, player_name); break;
		default: output[0] = '\0'; break;
	}
	
	clif->message(sd->fd, output);
}

/*==========================================
 * Request char server to change sex of char (modified by Yor)
 *------------------------------------------*/
int chrif_changedsex(int fd) {
	int acc, sex;
	struct map_session_data *sd;

	acc = RFIFOL(fd,2);
	sex = RFIFOL(fd,6);
	
	if ( battle_config.etc_log )
		ShowNotice("chrif_changedsex %d.\n", acc);
	
	sd = iMap->id2sd(acc);
	if ( sd ) { //Normally there should not be a char logged on right now!
		if ( sd->status.sex == sex ) 
			return 0; //Do nothing? Likely safe.
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
		iMap->quit(sd); // Remove leftovers (e.g. autotrading) [Paradox924X]
	}
	return 0;
}
/*==========================================
 * Request Char Server to Divorce Players
 *------------------------------------------*/
int chrif_divorce(int partner_id1, int partner_id2) {
	chrif_check(-1);

	WFIFOHEAD(char_fd,10);
	WFIFOW(char_fd,0) = 0x2b11;
	WFIFOL(char_fd,2) = partner_id1;
	WFIFOL(char_fd,6) = partner_id2;
	WFIFOSET(char_fd,10);

	return 0;
}

/*==========================================
 * Divorce players
 * only used if 'partner_id' is offline
 *------------------------------------------*/
int chrif_divorceack(int char_id, int partner_id) {
	struct map_session_data* sd;
	int i;

	if( !char_id || !partner_id )
		return 0;

	if( ( sd = iMap->charid2sd(char_id) ) != NULL && sd->status.partner_id == partner_id ) {
		sd->status.partner_id = 0;
		for(i = 0; i < MAX_INVENTORY; i++)
			if (sd->status.inventory[i].nameid == WEDDING_RING_M || sd->status.inventory[i].nameid == WEDDING_RING_F)
				pc->delitem(sd, i, 1, 0, 0, LOG_TYPE_OTHER);
	}

	if( ( sd = iMap->charid2sd(partner_id) ) != NULL && sd->status.partner_id == char_id ) {
		sd->status.partner_id = 0;
		for(i = 0; i < MAX_INVENTORY; i++)
			if (sd->status.inventory[i].nameid == WEDDING_RING_M || sd->status.inventory[i].nameid == WEDDING_RING_F)
				pc->delitem(sd, i, 1, 0, 0, LOG_TYPE_OTHER);
	}
	
	return 0;
}
/*==========================================
 * Removes Baby from parents
 *------------------------------------------*/
int chrif_deadopt(int father_id, int mother_id, int child_id) {
	struct map_session_data* sd;
	int idx = skill->get_index(WE_CALLBABY);

	if( father_id && ( sd = iMap->charid2sd(father_id) ) != NULL && sd->status.child == child_id ) {
		sd->status.child = 0;
		sd->status.skill[idx].id = 0;
		sd->status.skill[idx].lv = 0;
		sd->status.skill[idx].flag = 0;
		clif->deleteskill(sd,WE_CALLBABY);
	}

	if( mother_id && ( sd = iMap->charid2sd(mother_id) ) != NULL && sd->status.child == child_id ) {
		sd->status.child = 0;
		sd->status.skill[idx].id = 0;
		sd->status.skill[idx].lv = 0;
		sd->status.skill[idx].flag = 0;
		clif->deleteskill(sd,WE_CALLBABY);
	}

	return 0;
}

/*==========================================
 * Disconnection of a player (account has been banned of has a status, from login-server) by [Yor]
 *------------------------------------------*/
int chrif_accountban(int fd) {
	int acc;
	struct map_session_data *sd;

	acc = RFIFOL(fd,2);
	
	if ( battle_config.etc_log )
		ShowNotice("chrif_accountban %d.\n", acc);
	
	sd = iMap->id2sd(acc);

	if ( acc < 0 || sd == NULL ) {
		ShowError("chrif_accountban failed - player not online.\n");
		return 0;
	}

	sd->login_id1++; // change identify, because if player come back in char within the 5 seconds, he can change its characters
	if (RFIFOB(fd,6) == 0) { // 0: change of statut, 1: ban
                int ret_status = RFIFOL(fd,7); // status or final date of a banishment
                if(0<ret_status && ret_status<=9)
                    clif->message(sd->fd, msg_txt(411+ret_status));
                else if(ret_status==100)
                    clif->message(sd->fd, msg_txt(421));
                else    
                    clif->message(sd->fd, msg_txt(420)); //"Your account has not more authorised."
	} else if (RFIFOB(fd,6) == 1) { // 0: change of statut, 1: ban
		time_t timestamp;
		char tmpstr[2048];
		timestamp = (time_t)RFIFOL(fd,7); // status or final date of a banishment
		strcpy(tmpstr, msg_txt(423)); //"Your account has been banished until "
		strftime(tmpstr + strlen(tmpstr), 24, "%d-%m-%Y %H:%M:%S", localtime(&timestamp));
		clif->message(sd->fd, tmpstr);
	}

	set_eof(sd->fd); // forced to disconnect for the change
	iMap->quit(sd); // Remove leftovers (e.g. autotrading) [Paradox924X]
	return 0;
}

//Disconnect the player out of the game, simple packet
//packet.w AID.L WHY.B 2+4+1 = 7byte
int chrif_disconnectplayer(int fd) {
	struct map_session_data* sd;
	int account_id = RFIFOL(fd, 2);

	sd = iMap->id2sd(account_id);
	if( sd == NULL ) {
		struct auth_node* auth = chrif->search(account_id);
		
		if( auth != NULL && chrif->auth_delete(account_id, auth->char_id, ST_LOGIN) )
			return 0;
		
		return -1;
	}

	if (!sd->fd) { //No connection
		if (sd->state.autotrade)
			iMap->quit(sd); //Remove it.
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
		case MAPID_BLACKSMITH: type = 1; break;
		case MAPID_ALCHEMIST:  type = 2; break;
		case MAPID_TAEKWON:    type = 3; break;
		default:
			return 0;
	}

	WFIFOHEAD(char_fd, 11);
	WFIFOW(char_fd,0) = 0x2b10;
	WFIFOL(char_fd,2) = sd->status.char_id;
	WFIFOL(char_fd,6) = sd->status.fame;
	WFIFOB(char_fd,10) = type;
	WFIFOSET(char_fd,11);

	return 0;
}

int chrif_buildfamelist(void) {
	chrif_check(-1);

	WFIFOHEAD(char_fd,2);
	WFIFOW(char_fd,0) = 0x2b1a;
	WFIFOSET(char_fd,2);

	return 0;
}

int chrif_recvfamelist(int fd) {
	int num, size;
	int total = 0, len = 8;

	memset (smith_fame_list, 0, sizeof(smith_fame_list));
	memset (chemist_fame_list, 0, sizeof(chemist_fame_list));
	memset (taekwon_fame_list, 0, sizeof(taekwon_fame_list));

	size = RFIFOW(fd, 6); //Blacksmith block size
	
	for (num = 0; len < size && num < MAX_FAME_LIST; num++) {
		memcpy(&smith_fame_list[num], RFIFOP(fd,len), sizeof(struct fame_list));
 		len += sizeof(struct fame_list);
	}
	
	total += num;

	size = RFIFOW(fd, 4); //Alchemist block size
	
	for (num = 0; len < size && num < MAX_FAME_LIST; num++) {
		memcpy(&chemist_fame_list[num], RFIFOP(fd,len), sizeof(struct fame_list));
 		len += sizeof(struct fame_list);
	}
	
	total += num;

	size = RFIFOW(fd, 2); //Total packet length
	
	for (num = 0; len < size && num < MAX_FAME_LIST; num++) {
		memcpy(&taekwon_fame_list[num], RFIFOP(fd,len), sizeof(struct fame_list));
 		len += sizeof(struct fame_list);
	}
	
	total += num;

	ShowInfo("Received Fame List of '"CL_WHITE"%d"CL_RESET"' characters.\n", total);

	return 0;
}

/// fame ranking update confirmation
/// R 2b22 <table>.B <index>.B <value>.L
int chrif_updatefamelist_ack(int fd) {
	struct fame_list* list;
	uint8 index;
	
	switch (RFIFOB(fd,2)) {
		case 1: list = smith_fame_list;   break;
		case 2: list = chemist_fame_list; break;
		case 3: list = taekwon_fame_list; break;
		default: return 0;
	}
	
	index = RFIFOB(fd, 3);
	
	if (index >= MAX_FAME_LIST)
		return 0;
	
	list[index].fame = RFIFOL(fd,4);
	
	return 1;
}

int chrif_save_scdata(struct map_session_data *sd) { //parses the sc_data of the player and sends it to the char-server for saving. [Skotlex]

#ifdef ENABLE_SC_SAVING
	int i, count=0;
	unsigned int tick;
	struct status_change_data data;
	struct status_change *sc = &sd->sc;
	const struct TimerData *timer;

	chrif_check(-1);
	tick = iTimer->gettick();
	
	WFIFOHEAD(char_fd, 14 + SC_MAX*sizeof(struct status_change_data));
	WFIFOW(char_fd,0) = 0x2b1c;
	WFIFOL(char_fd,4) = sd->status.account_id;
	WFIFOL(char_fd,8) = sd->status.char_id;
	
	for (i = 0; i < SC_MAX; i++) {
		if (!sc->data[i])
			continue;
		if (sc->data[i]->timer != INVALID_TIMER) {
			timer = iTimer->get_timer(sc->data[i]->timer);
			if (timer == NULL || timer->func != iStatus->change_timer || DIFF_TICK(timer->tick,tick) < 0)
				continue;
			data.tick = DIFF_TICK(timer->tick,tick); //Duration that is left before ending.
		} else
			data.tick = -1; //Infinite duration
		data.type = i;
		data.val1 = sc->data[i]->val1;
		data.val2 = sc->data[i]->val2;
		data.val3 = sc->data[i]->val3;
		data.val4 = sc->data[i]->val4;
		memcpy(WFIFOP(char_fd,14 +count*sizeof(struct status_change_data)),
			&data, sizeof(struct status_change_data));
		count++;
	}
	
	if (count == 0)
		return 0; //Nothing to save.
	
	WFIFOW(char_fd,12) = count;
	WFIFOW(char_fd,2) = 14 +count*sizeof(struct status_change_data); //Total packet size
	WFIFOSET(char_fd,WFIFOW(char_fd,2));
#endif
	
	return 0;
}

//Retrieve and load sc_data for a player. [Skotlex]
int chrif_load_scdata(int fd) {

#ifdef ENABLE_SC_SAVING
	struct map_session_data *sd;
	struct status_change_data *data;
	int aid, cid, i, count;

	aid = RFIFOL(fd,4); //Player Account ID
	cid = RFIFOL(fd,8); //Player Char ID
	
	sd = iMap->id2sd(aid);
	
	if ( !sd ) {
		ShowError("chrif_load_scdata: Player of AID %d not found!\n", aid);
		return -1;
	}
	
	if ( sd->status.char_id != cid ) {
		ShowError("chrif_load_scdata: Receiving data for account %d, char id does not matches (%d != %d)!\n", aid, sd->status.char_id, cid);
		return -1;
	}
	
	count = RFIFOW(fd,12); //sc_count
	
	for (i = 0; i < count; i++) {
		data = (struct status_change_data*)RFIFOP(fd,14 + i*sizeof(struct status_change_data));
		iStatus->change_start(&sd->bl, (sc_type)data->type, 10000, data->val1, data->val2, data->val3, data->val4, data->tick, 15);
	}
#endif
	
	return 0;
}

/*==========================================
 * Send rates to char server [Wizputer]
 * S 2b16 <base rate>.L <job rate>.L <drop rate>.L
 *------------------------------------------*/
int chrif_ragsrvinfo(int base_rate, int job_rate, int drop_rate) {
	chrif_check(-1);

	WFIFOHEAD(char_fd,14);
	WFIFOW(char_fd,0) = 0x2b16;
	WFIFOL(char_fd,2) = base_rate;
	WFIFOL(char_fd,6) = job_rate;
	WFIFOL(char_fd,10) = drop_rate;
	WFIFOSET(char_fd,14);
	
	return 0;
}


/*=========================================
 * Tell char-server charcter disconnected [Wizputer]
 *-----------------------------------------*/
int chrif_char_offline(struct map_session_data *sd) {
	chrif_check(-1);

	WFIFOHEAD(char_fd,10);
	WFIFOW(char_fd,0) = 0x2b17;
	WFIFOL(char_fd,2) = sd->status.char_id;
	WFIFOL(char_fd,6) = sd->status.account_id;
	WFIFOSET(char_fd,10);

	return 0;
}
int chrif_char_offline_nsd(int account_id, int char_id) {
	chrif_check(-1);

	WFIFOHEAD(char_fd,10);
	WFIFOW(char_fd,0) = 0x2b17;
	WFIFOL(char_fd,2) = char_id;
	WFIFOL(char_fd,6) = account_id;
	WFIFOSET(char_fd,10);

	return 0;
}

/*=========================================
 * Tell char-server to reset all chars offline [Wizputer]
 *-----------------------------------------*/
int chrif_flush_fifo(void) {
	chrif_check(-1);

	set_nonblocking(char_fd, 0);
	flush_fifos();
	set_nonblocking(char_fd, 1);

	return 0;
}

/*=========================================
 * Tell char-server to reset all chars offline [Wizputer]
 *-----------------------------------------*/
int chrif_char_reset_offline(void) {
	chrif_check(-1);

	WFIFOHEAD(char_fd,2);
	WFIFOW(char_fd,0) = 0x2b18;
	WFIFOSET(char_fd,2);

	return 0;
}

/*=========================================
 * Tell char-server charcter is online [Wizputer]
 *-----------------------------------------*/

int chrif_char_online(struct map_session_data *sd) {
	chrif_check(-1);

	WFIFOHEAD(char_fd,10);
	WFIFOW(char_fd,0) = 0x2b19;
	WFIFOL(char_fd,2) = sd->status.char_id;
	WFIFOL(char_fd,6) = sd->status.account_id;
	WFIFOSET(char_fd,10);

	return 0;
}


/// Called when the connection to Char Server is disconnected.
void chrif_on_disconnect(void) {
	if( chrif->connected != 1 )
		ShowWarning("Connection to Char Server lost.\n\n");
	chrif->connected = 0;
	
 	chrif->other_mapserver_count = 0; //Reset counter. We receive ALL maps from all map-servers on reconnect.
	iMap->eraseallipport();

	//Attempt to reconnect in a second. [Skotlex]
	iTimer->add_timer(iTimer->gettick() + 1000, check_connect_char_server, 0, 0);
}


void chrif_update_ip(int fd) {
	uint32 new_ip;
	
	WFIFOHEAD(fd,6);
	
	new_ip = host2ip(char_ip_str);
	
	if (new_ip && new_ip != char_ip)
		char_ip = new_ip; //Update char_ip

	new_ip = clif->refresh_ip();
	
	if (!new_ip)
		return; //No change
	
	WFIFOW(fd,0) = 0x2736;
	WFIFOL(fd,2) = htonl(new_ip);
	WFIFOSET(fd,6);
}

// pings the charserver
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
	
	if( fd == 0 ) fd = char_fd;
	
	WFIFOHEAD(fd,4 + (MAX_SKILL * 4));
	WFIFOW(fd,0) = 0x2b0b;
	for(i = 0; i < MAX_SKILL; i++) {
		if( skill_db[i].nameid ) {
			WFIFOW(fd, 4 + (count*4)) = skill_db[i].nameid;
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
	int packet_len, cmd;

	// only process data from the char-server
	if ( fd != char_fd ) {
		ShowDebug("chrif_parse: Disconnecting invalid session #%d (is not the char-server)\n", fd);
		do_close(fd);
		return 0;
	}

	if ( session[fd]->flag.eof ) {
		do_close(fd);
		char_fd = -1;
		chrif_on_disconnect();
		return 0;
	} else if ( session[fd]->flag.ping ) {/* we've reached stall time */
		if( DIFF_TICK(last_tick, session[fd]->rdata_tick) > (stall_time * 2) ) {/* we can't wait any longer */
			set_eof(fd);
			return 0;
		} else if( session[fd]->flag.ping != 2 ) { /* we haven't sent ping out yet */
			chrif_keepalive(fd);
			session[fd]->flag.ping = 2;
		}
	}

	while ( RFIFOREST(fd) >= 2 ) {
		cmd = RFIFOW(fd,0);
		if (cmd < 0x2af8 || cmd >= 0x2af8 + ARRAYLENGTH(packet_len_table) || packet_len_table[cmd-0x2af8] == 0) {
			int r = intif->parse(fd); // Passed on to the intif

			if (r == 1) continue;	// Treated in intif 
			if (r == 2) return 0;	// Didn't have enough data (len==-1)

			ShowWarning("chrif_parse: session #%d, intif->parse failed (unrecognized command 0x%.4x).\n", fd, cmd);
			set_eof(fd);
			return 0;
		}

		if ( ( packet_len = packet_len_table[cmd-0x2af8] ) == -1) { // dynamic-length packet, second WORD holds the length
			if (RFIFOREST(fd) < 4)
				return 0;
			packet_len = RFIFOW(fd,2);
		}

		if ((int)RFIFOREST(fd) < packet_len)
			return 0;

		//ShowDebug("Received packet 0x%4x (%d bytes) from char-server (connection %d)\n", RFIFOW(fd,0), packet_len, fd);

		switch(cmd) {
			case 0x2af9: chrif_connectack(fd); break;
			case 0x2afb: chrif_sendmapack(fd); break;
			case 0x2afd: chrif->authok(fd); break;
			case 0x2b00: iMap->setusers(RFIFOL(fd,2)); chrif_keepalive(fd); break;
			case 0x2b03: clif->charselectok(RFIFOL(fd,2), RFIFOB(fd,6)); break;
			case 0x2b04: chrif_recvmap(fd); break;
			case 0x2b06: chrif_changemapserverack(RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOL(fd,14), RFIFOW(fd,18), RFIFOW(fd,20), RFIFOW(fd,22), RFIFOL(fd,24), RFIFOW(fd,28)); break;
			case 0x2b09: iMap->addnickdb(RFIFOL(fd,2), (char*)RFIFOP(fd,6)); break;
			case 0x2b0a: socket_datasync(fd, false); break;
			case 0x2b0d: chrif_changedsex(fd); break;
			case 0x2b0f: chrif_char_ask_name_answer(RFIFOL(fd,2), (char*)RFIFOP(fd,6), RFIFOW(fd,30), RFIFOW(fd,32)); break;
			case 0x2b12: chrif_divorceack(RFIFOL(fd,2), RFIFOL(fd,6)); break;
			case 0x2b14: chrif_accountban(fd); break;
			case 0x2b1b: chrif_recvfamelist(fd); break;
			case 0x2b1d: chrif_load_scdata(fd); break;
			case 0x2b1e: chrif_update_ip(fd); break;
			case 0x2b1f: chrif_disconnectplayer(fd); break;
			case 0x2b20: chrif_removemap(fd); break;
			case 0x2b21: chrif_save_ack(fd); break;
			case 0x2b22: chrif_updatefamelist_ack(fd); break;
			case 0x2b24: chrif_keepalive_ack(fd); break;
			case 0x2b25: chrif_deadopt(RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10)); break;
			case 0x2b27: chrif_authfail(fd); break;
			default:
				ShowError("chrif_parse : unknown packet (session #%d): 0x%x. Disconnecting.\n", fd, cmd);
				set_eof(fd);
				return 0;
		}
		if ( fd == char_fd ) //There's the slight chance we lost the connection during parse, in which case this would segfault if not checked [Skotlex]
			RFIFOSKIP(fd, packet_len);
	}

	return 0;
}

// unused
int send_usercount_tochar(int tid, unsigned int tick, int id, intptr_t data) {
	chrif_check(-1);

	WFIFOHEAD(char_fd,4);
	WFIFOW(char_fd,0) = 0x2afe;
	WFIFOW(char_fd,2) = iMap->usercount();
	WFIFOSET(char_fd,4);
	return 0;
}

/*==========================================
 * timerFunction
 * Send to char the number of client connected to map
 *------------------------------------------*/
int send_users_tochar(void) {
	int users = 0, i = 0;
	struct map_session_data* sd;
	struct s_mapiterator* iter;

	chrif_check(-1);

	users = iMap->usercount();
	
	WFIFOHEAD(char_fd, 6+8*users);
	WFIFOW(char_fd,0) = 0x2aff;
	
	iter = mapit_getallusers();
	
	for( sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); sd = (TBL_PC*)mapit->next(iter) ) {
		WFIFOL(char_fd,6+8*i) = sd->status.account_id;
		WFIFOL(char_fd,6+8*i+4) = sd->status.char_id;
		i++;
	}
	
	mapit->free(iter);
	
	WFIFOW(char_fd,2) = 6 + 8*users;
	WFIFOW(char_fd,4) = users;
	WFIFOSET(char_fd, 6+8*users);

	return 0;
}

/*==========================================
 * timerFunction
  * Chk the connection to char server, (if it down)
 *------------------------------------------*/
static int check_connect_char_server(int tid, unsigned int tick, int id, intptr_t data) {
	static int displayed = 0;
	if ( char_fd <= 0 || session[char_fd] == NULL ) {
		if ( !displayed ) {
			ShowStatus("Attempting to connect to Char Server. Please wait.\n");
			displayed = 1;
		}

		chrif_state = 0;
		
		if ( ( char_fd = make_connection(char_ip, char_port,NULL) ) == -1) //Attempt to connect later. [Skotlex]
			return 0;

		session[char_fd]->func_parse = chrif_parse;
		session[char_fd]->flag.server = 1;
		realloc_fifo(char_fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);

		chrif_connect(char_fd);
		chrif->connected = (chrif_state == 2);
		srvinfo = 0;
	} else {
		if (srvinfo == 0) {
			chrif->ragsrvinfo(battle_config.base_exp_rate, battle_config.job_exp_rate, battle_config.item_rate_common);
			srvinfo = 1;
		}
	}
	if ( chrif->isconnected() )
		displayed = 0;
	return 0;
}

/*==========================================
 * Asks char server to remove friend_id from the friend list of char_id
 *------------------------------------------*/
int chrif_removefriend(int char_id, int friend_id) {

	chrif_check(-1);

	WFIFOHEAD(char_fd,10);
	WFIFOW(char_fd,0) = 0x2b07;
	WFIFOL(char_fd,2) = char_id;
	WFIFOL(char_fd,6) = friend_id;
	WFIFOSET(char_fd,10);
	
	return 0;
}

void chrif_send_report(char* buf, int len) {
#ifndef STATS_OPT_OUT
	if( char_fd ) {
		WFIFOHEAD(char_fd,len + 2);
		
		WFIFOW(char_fd,0) = 0x3008;
		
		memcpy(WFIFOP(char_fd,2), buf, len);
		
		WFIFOSET(char_fd,len + 2);
		
		flush_fifo(char_fd); /* ensure it's sent now. */
	}
#endif
}

/**
 * @see DBApply
 */
int auth_db_final(DBKey key, DBData *data, va_list ap) {
	struct auth_node *node = DB->data2ptr(data);
	
	if (node->char_dat)
		aFree(node->char_dat);
	
	if (node->sd)
		aFree(node->sd);
	
	ers_free(auth_db_ers, node);
	
	return 0;
}

/*==========================================
 * Destructor
 *------------------------------------------*/
int do_final_chrif(void) {
	
	if( char_fd != -1 ) {
		do_close(char_fd);
		char_fd = -1;
	}

	auth_db->destroy(auth_db, auth_db_final);
	
	ers_destroy(auth_db_ers);
	
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int do_init_chrif(void) {
	
	auth_db = idb_alloc(DB_OPT_BASE);
	auth_db_ers = ers_new(sizeof(struct auth_node),"chrif.c::auth_db_ers",ERS_OPT_NONE);

	iTimer->add_timer_func_list(check_connect_char_server, "check_connect_char_server");
	iTimer->add_timer_func_list(auth_db_cleanup, "auth_db_cleanup");

	// establish map-char connection if not present
	iTimer->add_timer_interval(iTimer->gettick() + 1000, check_connect_char_server, 0, 0, 10 * 1000);

	// wipe stale data for timed-out client connection requests
	iTimer->add_timer_interval(iTimer->gettick() + 1000, auth_db_cleanup, 0, 0, 30 * 1000);

	// send the user count every 10 seconds, to hide the charserver's online counting problem
	iTimer->add_timer_interval(iTimer->gettick() + 1000, send_usercount_tochar, 0, 0, UPDATE_INTERVAL);

	return 0;
}


/*=====================================
* Default Functions : chrif.h 
* Generated by HerculesInterfaceMaker
* created by Susu
*-------------------------------------*/
void chrif_defaults(void) {
	chrif = &chrif_s;

	/* vars */
	
	chrif->connected = 0;
	chrif->other_mapserver_count = 0;

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
	chrif->char_offline = chrif_char_offline;
	chrif->char_offline_nsd = chrif_char_offline_nsd;
	chrif->char_reset_offline = chrif_char_reset_offline;
	chrif->send_users_tochar = send_users_tochar;
	chrif->char_online = chrif_char_online;
	chrif->changesex = chrif_changesex;
	//chrif->chardisconnect = chrif_chardisconnect;
	chrif->divorce = chrif_divorce;
	
	chrif->removefriend = chrif_removefriend;
	chrif->send_report = chrif_send_report;
	
	chrif->do_final_chrif = do_final_chrif;
	chrif->do_init_chrif = do_init_chrif;
	
	chrif->flush_fifo = chrif_flush_fifo;
	chrif->skillid2idx = chrif_skillid2idx;
}
