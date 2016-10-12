/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-2015  Hercules Dev Team
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

/**
 * Base Author: shennetsind @ http://herc.ws
 */
#define HERCULES_CORE

#include "irc-bot.h"

#include "map/channel.h"
#include "map/map.h"
#include "map/pc.h"
#include "common/cbasetypes.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/random.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define IRCBOT_DEBUG

struct irc_bot_interface irc_bot_s;
struct irc_bot_interface *ircbot;

char send_string[IRC_MESSAGE_LENGTH];

/// @copydoc irc_bot_interface::connect_timer()
int irc_connect_timer(int tid, int64 tick, int id, intptr_t data) {
	struct hSockOpt opt;
	if( ircbot->isOn || ++ircbot->fails >= 3 )
		return 0;

	opt.silent = 1;
	opt.setTimeo = 0;

	ircbot->last_try = timer->gettick();

	if ((ircbot->fd = sockt->make_connection(ircbot->ip, channel->config->irc_server_port, &opt)) > 0) {
		sockt->session[ircbot->fd]->func_parse = ircbot->parse;
		sockt->session[ircbot->fd]->flag.server = 1;
		timer->add(timer->gettick() + 3000, ircbot->identify_timer, 0, 0);
		ircbot->isOn = true;
	}
	return 0;
}

/// @copydoc irc_bot_interface::identify_timer()
int irc_identify_timer(int tid, int64 tick, int id, intptr_t data) {
	if( !ircbot->isOn )
		return 0;

	sprintf(send_string, "USER HerculesWS%d 8 * : Hercules IRC Bridge",rnd()%777);
	ircbot->send(send_string, true);
	sprintf(send_string, "NICK %s", channel->config->irc_nick);
	ircbot->send(send_string, true);

	timer->add(timer->gettick() + 3000, ircbot->join_timer, 0, 0);

	return 0;
}

/// @copydoc irc_bot_interface::join_timer()
int irc_join_timer(int tid, int64 tick, int id, intptr_t data) {
	if( !ircbot->isOn )
		return 0;

	if (channel->config->irc_nick_pw[0] != '\0') {
		sprintf(send_string, "PRIVMSG NICKSERV : IDENTIFY %s", channel->config->irc_nick_pw);
		ircbot->send(send_string, true);
		if (channel->config->irc_use_ghost) {
			sprintf(send_string, "PRIVMSG NICKSERV : GHOST %s %s", channel->config->irc_nick, channel->config->irc_nick_pw);
			ircbot->send(send_string, true);
		}
	}

	sprintf(send_string, "JOIN %s", channel->config->irc_channel);
	ircbot->send(send_string, true);
	ircbot->isIn = true;

	return 0;
}

/// @copydoc irc_bot_interface::func_search()
struct irc_func* irc_func_search(char* function_name) {
	int i;
	nullpo_retr(NULL, function_name);
	for(i = 0; i < ircbot->funcs.size; i++) {
		if( strcmpi(ircbot->funcs.list[i]->name, function_name) == 0 ) {
			return ircbot->funcs.list[i];
		}
	}
	return NULL;
}

/// @copydoc irc_bot_interface::parse()
int irc_parse(int fd) {
	char *parse_string = NULL, *p = NULL, *str_safe = NULL;

	if (sockt->session[fd]->flag.eof) {
		sockt->close(fd);
		ircbot->fd = 0;
		ircbot->isOn = false;
		ircbot->isIn = false;
		ircbot->fails = 0;
		ircbot->ip = sockt->host2ip(channel->config->irc_server);
		timer->add(timer->gettick() + 120000, ircbot->connect_timer, 0, 0);
		return 0;
	}

	if( !RFIFOREST(fd) )
		return 0;

	parse_string = aMalloc(RFIFOREST(fd));
	safestrncpy(parse_string, RFIFOP(fd,0), RFIFOREST(fd));
	RFIFOSKIP(fd, RFIFOREST(fd));
	RFIFOFLUSH(fd);

	p = strtok_r(parse_string,"\r\n",&str_safe);

	while (p != NULL) {
		ircbot->parse_sub(fd,parse_string);
		p = strtok_r(NULL,"\r\n",&str_safe);
	}
	aFree(parse_string);

	return 0;
}

/// @copydoc irc_bot_interface::parse_source()
void irc_parse_source(char *source, char *nick, char *ident, char *host) {
	int i, pos = 0;
	size_t len;
	unsigned char stage = 0;

	nullpo_retv(source);
	len = strlen(source);
	nullpo_retv(nick);
	nullpo_retv(ident);
	nullpo_retv(host);
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

/// @copydoc irc_bot_interface::parse_sub()
void irc_parse_sub(int fd, char *str) {
	char source[180], command[60], buf1[IRC_MESSAGE_LENGTH], buf2[IRC_MESSAGE_LENGTH];
	char *target = buf1, *message = buf2;
	struct irc_func *func;

	nullpo_retv(str);
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

	if ((func = ircbot->func_search(command)) == NULL && (func = ircbot->func_search(source)) == NULL) {
#ifdef IRCBOT_DEBUG
		ShowWarning("Unknown command received %s from %s\n",command,source);
#endif // IRCBOT_DEBUG
		return;
	}
	func->func(fd,command,source,target,message);
}

/// @copydoc irc_bot_interface::queue()
void irc_queue(char *str)
{
	struct message_flood *queue_entry = NULL;

	if (!ircbot->flood_protection_enabled) {
		ircbot->send(str, true);
		return;
	}

	if (ircbot->message_current == NULL) {
		// No queue yet
		if (ircbot->messages_burst_count < ircbot->flood_protection_burst) {
			ircbot->send(str, true);
			if (DIFF_TICK(timer->gettick(), ircbot->last_message_tick) <= ircbot->flood_protection_rate)
				ircbot->messages_burst_count++;
			else
				ircbot->messages_burst_count = 0;
			ircbot->last_message_tick = timer->gettick();
		} else { //queue starts
			CREATE(queue_entry, struct message_flood, 1);
			safestrncpy(queue_entry->message, str, sizeof(queue_entry->message));
			queue_entry->next = NULL;
			ircbot->message_current = queue_entry;
			ircbot->message_last = queue_entry;
			ircbot->queue_tid = timer->add(timer->gettick() + ircbot->flood_protection_rate, ircbot->queue_timer, 0, 0); //start queue timer
			ircbot->messages_burst_count = 0;
		}
	} else {
		CREATE(queue_entry, struct message_flood, 1);
		safestrncpy(queue_entry->message, str, sizeof(queue_entry->message));
		queue_entry->next = NULL;
		ircbot->message_last->next = queue_entry;
		ircbot->message_last = queue_entry;
	}
}

/// @copydoc irc_bot_interface::queue_timer()
int irc_queue_timer(int tid, int64 tick, int id, intptr_t data)
{
	struct message_flood *queue_entry = ircbot->message_current;
	nullpo_ret(queue_entry);

	ircbot->send(queue_entry->message, true);
	if (queue_entry->next != NULL) {
		ircbot->message_current = queue_entry->next;
		ircbot->queue_tid = timer->add(timer->gettick() + ircbot->flood_protection_rate, ircbot->queue_timer, 0, 0);
	} else {
		ircbot->message_current = NULL;
		ircbot->message_last = NULL;
		ircbot->queue_tid = INVALID_TIMER;
	}

	aFree(queue_entry);

	return 0;
}

/// @copydoc irc_bot_interface::send()
void irc_send(char *str, bool force)
{
	size_t len;
	nullpo_retv(str);
	len = strlen(str) + 2;
	if (len > IRC_MESSAGE_LENGTH-3)
		len = IRC_MESSAGE_LENGTH-3;

	if (!force && ircbot->flood_protection_enabled) {
		// Add to queue
		ircbot->queue(str);
		return;
	}

	WFIFOHEAD(ircbot->fd, len);
	snprintf(WFIFOP(ircbot->fd,0),IRC_MESSAGE_LENGTH, "%s\r\n", str);
	WFIFOSET(ircbot->fd, len);
}

/// @copydoc irc_interface_bot::pong()
void irc_pong(int fd, char *cmd, char *source, char *target, char *msg) {
	nullpo_retv(cmd);
	snprintf(send_string, IRC_MESSAGE_LENGTH, "PONG %s", cmd);
	ircbot->send(send_string, false);
}

/// @copydoc irc_interface_bot::privmsg_ctcp()
void irc_privmsg_ctcp(int fd, char *cmd, char *source, char *target, char *msg) {
	char source_nick[IRC_NICK_LENGTH], source_ident[IRC_IDENT_LENGTH], source_host[IRC_HOST_LENGTH];

	source_nick[0] = source_ident[0] = source_host[0] = '\0';

	nullpo_retv(source);
	if( source[0] != '\0' )
		ircbot->parse_source(source,source_nick,source_ident,source_host);

	if( strcmpi(cmd,"ACTION") == 0 ) {
		if( ircbot->channel ) {
			snprintf(send_string, 150, "[ #%s ] * IRC.%s %s *",ircbot->channel->name,source_nick,msg);
			clif->channel_msg2(ircbot->channel,send_string);
		}
	} else if( strcmpi(cmd,"ERRMSG") == 0 ) {
		// Ignore it
	} else if( strcmpi(cmd,"FINGER") == 0 ) {
		// Ignore it
	} else if( strcmpi(cmd,"PING") == 0 ) {
		snprintf(send_string, IRC_MESSAGE_LENGTH, "NOTICE %s :\001PING %s\001",source_nick,msg);
		ircbot->send(send_string, false);
	} else if( strcmpi(cmd,"TIME") == 0 ) {
		time_t time_server;  // variable for number of seconds (used with time() function)
		struct tm *datetime; // variable for time in structure ->tm_mday, ->tm_sec, ...
		char temp[CHAT_SIZE_MAX];

		memset(temp, '\0', sizeof(temp));

		time(&time_server);  // get time in seconds since 1/1/1970
		datetime = localtime(&time_server); // convert seconds in structure
		// like sprintf, but only for date/time (Sunday, November 02 2003 15:12:52)
		strftime(temp, sizeof(temp)-1, msg_txt(230), datetime); // Server time (normal time): %A, %B %d %Y %X.

		snprintf(send_string, IRC_MESSAGE_LENGTH, "NOTICE %s :\001TIME %s\001",source_nick,temp);
		ircbot->send(send_string, false);
	} else if( strcmpi(cmd,"VERSION") == 0 ) {
		snprintf(send_string, IRC_MESSAGE_LENGTH, "NOTICE %s :\001VERSION Herc.ws IRC Bridge\001",source_nick);
		ircbot->send(send_string, false);
#ifdef IRCBOT_DEBUG
	} else {
		ShowWarning("Unknown CTCP command received %s (%s) from %s\n",cmd,msg,source);
#endif // IRCBOT_DEBUG
	}
}

/// @copydoc irc_bot_interface::privmsg()
void irc_privmsg(int fd, char *cmd, char *source, char *target, char *msg) {
	size_t len = msg ? strlen(msg) : 0;
	nullpo_retv(source);
	nullpo_retv(target);
	if (msg && *msg == '\001' && len > 2 && msg[len - 1] == '\001') {
		// CTCP
		char command[IRC_MESSAGE_LENGTH], message[IRC_MESSAGE_LENGTH];
		command[0] = message[0] = '\0';
		sscanf(msg, "\001%499[^\001\r\n ] %499[^\r\n\001]\001", command, message);

		ircbot->privmsg_ctcp(fd, command, source, target, message);
#ifdef IRCBOT_DEBUG
	} else if (strcmpi(target, channel->config->irc_nick) == 0) {
		ShowDebug("irc_privmsg: Received message from %s: '%s'\n", source ? source : "(null)", msg);
#endif // IRCBOT_DEBUG
	} else if (msg && strcmpi(target, channel->config->irc_channel) == 0) {
		char source_nick[IRC_NICK_LENGTH], source_ident[IRC_IDENT_LENGTH], source_host[IRC_HOST_LENGTH];

		source_nick[0] = source_ident[0] = source_host[0] = '\0';

		if( source[0] != '\0' )
			ircbot->parse_source(source,source_nick,source_ident,source_host);

		if( ircbot->channel ) {
			size_t padding_len = strlen(ircbot->channel->name) + strlen(source_nick) + 13;
			while (1) {
				snprintf(send_string, 150, "[ #%s ] IRC.%s : %s",ircbot->channel->name,source_nick,msg);
				clif->channel_msg2(ircbot->channel,send_string);
				//break; // Uncomment this line to truncate long messages instead of posting them as multiple lines
				if (strlen(msg) <= 149 - padding_len)
					break;
				msg += 149 - padding_len;
			}
		}
	}
}

/// @copydoc irc_bot_interface::userjoin()
void irc_userjoin(int fd, char *cmd, char *source, char *target, char *msg) {
	char source_nick[IRC_NICK_LENGTH], source_ident[IRC_IDENT_LENGTH], source_host[IRC_HOST_LENGTH];

	nullpo_retv(source);
	source_nick[0] = source_ident[0] = source_host[0] = '\0';

	if( source[0] != '\0' )
		ircbot->parse_source(source,source_nick,source_ident,source_host);

	if( ircbot->channel ) {
		snprintf(send_string, 150, "[ #%s ] User IRC.%s joined the channel.",ircbot->channel->name,source_nick);
		clif->channel_msg2(ircbot->channel,send_string);
	}
}

/// @copydoc irc_bot_interface::userleave()
void irc_userleave(int fd, char *cmd, char *source, char *target, char *msg) {
	char source_nick[IRC_NICK_LENGTH], source_ident[IRC_IDENT_LENGTH], source_host[IRC_HOST_LENGTH];

	nullpo_retv(source);
	source_nick[0] = source_ident[0] = source_host[0] = '\0';

	if( source[0] != '\0' )
		ircbot->parse_source(source,source_nick,source_ident,source_host);

	if( ircbot->channel ) {
		if (!strcmpi(cmd, "QUIT"))
			snprintf(send_string, 150, "[ #%s ] User IRC.%s left the channel. [Quit: %s]",ircbot->channel->name,source_nick,msg);
		else
			snprintf(send_string, 150, "[ #%s ] User IRC.%s left the channel. [%s]",ircbot->channel->name,source_nick,msg);
		clif->channel_msg2(ircbot->channel,send_string);
	}
}

/// @copydoc irc_bot_interface::usernick()
void irc_usernick(int fd, char *cmd, char *source, char *target, char *msg) {
	char source_nick[IRC_NICK_LENGTH], source_ident[IRC_IDENT_LENGTH], source_host[IRC_HOST_LENGTH];

	nullpo_retv(source);
	source_nick[0] = source_ident[0] = source_host[0] = '\0';

	if( source[0] != '\0' )
		ircbot->parse_source(source,source_nick,source_ident,source_host);

	if( ircbot->channel ) {
		snprintf(send_string, 150, "[ #%s ] User IRC.%s is now known as IRC.%s",ircbot->channel->name,source_nick,msg);
		clif->channel_msg2(ircbot->channel,send_string);
	}
}

/// @copydoc irc_bot_interface::relay()
void irc_relay(const char *name, const char *msg)
{
	if (!ircbot->isIn)
		return;

	nullpo_retv(msg);
	if (name)
		sprintf(send_string,"PRIVMSG %s :[ %s ] : %s", channel->config->irc_channel, name, msg);
	else
		sprintf(send_string,"PRIVMSG %s :%s", channel->config->irc_channel, msg);

	ircbot->send(send_string, false);
}

/// @copydoc irc_bot_interface::init()
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

	if (!channel->config->irc)
		return;

	if (!(ircbot->ip = sockt->host2ip(channel->config->irc_server))) {
		ShowError("Unable to resolve '%s' (irc server), disabling irc channel...\n", channel->config->irc_server);
		channel->config->irc = false;
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
	timer->add_func_list(ircbot->queue_timer, "irc_queue_timer");

	timer->add(timer->gettick() + 7000, ircbot->connect_timer, 0, 0);
}

/// @copydoc irc_bot_interface::final()
void irc_bot_final(void) {
	int i;

	if (!channel->config->irc)
		return;
	if( ircbot->isOn ) {
		ircbot->send("QUIT :Hercules is shutting down", true);
		sockt->close(ircbot->fd);
	}

	if (ircbot->queue_tid != INVALID_TIMER)
		timer->delete(ircbot->queue_tid, ircbot->queue_timer);

	while (ircbot->message_current != NULL) {
		struct message_flood *next = ircbot->message_current->next;
		aFree(ircbot->message_current);
		ircbot->message_current = next;
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

	ircbot->flood_protection_enabled = true;
	ircbot->flood_protection_rate = 1000;
	ircbot->flood_protection_burst = 3;
	ircbot->last_message_tick = INVALID_TIMER;
	ircbot->queue_tid = INVALID_TIMER;
	ircbot->messages_burst_count = 0;
	ircbot->message_current = NULL;
	ircbot->message_last = NULL;

	ircbot->init = irc_bot_init;
	ircbot->final = irc_bot_final;

	ircbot->parse = irc_parse;
	ircbot->parse_sub = irc_parse_sub;
	ircbot->parse_source = irc_parse_source;

	ircbot->func_search = irc_func_search;

	ircbot->connect_timer = irc_connect_timer;
	ircbot->identify_timer = irc_identify_timer;
	ircbot->join_timer = irc_join_timer;

	ircbot->queue_timer = irc_queue_timer;
	ircbot->queue = irc_queue;
	ircbot->send = irc_send;
	ircbot->relay = irc_relay;

	ircbot->pong = irc_pong;
	ircbot->privmsg = irc_privmsg;
	ircbot->privmsg_ctcp = irc_privmsg_ctcp;

	ircbot->userjoin = irc_userjoin;
	ircbot->userleave = irc_userleave;
	ircbot->usernick = irc_usernick;
}
