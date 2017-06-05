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
#ifndef MAP_CHANNEL_H
#define MAP_CHANNEL_H

#include "common/hercules.h"
#include "common/mmo.h"

#include "map/map.h" // EVENT_NAME_LENGTH, MAX_EVENTQUEUE

/**
 * Declarations
 **/
struct DBMap; // common/db.h
struct map_session_data;
struct guild;

/**
 * Defines
 **/
#define HCS_NAME_LENGTH 20

enum channel_options {
	HCS_OPT_BASE          = 0x0,
	HCS_OPT_ANNOUNCE_JOIN = 0x1,
	HCS_OPT_MSG_DELAY     = 0x2,
};

enum channel_types {
	HCS_TYPE_PUBLIC  = 0,
	HCS_TYPE_PRIVATE = 1,
	HCS_TYPE_MAP     = 2,
	HCS_TYPE_ALLY    = 3,
	HCS_TYPE_IRC     = 4,
};

enum channel_operation_status {
	HCS_STATUS_OK = 0,
	HCS_STATUS_FAIL,
	HCS_STATUS_ALREADY,
	HCS_STATUS_NOPERM,
	HCS_STATUS_BANNED,
};

/**
 * Structures
 **/
struct Channel_Config {
	unsigned int *colors;
	char **colors_name;
	unsigned char colors_count;
	bool local, ally, irc;
	bool local_autojoin, ally_autojoin, irc_autojoin;
	char local_name[HCS_NAME_LENGTH], ally_name[HCS_NAME_LENGTH], irc_name[HCS_NAME_LENGTH];
	unsigned char local_color, ally_color, irc_color;
	bool closing;
	bool allow_user_channel_creation;
	char irc_server[40], irc_channel[50], irc_nick[40], irc_nick_pw[30];
	unsigned short irc_server_port;
	bool irc_use_ghost;
};

struct channel_ban_entry {
	char name[NAME_LENGTH];
};

struct channel_data {
	char name[HCS_NAME_LENGTH];
	char password[HCS_NAME_LENGTH];
	unsigned char color;
	struct DBMap *users;
	struct DBMap *banned;
	char handlers[MAX_EVENTQUEUE][EVENT_NAME_LENGTH];
	unsigned int options;
	unsigned int owner;
	enum channel_types type;
	uint16 m;
	unsigned char msg_delay;
};

struct channel_interface {
	/* vars */
	struct DBMap *db;
	struct Channel_Config *config;

	int (*init) (bool minimal);
	void (*final) (void);

	struct channel_data *(*search) (const char *name, struct map_session_data *sd);
	struct channel_data *(*create) (enum channel_types type, const char *name, unsigned char color);
	void (*delete) (struct channel_data *chan);

	void (*set_password) (struct channel_data *chan, const char *password);
	enum channel_operation_status (*ban) (struct channel_data *chan, const struct map_session_data *ssd, struct map_session_data *tsd);
	enum channel_operation_status (*unban) (struct channel_data *chan, const struct map_session_data *ssd, struct map_session_data *tsd);
	void (*set_options) (struct channel_data *chan, unsigned int options);

	void (*send) (struct channel_data *chan, struct map_session_data *sd, const char *msg);
	void (*join_sub) (struct channel_data *chan, struct map_session_data *sd, bool stealth);
	enum channel_operation_status (*join) (struct channel_data *chan, struct map_session_data *sd, const char *password, bool silent);
	void (*leave) (struct channel_data *chan, struct map_session_data *sd);
	void (*leave_sub) (struct channel_data *chan, struct map_session_data *sd);
	void (*quit) (struct map_session_data *sd);

	void (*map_join) (struct map_session_data *sd);
	void (*guild_join_alliance) (const struct guild *g_source, const struct guild *g_ally);
	void (*guild_leave_alliance) (const struct guild *g_source, const struct guild *g_ally);
	void (*quit_guild) (struct map_session_data *sd);
	void (*irc_join) (struct map_session_data *sd);

	void (*config_read) (void);
};

#ifdef HERCULES_CORE
void channel_defaults(void);
#endif // HERCULES_CORE

HPShared struct channel_interface *channel;

#endif /* MAP_CHANNEL_H */
