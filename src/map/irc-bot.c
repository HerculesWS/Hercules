// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Base Author: shennetsind @ http://hercules.ws

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/random.h"

#include "map.h"
#include "pc.h"
#include "clif.h"
#include "irc-bot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define IRCBOT_DEBUG

struct irc_bot_interface irc_bot_s;

char send_string[IRC_MESSAGE_LENGTH];

/**
 * Timer callback to (re-)connect to an IRC server
 * @see timer->do_timer
 */
int irc_connect_timer(int tid, int64 tick, int id, intptr_t data) {
	struct hSockOpt opt;
	if( ircbot->isOn || ++ircbot->fails >= 3 )
		return 0;
	
	opt.silent = 1;
	opt.setTimeo = 0;
	
	ircbot->last_try = timer->gettick();

	if( ( ircbot->fd = make_connection(ircbot->ip,hChSys.irc_server_port,&opt) ) > 0 ){
		session[ircbot->fd]->func_parse = ircbot->parse;
		session[ircbot->fd]->flag.server = 1;
		timer->add(timer->gettick() + 3000, ircbot->identify_timer, 0, 0);
		ircbot->isOn = true;
	}
	return 0;
}

/**
 * Timer callback to send identification commands to an IRC server
 * @see timer->do_timer
 */
int irc_identify_timer(int tid, int64 tick, int id, intptr_t data) {
	if( !ircbot->isOn )
		return 0;
	
	sprintf(send_string, "USER HerculesWS%d 8 * : Hercules IRC Bridge",rand()%777);
	ircbot->send(send_string);
	sprintf(send_string, "NICK %s", hChSys.irc_nick);
	ircbot->send(send_string);

	timer->add(timer->gettick() + 3000, ircbot->join_timer, 0, 0);
	
	return 0;
}

/**
 * Timer callback to join channels (and optionally send NickServ commands)
 * @see timer->do_timer
 */
int irc_join_timer(int tid, int64 tick, int id, intptr_t data) {
	if( !ircbot->isOn )
		return 0;
	
	if( hChSys.irc_nick_pw[0] != '\0' ) {
		sprintf(send_string, "PRIVMSG NICKSERV : IDENTIFY %s", hChSys.irc_nick_pw);
		ircbot->send(send_string);
		if( hChSys.irc_use_ghost ) {
			sprintf(send_string, "PRIVMSG NICKSERV : GHOST %s %s", hChSys.irc_nick, hChSys.irc_nick_pw);
		}
	}
	
	sprintf(send_string, "JOIN %s", hChSys.irc_channel);
	ircbot->send(send_string);
	ircbot->isIn = true;

	return 0;
}

/**
 * Search the handler for a given IRC received command
 * @param function_name Name of the received IRC command
 * @return              Function pointer to the command handler, NULL in case
 *                      of unhandled commands
 */
struct irc_func* irc_func_search(char* function_name) {
	int i;
	for(i = 0; i < ircbot->funcs.size; i++) {
		if( strcmpi(ircbot->funcs.list[i]->name, function_name) == 0 ) {
			return ircbot->funcs.list[i];
		}
	}
	return NULL;
}

/**
 * Parser for the IRC server connection
 * @see do_sockets
 */
int irc_parse(int fd) {
	char *parse_string = NULL, *str_safe = NULL;

	if (session[fd]->flag.eof) {
		do_close(fd);
		ircbot->fd = 0;
		ircbot->isOn = false;
		ircbot->isIn = false;
		ircbot->fails = 0;
		ircbot->ip = host2ip(hChSys.irc_server);
		timer->add(timer->gettick() + 120000, ircbot->connect_timer, 0, 0);
      	return 0;
	}
	
	if( !RFIFOREST(fd) )
		return 0;
	
	parse_string = (char*)RFIFOP(fd,0);
	parse_string[ RFIFOREST(fd) - 1 ] = '\0';
	
	parse_string = strtok_r(parse_string,"\r\n",&str_safe);
	
	while (parse_string != NULL) {
		ircbot->parse_sub(fd,parse_string);
		parse_string = strtok_r(NULL,"\r\n",&str_safe);
	}
	
	RFIFOSKIP(fd, RFIFOREST(fd));
	RFIFOFLUSH(fd);
	return 0;
}

/**
 * Parse the source from a received irc message
 * @param source Source string, as reported by the server
 * @param nick   Pointer to a string where to return the nick (may not be NULL,
 *               needs to be able to fit an IRC_NICK_LENGTH long string)
 * @param ident  Pointer to a string where to return the ident (may not be
 *               NULL, needs to be able to fit an IRC_IDENT_LENGTH long string)
 * @param host   Pointer to a string where to return the hostname (may not be
 *               NULL, needs to be able to fit an IRC_HOST_LENGTH long string)
 */
void irc_parse_source(char *source, char *nick, char *ident, char *host) {
	int i, pos = 0;
	size_t len = strlen(source);
	unsigned char stage = 0;
	
	for(i = 0; i < len; i++) {
		if( stage == 0 && source[i] == '!' ) {
			safestrncpy(nick, &source[0], min(i + 1, IRC_NICK_LENGTH));
			pos = i+1;
			stage = 1;
		} else if( stage == 1 && source[i] == '@' ) {
			safestrncpy(ident, &source[pos], min(i - pos + 1, IRC_IDENT_LENGTH));
			safestrncpy(host, &source[i+1], min(len - i, IRC_HOST_LENGTH));
			break;
		}
	}
}

/**
 * Parse a received message from the irc server, and do the appropriate action
 * for the detected command
 * @param fd  IRC server connection file descriptor
 * @param str Raw received message
 */
void irc_parse_sub(int fd, char *str) {
	char source[180], command[60], buf1[IRC_MESSAGE_LENGTH], buf2[IRC_MESSAGE_LENGTH];
	char *target = buf1, *message = buf2;
	struct irc_func *func;
	
	source[0] = command[0] = buf1[0] = buf2[0] = '\0';
	
	if( str[0] == ':' )
		str++;
	
	if (sscanf(str, "%179s %59s %499s :%499[^\r\n]", source, command, buf1, buf2) == 3 && buf1[0] == ':') {
		// source command :message (i.e. QUIT)
		message = buf1+1;
		target = buf2;
	}
		
	if( command[0] == '\0' )
		return;
		
	if( !(func = ircbot->func_search(command)) && !(func = ircbot->func_search(source)) ) {
#ifdef IRCBOT_DEBUG
		ShowWarning("Unknown command received %s from %s\n",command,source);
#endif // IRCBOT_DEBUG
		return;
	}
	func->func(fd,command,source,target,message);
	
}

/**
 * Send a raw command to the irc server
 * @param str Command to send
 */
void irc_send(char *str) {
	size_t len = strlen(str) + 2;
	if (len > IRC_MESSAGE_LENGTH-3)
		len = IRC_MESSAGE_LENGTH-3;
	WFIFOHEAD(ircbot->fd, len);
	snprintf((char*)WFIFOP(ircbot->fd,0),IRC_MESSAGE_LENGTH, "%s\r\n", str);
	WFIFOSET(ircbot->fd, len);
}

/**
 * Handler for the PING IRC command (send back a PONG)
 * @see irc_parse_sub
 */
void irc_pong(int fd, char *cmd, char *source, char *target, char *msg) {
	sprintf(send_string, "PONG %s", cmd);
	ircbot->send(send_string);
}

/**
 * Handler for CTCP commands received via PRIVMSG
 * @see irc_privmsg
 */
void irc_privmsg_ctcp(int fd, char *cmd, char *source, char *target, char *msg) {
	char source_nick[IRC_NICK_LENGTH], source_ident[IRC_IDENT_LENGTH], source_host[IRC_HOST_LENGTH];

	source_nick[0] = source_ident[0] = source_host[0] = '\0';

	if( source[0] != '\0' )
		ircbot->parse_source(source,source_nick,source_ident,source_host);

	if( strcmpi(cmd,"ACTION") == 0 ) {
		if( ircbot->channel ) {
			snprintf(send_string, 150, "[ #%s ] * IRC.%s %s *",ircbot->channel->name,source_nick,msg);
			clif->chsys_msg2(ircbot->channel,send_string);
		}
	} else if( strcmpi(cmd,"ERRMSG") == 0 ) {
		// Ignore it
	} else if( strcmpi(cmd,"FINGER") == 0 ) {
		// Ignore it
	} else if( strcmpi(cmd,"PING") == 0 ) {
		sprintf(send_string, "NOTICE %s :\001PING %s\001",source_nick,msg);
		ircbot->send(send_string);
	} else if( strcmpi(cmd,"TIME") == 0 ) {
		time_t time_server;  // variable for number of seconds (used with time() function)
		struct tm *datetime; // variable for time in structure ->tm_mday, ->tm_sec, ...
		char temp[CHAT_SIZE_MAX];

		memset(temp, '\0', sizeof(temp));

		time(&time_server);  // get time in seconds since 1/1/1970
		datetime = localtime(&time_server); // convert seconds in structure
		// like sprintf, but only for date/time (Sunday, November 02 2003 15:12:52)
		strftime(temp, sizeof(temp)-1, msg_txt(230), datetime); // Server time (normal time): %A, %B %d %Y %X.

		sprintf(send_string, "NOTICE %s :\001TIME %s\001",source_nick,temp);
		ircbot->send(send_string);
	} else if( strcmpi(cmd,"VERSION") == 0 ) {
		sprintf(send_string, "NOTICE %s :\001VERSION Hercules.ws IRC Bridge\001",source_nick);
		ircbot->send(send_string);
#ifdef IRCBOT_DEBUG
	} else {
		ShowWarning("Unknown CTCP command received %s (%s) from %s\n",cmd,msg,source);
#endif // IRCBOT_DEBUG
	}
}

/**
 * Handler for the PRIVMSG IRC command (action depends on the message contents)
 * @see irc_parse_sub
 */
void irc_privmsg(int fd, char *cmd, char *source, char *target, char *msg) {
	if( msg && *msg == '\001' && strlen(msg) > 2 && msg[strlen(msg)-1] == '\001' ) {
		// CTCP
		char command[IRC_MESSAGE_LENGTH], message[IRC_MESSAGE_LENGTH];
		command[0] = message[0] = '\0';
		sscanf(msg, "\001%499[^\001\r\n ] %499[^\r\n\001]\001", command, message);

		irc_privmsg_ctcp(fd, command, source, target, message);
#ifdef IRCBOT_DEBUG
	} else if( strcmpi(target,hChSys.irc_nick) == 0 ) {
		ShowDebug("irc_privmsg: Received message from %s: '%s'\n", source ? source : "(null)", msg);
#endif // IRCBOT_DEBUG
	} else if( msg && strcmpi(target,hChSys.irc_channel) == 0 ) {
		char source_nick[IRC_NICK_LENGTH], source_ident[IRC_IDENT_LENGTH], source_host[IRC_HOST_LENGTH];

		source_nick[0] = source_ident[0] = source_host[0] = '\0';

		if( source[0] != '\0' )
			ircbot->parse_source(source,source_nick,source_ident,source_host);
				
		if( ircbot->channel ) {
			size_t padding_len = strlen(ircbot->channel->name) + strlen(source_nick) + 13;
			while (1) {
				snprintf(send_string, 150, "[ #%s ] IRC.%s : %s",ircbot->channel->name,source_nick,msg);
				clif->chsys_msg2(ircbot->channel,send_string);
				//break; // Uncomment this line to truncate long messages instead of posting them as multiple lines
				if (strlen(msg) <= 149 - padding_len)
					break;
				msg += 149 - padding_len;
			}
		}
	}
}

/**
 * Handler for the JOIN IRC command (notify an in-game channel of users joining
 * the IRC channel)
 * @see irc_parse_sub
 */
void irc_userjoin(int fd, char *cmd, char *source, char *target, char *msg) {
	char source_nick[IRC_NICK_LENGTH], source_ident[IRC_IDENT_LENGTH], source_host[IRC_HOST_LENGTH];

	source_nick[0] = source_ident[0] = source_host[0] = '\0';

	if( source[0] != '\0' )
		ircbot->parse_source(source,source_nick,source_ident,source_host);

	if( ircbot->channel ) {
		snprintf(send_string, 150, "[ #%s ] User IRC.%s joined the channel.",ircbot->channel->name,source_nick);
		clif->chsys_msg2(ircbot->channel,send_string);
	}
}

/**
 * Handler for the PART and QUIT IRC commands (notify an in-game channel of
 * users leaving the IRC channel)
 * @see irc_parse_sub
 */
void irc_userleave(int fd, char *cmd, char *source, char *target, char *msg) {
	char source_nick[IRC_NICK_LENGTH], source_ident[IRC_IDENT_LENGTH], source_host[IRC_HOST_LENGTH];

	source_nick[0] = source_ident[0] = source_host[0] = '\0';

	if( source[0] != '\0' )
		ircbot->parse_source(source,source_nick,source_ident,source_host);

	if( ircbot->channel ) {
		if (!strcmpi(cmd, "QUIT"))
			snprintf(send_string, 150, "[ #%s ] User IRC.%s left the channel. [Quit: %s]",ircbot->channel->name,source_nick,msg);
		else
			snprintf(send_string, 150, "[ #%s ] User IRC.%s left the channel. [%s]",ircbot->channel->name,source_nick,msg);
		clif->chsys_msg2(ircbot->channel,send_string);
	}
}

/**
 * Handler for the NICK IRC commands (notify an in-game channel of users
 * changing their name while in the IRC channel)
 * @see irc_parse_sub
 */
void irc_usernick(int fd, char *cmd, char *source, char *target, char *msg) {
	char source_nick[IRC_NICK_LENGTH], source_ident[IRC_IDENT_LENGTH], source_host[IRC_HOST_LENGTH];

	source_nick[0] = source_ident[0] = source_host[0] = '\0';

	if( source[0] != '\0' )
		ircbot->parse_source(source,source_nick,source_ident,source_host);

	if( ircbot->channel ) {
		snprintf(send_string, 150, "[ #%s ] User IRC.%s is now known as IRC.%s",ircbot->channel->name,source_nick,msg);
		clif->chsys_msg2(ircbot->channel,send_string);
	}
}

/**
 * Relay a chat message to the irc channel the bot is connected to
 * @param name Sender's name
 * @param msg  Message text
 */
void irc_relay(char *name, const char *msg) {
	if( !ircbot->isIn )
		return;
	sprintf(send_string,"PRIVMSG %s :[ %s ] : %s",hChSys.irc_channel,name,msg);
	ircbot->send(send_string);
}

/**
 * IRC bot initializer
 */
void irc_bot_init(bool minimal) {
	/// Command handlers
	const struct irc_func irc_func_base[] = {
		{ "PING" , ircbot->pong },
		{ "PRIVMSG", ircbot->privmsg },
		{ "JOIN", ircbot->userjoin },
		{ "QUIT", ircbot->userleave },
		{ "PART", ircbot->userleave },
		{ "NICK", ircbot->usernick },
	};
	struct irc_func* function;
	int i;

	if (minimal)
		return;

	if( !hChSys.irc )
		return;

	if (!(ircbot->ip = host2ip(hChSys.irc_server))) {
		ShowError("Unable to resolve '%s' (irc server), disabling irc channel...\n", hChSys.irc_server);
		hChSys.irc = false;
		return;
	}
	
	ircbot->funcs.size = ARRAYLENGTH(irc_func_base);

	CREATE(ircbot->funcs.list,struct irc_func*,ircbot->funcs.size);
	
	for( i = 0; i < ircbot->funcs.size; i++ ) {
		
		CREATE(function, struct irc_func, 1);
		
		safestrncpy(function->name, irc_func_base[i].name, sizeof(function->name));
		function->func = irc_func_base[i].func;
		
		ircbot->funcs.list[i] = function;
	}
	
	ircbot->fails = 0;
	ircbot->fd = 0;
	ircbot->isIn = false;
	ircbot->isOn = false;
	
	timer->add_func_list(ircbot->connect_timer, "irc_connect_timer");
	timer->add(timer->gettick() + 7000, ircbot->connect_timer, 0, 0);
}

/**
 * IRC bot finalizer
 */
void irc_bot_final(void) {
	int i;
	
	if( !hChSys.irc )
		return;
	if( ircbot->isOn ) {
		ircbot->send("QUIT :Hercules is shutting down");
		do_close(ircbot->fd);
	}
	
	for( i = 0; i < ircbot->funcs.size; i++ ) {
		aFree(ircbot->funcs.list[i]);
	}
	aFree(ircbot->funcs.list);
}

/**
 * IRC bot interface defaults initializer
 */
void ircbot_defaults(void) {
	ircbot = &irc_bot_s;
	
	ircbot->channel = NULL;
	
	ircbot->init = irc_bot_init;
	ircbot->final = irc_bot_final;
	
	ircbot->parse = irc_parse;
	ircbot->parse_sub = irc_parse_sub;
	ircbot->parse_source = irc_parse_source;
	
	ircbot->func_search = irc_func_search;
	
	ircbot->connect_timer = irc_connect_timer;
	ircbot->identify_timer = irc_identify_timer;
	ircbot->join_timer = irc_join_timer;
	
	ircbot->send = irc_send;
	ircbot->relay = irc_relay;
	
	ircbot->pong = irc_pong;
	ircbot->privmsg = irc_privmsg;

	ircbot->userjoin = irc_userjoin;
	ircbot->userleave = irc_userleave;
	ircbot->usernick = irc_usernick;
}
