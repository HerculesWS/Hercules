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
#ifndef MAP_IRC_BOT_H
#define MAP_IRC_BOT_H

#include "common/hercules.h"

#define IRC_NICK_LENGTH 40
#define IRC_IDENT_LENGTH 40
#define IRC_HOST_LENGTH 63
#define IRC_FUNC_LENGTH 30
#define IRC_MESSAGE_LENGTH 500

struct channel_data;

struct irc_func {
	char name[IRC_FUNC_LENGTH];
	void (*func)(int, char*, char*, char*, char*);
};

struct message_flood {
	char message[IRC_MESSAGE_LENGTH];
	struct message_flood *next;
};

struct irc_bot_interface {
	int fd;
	bool isIn, isOn;
	int64 last_try;
	unsigned char fails;
	uint32 ip;
	unsigned short port;
	/* messages flood protection */
	bool flood_protection_enabled;
	int flood_protection_rate;
	int flood_protection_burst;
	int64 last_message_tick;
	int messages_burst_count;
	int queue_tid;
	struct message_flood *message_current;
	struct message_flood *message_last;
	/* */
	struct channel_data *channel;
	/* */
	struct {
		struct irc_func **list;
		unsigned int size;
	} funcs;
	/* */
	void (*init) (bool minimal);
	void (*final) (void);
	/* */
	int (*parse) (int fd);
	void (*parse_sub) (int fd, char *str);
	void (*parse_source) (char *source, char *nick, char *ident, char *host);
	/* */
	struct irc_func* (*func_search) (char* function_name);
	/* */
	int (*connect_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*identify_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*join_timer) (int tid, int64 tick, int id, intptr_t data);
	/* */
	int (*queue_timer) (int tid, int64 tick, int id, intptr_t data);
	void (*queue) (char *str);
	void (*send)(char *str, bool force);
	void (*relay) (const char *name, const char *msg);
	/* */
	void (*pong) (int fd, char *cmd, char *source, char *target, char *msg);
	void (*privmsg) (int fd, char *cmd, char *source, char *target, char *msg);
	void (*userjoin) (int fd, char *cmd, char *source, char *target, char *msg);
	void (*userleave) (int fd, char *cmd, char *source, char *target, char *msg);
	void (*usernick) (int fd, char *cmd, char *source, char *target, char *msg);
};

#ifdef HERCULES_CORE
void ircbot_defaults(void);
#endif // HERCULES_CORE

HPShared struct irc_bot_interface *ircbot;

#endif /* MAP_IRC_BOT_H */
