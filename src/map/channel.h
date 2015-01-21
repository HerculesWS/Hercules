// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#ifndef MAP_CHANNEL_H
#define MAP_CHANNEL_H

#include <stdarg.h>

#include "map.h"
#include "../common/cbasetypes.h"
#include "../common/db.h"

/**
 * Declarations
 **/
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

/**
 * Structures
 **/
struct Channel_Config {
	unsigned int *colors;
	char **colors_name;
	unsigned char colors_count;
	bool local, ally, irc;
	bool local_autojoin, ally_autojoin;
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
	char pass[HCS_NAME_LENGTH];
	unsigned char color;
	DBMap *users;
	DBMap *banned;
	unsigned int opt;
	unsigned int owner;
	enum channel_types type;
	uint16 m;
	unsigned char msg_delay;
};

struct channel_interface {
	/* vars */
	DBMap *db;
	struct Channel_Config *config;

	int (*init) (bool minimal);
	void (*final) (void);

	void (*create) (struct channel_data *chan, char *name, char *pass, unsigned char color);
	void (*send) (struct channel_data *chan, struct map_session_data *sd, const char *msg);
	void (*join) (struct channel_data *chan, struct map_session_data *sd);
	void (*leave) (struct channel_data *chan, struct map_session_data *sd);
	void (*delete) (struct channel_data *chan);
	void (*map_join) (struct map_session_data *sd);
	void (*quit) (struct map_session_data *sd);
	void (*quit_guild) (struct map_session_data *sd);
	void (*guild_join) (struct guild *g1,struct guild *g2);
	void (*guild_leave) (struct guild *g1,struct guild *g2);
	void (*config_read) (void);
};

struct channel_interface *channel;

#ifdef HERCULES_CORE
void channel_defaults(void);
#endif // HERCULES_CORE

#endif /* MAP_CHANNEL_H */
