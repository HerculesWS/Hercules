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

	/**
	 * IRC bot initializer
	 */
	void (*init) (bool minimal);

	/**
	 * IRC bot finalizer
	 */
	void (*final) (void);

	/**
	 * Parser for the IRC server connection
	 * @see do_sockets
	 */
	int (*parse) (int fd);

	/**
	 * Parse a received message from the irc server, and do the appropriate action
	 * for the detected command
	 * @param fd  IRC server connection file descriptor
	 * @param str Raw received message
	 */
	void (*parse_sub) (int fd, char *str);

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
	void (*parse_source) (char *source, char *nick, char *ident, char *host);

	/**
	 * Search the handler for a given IRC received command
	 * @param function_name Name of the received IRC command
	 * @return              Function pointer to the command handler, NULL in case
	 *                      of unhandled commands
	 */
	struct irc_func* (*func_search) (char* function_name);

	/**
	 * Timer callback to (re-)connect to an IRC server
	 * @see timer_interface::do_timer
	 */
	int (*connect_timer) (int tid, int64 tick, int id, intptr_t data);

	/**
	 * Timer callback to send identification commands to an IRC server
	 * @see timer_interface::do_timer
	 */
	int (*identify_timer) (int tid, int64 tick, int id, intptr_t data);

	/**
	 * Timer callback to join channels (and optionally send NickServ commands)
	 * @see timer_interface::do_timer
	 */
	int (*join_timer) (int tid, int64 tick, int id, intptr_t data);

	/**
	 * Timer callback to send queued IRC Commands
	 * @see timer_interface::do_timer
	 */
	int (*queue_timer) (int tid, int64 tick, int id, intptr_t data);

	/**
	 * Decides if an IRC Command should be queued or not, based on the flood protection settings.
	 *
	 * @param str Command to be checked
	 */
	void (*queue) (char *str);

	/**
	 * Send a raw command to the irc server
	 * @param str Command to send
	 */
	void (*send)(char *str, bool force);

	/**
	 * Relay a chat message to the irc channel the bot is connected to
	 * @param name Sender's name
	 * @param msg  Message text
	 */
	void (*relay) (const char *name, const char *msg);

	/**
	 * Handler for the PING IRC command (send back a PONG)
	 * @see irc_bot_interface::parse_sub
	 */
	void (*pong) (int fd, char *cmd, char *source, char *target, char *msg);

	/**
	 * Handler for the PRIVMSG IRC command (action depends on the message contents)
	 * @see irc_bot_interface::parse_sub
	 */
	void (*privmsg) (int fd, char *cmd, char *source, char *target, char *msg);

	/**
	 * Handler for CTCP commands received via PRIVMSG
	 * @see irc_bot_interface::privmsg
	 */
	void (*privmsg_ctcp) (int fd, char *cmd, char *source, char *target, char *msg);

	/**
	 * Handler for the JOIN IRC command (notify an in-game channel of users joining
	 * the IRC channel)
	 * @see irc_bot_interface::parse_sub
	 */
	void (*userjoin) (int fd, char *cmd, char *source, char *target, char *msg);

	/**
	 * Handler for the PART and QUIT IRC commands (notify an in-game channel of
	 * users leaving the IRC channel)
	 * @see irc_bot_interface::parse_sub
	 */
	void (*userleave) (int fd, char *cmd, char *source, char *target, char *msg);

	/**
	 * Handler for the NICK IRC commands (notify an in-game channel of users
	 * changing their name while in the IRC channel)
	 * @see irc_bot_interface::parse_sub
	 */
	void (*usernick) (int fd, char *cmd, char *source, char *target, char *msg);
};

#ifdef HERCULES_CORE
void ircbot_defaults(void);
#endif // HERCULES_CORE

HPShared struct irc_bot_interface *ircbot;

#endif /* MAP_IRC_BOT_H */
