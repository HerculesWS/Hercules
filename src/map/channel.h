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
	char password[HCS_NAME_LENGTH];
	unsigned char color;
	DBMap *users;
	DBMap *banned;
	unsigned int options;
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

	void (*config_read) (void);
};

struct channel_interface *channel;

#ifdef HERCULES_CORE
void channel_defaults(void);
#endif // HERCULES_CORE

#endif /* MAP_CHANNEL_H */
