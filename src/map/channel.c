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
#define HERCULES_CORE

#include "channel.h"

#include "map/atcommand.h"
#include "map/guild.h"
#include "map/instance.h"
#include "map/irc-bot.h"
#include "map/map.h"
#include "map/pc.h"
#include "common/cbasetypes.h"
#include "common/conf.h"
#include "common/db.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/random.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "common/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct channel_interface channel_s;
struct channel_interface *channel;

static struct Channel_Config channel_config;

/**
 * Returns the named channel.
 *
 * @param name The channel name
 * @param sd   The issuer character, for character-specific channels (i.e. map, ally)
 * @return a pointer to the channel, or NULL.
 */
struct channel_data *channel_search(const char *name, struct map_session_data *sd)
{
	const char *realname = name;
	if (!realname || !*realname)
		return NULL;
	if (*realname == '#') {
		realname++;
		if (!*realname)
			return NULL;
	}

	if (channel->config->local && strcmpi(realname, channel->config->local_name) == 0) {
		if (!sd)
			return NULL;
		if (!map->list[sd->bl.m].channel) {
			channel->map_join(sd);
		}
		return map->list[sd->bl.m].channel;
	}

	if (channel->config->ally && strcmpi(realname, channel->config->ally_name) == 0) {
		if (!sd || !sd->status.guild_id || !sd->guild)
			return NULL;
		return sd->guild->channel;
	}

	return strdb_get(channel->db, realname);
}

/**
 * Creates a chat channel.
 *
 * If the channel type isn't HCS_TYPE_MAP or HCS_TYPE_ALLY, the channel is added to the channel->db.
 *
 * @param type The channel type.
 * @param name The channel name.
 * @param color The channel chat color.
 * @return A pointer to the created channel.
 */
struct channel_data *channel_create(enum channel_types type, const char *name, unsigned char color)
{
	struct channel_data *chan;

	if (!name)
		return NULL;

	CREATE(chan, struct channel_data, 1);
	chan->users = idb_alloc(DB_OPT_BASE);
	safestrncpy(chan->name, name, HCS_NAME_LENGTH);
	chan->color = color;

	chan->options = HCS_OPT_BASE;
	chan->banned = NULL;

	chan->msg_delay = 0;

	chan->type = type;
	if (chan->type != HCS_TYPE_MAP && chan->type != HCS_TYPE_ALLY)
		strdb_put(channel->db, chan->name, chan);
	return chan;
}

/**
 * Deletes a chat channel.
 *
 * @param chan The channel to delete
 */
void channel_delete(struct channel_data *chan)
{
	nullpo_retv(chan);

	if (db_size(chan->users) && !channel->config->closing) {
		struct map_session_data *sd;
		struct DBIterator *iter = db_iterator(chan->users);
		for (sd = dbi_first(iter); dbi_exists(iter); sd = dbi_next(iter)) {
			channel->leave_sub(chan, sd);
		}
		dbi_destroy(iter);
	}
	if (chan->banned) {
		db_destroy(chan->banned);
		chan->banned = NULL;
	}
	db_destroy(chan->users);
	if (chan->m) {
		map->list[chan->m].channel = NULL;
		aFree(chan);
	} else if (chan->type == HCS_TYPE_ALLY) {
		aFree(chan);
	} else if (!channel->config->closing) {
		strdb_remove(channel->db, chan->name);
	}
}

/**
 * Sets a chat channel password.
 *
 * @param chan The channel to edit.
 * @param pass The password to set. Pass NULL to remove existing passwords.
 */
void channel_set_password(struct channel_data *chan, const char *password)
{
	nullpo_retv(chan);
	if (password)
		safestrncpy(chan->password, password, HCS_NAME_LENGTH);
	else
		chan->password[0] = '\0';
}

/**
 * Bans a character from the given channel.
 *
 * @param chan The channel.
 * @param ssd  The source character, if any.
 * @param tsd  The target character.
 * @retval HCS_STATUS_OK if the operation succeeded.
 * @retval HCS_STATUS_ALREADY if the target character is already banned.
 * @retval HCS_STATUS_NOPERM if the source character doesn't have enough permissions.
 * @retval HCS_STATUS_FAIL in case of generic failure.
 */
enum channel_operation_status channel_ban(struct channel_data *chan, const struct map_session_data *ssd, struct map_session_data *tsd)
{
	struct channel_ban_entry *entry = NULL;

	nullpo_retr(HCS_STATUS_FAIL, chan);
	nullpo_retr(HCS_STATUS_FAIL, tsd);

	if (ssd && chan->owner != ssd->status.char_id && !pc_has_permission(ssd, PC_PERM_HCHSYS_ADMIN))
		return HCS_STATUS_NOPERM;

	if (pc_has_permission(tsd, PC_PERM_HCHSYS_ADMIN))
		return HCS_STATUS_FAIL;

	if (chan->banned && idb_exists(chan->banned, tsd->status.account_id))
		return HCS_STATUS_ALREADY;

	if (!chan->banned)
		chan->banned = idb_alloc(DB_OPT_BASE|DB_OPT_ALLOW_NULL_DATA|DB_OPT_RELEASE_DATA);

	CREATE(entry, struct channel_ban_entry, 1);
	safestrncpy(entry->name, tsd->status.name, NAME_LENGTH);
	idb_put(chan->banned, tsd->status.account_id, entry);

	channel->leave(chan, tsd);

	return HCS_STATUS_OK;
}

/**
 * Unbans a character from the given channel.
 *
 * @param chan The channel.
 * @param ssd  The source character, if any.
 * @param tsd  The target character. If no target character is specified, all characters are unbanned.
 * @retval HCS_STATUS_OK if the operation succeeded.
 * @retval HCS_STATUS_ALREADY if the target character is not banned.
 * @retval HCS_STATUS_NOPERM if the source character doesn't have enough permissions.
 * @retval HCS_STATUS_FAIL in case of generic failure.
 */
enum channel_operation_status channel_unban(struct channel_data *chan, const struct map_session_data *ssd, struct map_session_data *tsd)
{
	nullpo_retr(HCS_STATUS_FAIL, chan);

	if (ssd && chan->owner != ssd->status.char_id && !pc_has_permission(ssd, PC_PERM_HCHSYS_ADMIN))
		return HCS_STATUS_NOPERM;

	if (!chan->banned)
		return HCS_STATUS_ALREADY;

	if (!tsd) {
		// Unban all
		db_destroy(chan->banned);
		chan->banned = NULL;
		return HCS_STATUS_OK;
	}

	// Unban one
	if (!idb_exists(chan->banned, tsd->status.account_id))
		return HCS_STATUS_ALREADY;

	idb_remove(chan->banned, tsd->status.account_id);
	if (!db_size(chan->banned)) {
		db_destroy(chan->banned);
		chan->banned = NULL;
	}

	return HCS_STATUS_OK;
}

/**
 * Sets or edits a channel's options.
 *
 * @param chan The channel.
 * @param options The new options set to apply.
 */
void channel_set_options(struct channel_data *chan, unsigned int options)
{
	nullpo_retv(chan);

	chan->options = options;
}

/**
 * Sends a message to a channel.
 *
 * @param chan The destination channel.
 * @param sd   The source character.
 * @param msg  The message to send.
 *
 * If no source character is specified, it'll send an anonymous message.
 */
void channel_send(struct channel_data *chan, struct map_session_data *sd, const char *msg)
{
	char message[150];
	nullpo_retv(chan);
	nullpo_retv(msg);

	if (sd && chan->msg_delay != 0
	 && DIFF_TICK(sd->hchsysch_tick + chan->msg_delay*1000, timer->gettick()) > 0
	 && !pc_has_permission(sd, PC_PERM_HCHSYS_ADMIN)) {
		clif->messagecolor_self(sd->fd, COLOR_RED, msg_sd(sd,1455));
		return;
	} else if (sd) {
		safesnprintf(message, 150, "[ #%s ] %s : %s", chan->name, sd->status.name, msg);
		clif->channel_msg(chan,sd,message);
		if (chan->type == HCS_TYPE_IRC)
			ircbot->relay(sd->status.name,msg);
		if (chan->msg_delay != 0)
			sd->hchsysch_tick = timer->gettick();
	} else {
		safesnprintf(message, 150, "[ #%s ] %s", chan->name, msg);
		clif->channel_msg2(chan, message);
		if (chan->type == HCS_TYPE_IRC)
			ircbot->relay(NULL, msg);
	}
}

/**
 * Joins a channel, without any permission checks.
 *
 * @param chan    The channel to join
 * @param sd      The character
 * @param stealth If true, hide join announcements.
 */
void channel_join_sub(struct channel_data *chan, struct map_session_data *sd, bool stealth)
{
	nullpo_retv(chan);
	nullpo_retv(sd);

	if (idb_put(chan->users, sd->status.char_id, sd))
		return;

	RECREATE(sd->channels, struct channel_data *, ++sd->channel_count);
	sd->channels[sd->channel_count - 1] = chan;

	if (!stealth && (chan->options&HCS_OPT_ANNOUNCE_JOIN)) {
		char message[60];
		sprintf(message, "#%s '%s' joined",chan->name,sd->status.name);
		clif->channel_msg(chan,sd,message);
	}

	/* someone is cheating, we kindly disconnect the bastard */
	if (sd->channel_count > 200) {
		sockt->eof(sd->fd);
	}

}

/**
 * Joins a channel.
 *
 * Ban and password checks are performed before joining the channel.
 * If the channel is an HCS_TYPE_ALLY channel, alliance channels are automatically joined.
 *
 * @param chan     The channel to join
 * @param sd       The character
 * @param password The specified join password, if any
 * @param silent   If true, suppress the "You're now in the <x> channel" greeting message
 * @retval HCS_STATUS_OK      if the operation succeeded
 * @retval HCS_STATUS_ALREADY if the character is already in the channel
 * @retval HCS_STATUS_NOPERM  if the specified password doesn't match
 * @retval HCS_STATUS_BANNED  if the character is in the channel's ban list
 * @retval HCS_STATUS_FAIL    in case of generic error
 */
enum channel_operation_status channel_join(struct channel_data *chan, struct map_session_data *sd, const char *password, bool silent)
{
	bool stealth = false;

	nullpo_retr(HCS_STATUS_FAIL, chan);
	nullpo_retr(HCS_STATUS_FAIL, sd);
	nullpo_retr(HCS_STATUS_FAIL, password);

	if (idb_exists(chan->users, sd->status.char_id)) {
		return HCS_STATUS_ALREADY;
	}

	if (chan->password[0] != '\0' && strcmp(chan->password, password) != 0) {
		if (pc_has_permission(sd, PC_PERM_HCHSYS_ADMIN)) {
			stealth = true;
		} else {
			return HCS_STATUS_NOPERM;
		}
	}

	if (chan->banned && idb_exists(chan->banned, sd->status.account_id)) {
		return HCS_STATUS_BANNED;
	}

	if (!silent && !(chan->options&HCS_OPT_ANNOUNCE_JOIN)) {
		char output[CHAT_SIZE_MAX];
		if (chan->type == HCS_TYPE_MAP) {
			sprintf(output, msg_sd(sd,1435), chan->name, map->list[chan->m].name); // You're now in the '#%s' channel for '%s'
		} else {
			sprintf(output, msg_sd(sd,1403), chan->name); // You're now in the '%s' channel
		}
		clif->messagecolor_self(sd->fd, COLOR_DEFAULT, output);
	}

	if (chan->type == HCS_TYPE_ALLY) {
		struct guild *g = sd->guild;
		int i;
		for (i = 0; i < MAX_GUILDALLIANCE; i++) {
			struct guild *sg = NULL;
			if (g->alliance[i].opposition == 0 && g->alliance[i].guild_id && (sg = guild->search(g->alliance[i].guild_id)) != NULL) {
				if (!(sg->channel->banned && idb_exists(sg->channel->banned, sd->status.account_id))) {
					channel->join_sub(sg->channel, sd, stealth);
				}
			}
		}
	}

	channel->join_sub(chan, sd, stealth);

	return HCS_STATUS_OK;
}

/**
 * Removes a channel from a character's join lists.
 *
 * @param chan The channel to leave
 * @param sd   The character
 */
void channel_leave_sub(struct channel_data *chan, struct map_session_data *sd)
{
	unsigned char i;

	nullpo_retv(chan);
	nullpo_retv(sd);
	for (i = 0; i < sd->channel_count; i++) {
		if (sd->channels[i] == chan) {
			sd->channels[i] = NULL;
			break;
		}
	}
	if (i < sd->channel_count) {
		unsigned char cursor = 0;
		for (i = 0; i < sd->channel_count; i++) {
			if (sd->channels[i] == NULL)
				continue;
			if (cursor != i) {
				sd->channels[cursor] = sd->channels[i];
			}
			cursor++;
		}
		if (!(sd->channel_count = cursor)) {
			aFree(sd->channels);
			sd->channels = NULL;
		}
	}
}
/**
 * Leaves a channel.
 *
 * @param chan The channel to leave
 * @param sd   The character
 */
void channel_leave(struct channel_data *chan, struct map_session_data *sd)
{
	nullpo_retv(chan);
	nullpo_retv(sd);

	if (!idb_remove(chan->users,sd->status.char_id))
		return;

	if (chan == sd->gcbind)
		sd->gcbind = NULL;

	if (!db_size(chan->users) && chan->type == HCS_TYPE_PRIVATE) {
		channel->delete(chan);
	} else if (!channel->config->closing && (chan->options & HCS_OPT_ANNOUNCE_JOIN)) {
		char message[60];
		sprintf(message, "#%s '%s' left",chan->name,sd->status.name);
		clif->channel_msg(chan,sd,message);
	}

	channel->leave_sub(chan, sd);
}

/**
 * Quits the channel system.
 *
 * Leaves all joined channels.
 *
 * @param sd The target character
 */
void channel_quit(struct map_session_data *sd)
{
	nullpo_retv(sd);
	while (sd->channel_count > 0) {
		// Loop downward to avoid unnecessary array compactions by channel_leave
		struct channel_data *chan = sd->channels[sd->channel_count-1];

		if (chan == NULL) {
			sd->channel_count--;
			continue;
		}

		channel->leave(chan, sd);
	}
}

/**
 * Joins the local map channel.
 *
 * @param sd The target character (sd must be non null)
 */
void channel_map_join(struct map_session_data *sd)
{
	nullpo_retv(sd);
	if (sd->state.autotrade || sd->state.standalone)
		return;
	if (!map->list[sd->bl.m].channel) {
		if (map->list[sd->bl.m].flag.chsysnolocalaj || (map->list[sd->bl.m].instance_id >= 0 && instance->list[map->list[sd->bl.m].instance_id].owner_type != IOT_NONE))
			return;

		map->list[sd->bl.m].channel = channel->create(HCS_TYPE_MAP, channel->config->local_name, channel->config->local_color);
		map->list[sd->bl.m].channel->m = sd->bl.m;
	}

	channel->join(map->list[sd->bl.m].channel, sd, "", false);
}

void channel_irc_join(struct map_session_data *sd)
{
	struct channel_data *chan = ircbot->channel;

	nullpo_retv(sd);
	if (sd->state.autotrade || sd->state.standalone)
		return;
	if (channel->config->irc_name[0] == '\0')
		return;
	if (chan)
		channel->join(chan, sd, "", false);
}

/**
 * Lets a guild's members join a newly allied guild's channel.
 *
 * Note: g_source members will join g_ally's channel.
 * To have g_ally members join g_source's channel, call this function twice, with inverted arguments.
 *
 * @param g_source Source guild
 * @param g_ally   Allied guild
 */
void channel_guild_join_alliance(const struct guild *g_source, const struct guild *g_ally)
{
	struct channel_data *chan;

	nullpo_retv(g_source);
	nullpo_retv(g_ally);

	if ((chan = g_ally->channel)) {
		int i;
		for (i = 0; i < g_source->max_member; i++) {
			struct map_session_data *sd = g_source->member[i].sd;
			if (sd == NULL)
				continue;
			if (!(g_ally->channel->banned && idb_exists(g_ally->channel->banned, sd->status.account_id)))
				channel->join_sub(chan,sd, false);
		}
	}
}

/**
 * Makes a guild's members leave a no former allied guild's channel.
 *
 * Note: g_source members will leave g_ally's channel.
 * To have g_ally members leave g_source's channel, call this function twice, with inverted arguments.
 *
 * @param g_source Source guild
 * @param g_ally   Former allied guild
 */
void channel_guild_leave_alliance(const struct guild *g_source, const struct guild *g_ally)
{
	struct channel_data *chan;

	nullpo_retv(g_source);
	nullpo_retv(g_ally);

	if ((chan = g_ally->channel)) {
		int i;
		for (i = 0; i < g_source->max_member; i++) {
			struct map_session_data *sd = g_source->member[i].sd;
			if (sd == NULL)
				continue;

			channel->leave(chan,sd);
		}
	}
}

/**
 * Makes a character quit all guild-related channels.
 *
 * @param sd The character (must be non null)
 */
void channel_quit_guild(struct map_session_data *sd)
{
	unsigned char i;

	nullpo_retv(sd);
	for (i = 0; i < sd->channel_count; i++) {
		struct channel_data *chan = sd->channels[i];

		if (chan == NULL || chan->type != HCS_TYPE_ALLY)
			continue;

		channel->leave(chan, sd);
	}
}

void read_channels_config(void)
{
	struct config_t channels_conf;
	struct config_setting_t *chsys = NULL;
	const char *config_filename = "conf/channels.conf"; // FIXME hardcoded name

	if (!libconfig->load_file(&channels_conf, config_filename))
		return;

	chsys = libconfig->lookup(&channels_conf, "chsys");

	if (chsys != NULL) {
		struct config_setting_t *settings = libconfig->setting_get_elem(chsys, 0);
		struct config_setting_t *channels;
		struct config_setting_t *colors;
		int i,k;
		const char *local_name, *ally_name,
					*local_color, *ally_color,
					*irc_name, *irc_color;
		int ally_enabled = 0, local_enabled = 0,
			local_autojoin = 0, ally_autojoin = 0,
			allow_user_channel_creation = 0,
			irc_enabled = 0,
			irc_autojoin = 0,
			irc_flood_protection_rate = 0,
			irc_flood_protection_burst = 0,
			irc_flood_protection_enabled = 0;

		if( !libconfig->setting_lookup_string(settings, "map_local_channel_name", &local_name) )
			local_name = "map";
		safestrncpy(channel->config->local_name, local_name, HCS_NAME_LENGTH);

		if( !libconfig->setting_lookup_string(settings, "ally_channel_name", &ally_name) )
			ally_name = "ally";
		safestrncpy(channel->config->ally_name, ally_name, HCS_NAME_LENGTH);

		if( !libconfig->setting_lookup_string(settings, "irc_channel_name", &irc_name) )
			irc_name = "irc";
		safestrncpy(channel->config->irc_name, irc_name, HCS_NAME_LENGTH);

		libconfig->setting_lookup_bool(settings, "map_local_channel", &local_enabled);
		libconfig->setting_lookup_bool(settings, "ally_channel_enabled", &ally_enabled);
		libconfig->setting_lookup_bool(settings, "irc_channel_enabled", &irc_enabled);

		if (local_enabled)
			channel->config->local = true;
		if (ally_enabled)
			channel->config->ally = true;
		if (irc_enabled)
			channel->config->irc = true;

		channel->config->irc_server[0] = channel->config->irc_channel[0] = channel->config->irc_nick[0] = channel->config->irc_nick_pw[0] = '\0';

		if (channel->config->irc) {
			const char *irc_server, *irc_channel,
				 *irc_nick, *irc_nick_pw;
			int irc_use_ghost = 0;
			if (!libconfig->setting_lookup_string(settings, "irc_channel_network", &irc_server)) {
				channel->config->irc = false;
				ShowWarning("channels.conf : irc channel enabled but irc_channel_network wasn't found, disabling irc channel...\n");
			} else {
				char *server = aStrdup(irc_server);
				char *port = strchr(server, ':');
				if (port == NULL) {
					channel->config->irc = false;
					ShowWarning("channels.conf: network port wasn't found in 'irc_channel_network', disabling irc channel...\n");
				} else if ((size_t)(port-server) > sizeof channel->config->irc_server - 1) {
					channel->config->irc = false;
					ShowWarning("channels.conf: server name is too long in 'irc_channel_network', disabling irc channel...\n");
				} else {
					*port = '\0';
					port++;
					safestrncpy(channel->config->irc_server, server, sizeof channel->config->irc_server);
					channel->config->irc_server_port = atoi(port);
				}
				aFree(server);
			}
			if (libconfig->setting_lookup_string(settings, "irc_channel_channel", &irc_channel)) {
				safestrncpy(channel->config->irc_channel, irc_channel, 50);
			} else {
				channel->config->irc = false;
				ShowWarning("channels.conf : irc channel enabled but irc_channel_channel wasn't found, disabling irc channel...\n");
			}
			if (libconfig->setting_lookup_string(settings, "irc_channel_nick", &irc_nick)) {
				if (strcmpi(irc_nick,"Hercules_chSysBot") == 0) {
					sprintf(channel->config->irc_nick, "Hercules_chSysBot%d",rnd()%777);
				} else {
					safestrncpy(channel->config->irc_nick, irc_nick, 40);
				}
			} else {
				channel->config->irc = false;
				ShowWarning("channels.conf : irc channel enabled but irc_channel_nick wasn't found, disabling irc channel...\n");
			}
			if (libconfig->setting_lookup_string(settings, "irc_channel_nick_pw", &irc_nick_pw)) {
				safestrncpy(channel->config->irc_nick_pw, irc_nick_pw, 30);
				config_setting_lookup_bool(settings, "irc_channel_use_ghost", &irc_use_ghost);
				channel->config->irc_use_ghost = irc_use_ghost;
			}
		}

		libconfig->setting_lookup_bool(settings, "map_local_channel_autojoin", &local_autojoin);
		libconfig->setting_lookup_bool(settings, "ally_channel_autojoin", &ally_autojoin);
		libconfig->setting_lookup_bool(settings, "irc_channel_autojoin", &irc_autojoin);
		if (local_autojoin)
			channel->config->local_autojoin = true;
		if (ally_autojoin)
			channel->config->ally_autojoin = true;
		if (irc_autojoin)
			channel->config->irc_autojoin = true;

		libconfig->setting_lookup_bool(settings, "irc_flood_protection_enabled", &irc_flood_protection_enabled);

		if (irc_flood_protection_enabled) {
			ircbot->flood_protection_enabled = true;

			libconfig->setting_lookup_int(settings, "irc_flood_protection_rate", &irc_flood_protection_rate);
			libconfig->setting_lookup_int(settings, "irc_flood_protection_burst", &irc_flood_protection_burst);

			if (irc_flood_protection_rate > 0)
				ircbot->flood_protection_rate = irc_flood_protection_rate;
			else
				ircbot->flood_protection_rate = 1000;
			if (irc_flood_protection_burst > 0)
				ircbot->flood_protection_burst = irc_flood_protection_burst;
			else
				ircbot->flood_protection_burst = 3;
		} else {
			ircbot->flood_protection_enabled = false;
		}

		libconfig->setting_lookup_bool(settings, "allow_user_channel_creation", &allow_user_channel_creation);

		if( allow_user_channel_creation )
			channel->config->allow_user_channel_creation = true;

		if( (colors = libconfig->setting_get_member(settings, "colors")) != NULL ) {
			int color_count = libconfig->setting_length(colors);
			CREATE(channel->config->colors, unsigned int, color_count);
			CREATE(channel->config->colors_name, char *, color_count);
			for(i = 0; i < color_count; i++) {
				struct config_setting_t *color = libconfig->setting_get_elem(colors, i);

				CREATE(channel->config->colors_name[i], char, HCS_NAME_LENGTH);

				safestrncpy(channel->config->colors_name[i], config_setting_name(color), HCS_NAME_LENGTH);

				channel->config->colors[i] = (unsigned int)strtoul(libconfig->setting_get_string_elem(colors,i),NULL,0);
			}
			channel->config->colors_count = color_count;
		}

		libconfig->setting_lookup_string(settings, "map_local_channel_color", &local_color);

		for (k = 0; k < channel->config->colors_count; k++) {
			if (strcmpi(channel->config->colors_name[k], local_color) == 0)
				break;
		}

		if (k < channel->config->colors_count) {
			channel->config->local_color = k;
		} else {
			ShowError("channels.conf: unknown color '%s' for 'map_local_channel_color', disabling '#%s'...\n",local_color,local_name);
			channel->config->local = false;
		}

		libconfig->setting_lookup_string(settings, "ally_channel_color", &ally_color);

		for (k = 0; k < channel->config->colors_count; k++) {
			if (strcmpi(channel->config->colors_name[k], ally_color) == 0)
				break;
		}

		if( k < channel->config->colors_count ) {
			channel->config->ally_color = k;
		} else {
			ShowError("channels.conf: unknown color '%s' for 'ally_channel_color', disabling '#%s'...\n",ally_color,ally_name);
			channel->config->ally = false;
		}

		libconfig->setting_lookup_string(settings, "irc_channel_color", &irc_color);

		for (k = 0; k < channel->config->colors_count; k++) {
			if (strcmpi(channel->config->colors_name[k], irc_color) == 0)
				break;
		}

		if (k < channel->config->colors_count) {
			channel->config->irc_color = k;
		} else {
			ShowError("channels.conf: unknown color '%s' for 'irc_channel_color', disabling '#%s'...\n",irc_color,irc_name);
			channel->config->irc = false;
		}

		if (channel->config->irc) {
			ircbot->channel = channel->create(HCS_TYPE_IRC, channel->config->irc_name, channel->config->irc_color);
		}

		if( (channels = libconfig->setting_get_member(settings, "default_channels")) != NULL ) {
			int channel_count = libconfig->setting_length(channels);

			for(i = 0; i < channel_count; i++) {
				struct config_setting_t *chan = libconfig->setting_get_elem(channels, i);
				const char *name = config_setting_name(chan);
				const char *color = libconfig->setting_get_string_elem(channels,i);

				ARR_FIND(0, channel->config->colors_count, k, strcmpi(channel->config->colors_name[k],color) == 0);
				if (k == channel->config->colors_count) {
					ShowError("channels.conf: unknown color '%s' for channel '%s', skipping channel...\n",color,name);
					continue;
				}
				if (strcmpi(name, channel->config->local_name) == 0
				 || strcmpi(name, channel->config->ally_name) == 0
				 || strcmpi(name, channel->config->irc_name) == 0
				 || strdb_exists(channel->db, name)) {
					ShowError("channels.conf: duplicate channel '%s', skipping channel...\n",name);
					continue;

				}
				channel->create(HCS_TYPE_PUBLIC, name, k);
			}
		}

		ShowStatus("Done reading '"CL_WHITE"%u"CL_RESET"' channels in '"CL_WHITE"%s"CL_RESET"'.\n", db_size(channel->db), config_filename);
	}
	libconfig->destroy(&channels_conf);
}

/*==========================================
 *
 *------------------------------------------*/
int do_init_channel(bool minimal)
{
	if (minimal)
		return 0;

	channel->db = stridb_alloc(DB_OPT_DUP_KEY|DB_OPT_RELEASE_DATA, HCS_NAME_LENGTH);
	channel->config->ally = channel->config->local = channel->config->irc = channel->config->ally_autojoin = channel->config->local_autojoin = channel->config->irc_autojoin = false;
	channel->config_read();

	return 0;
}

void do_final_channel(void)
{
	struct DBIterator *iter = db_iterator(channel->db);
	struct channel_data *chan;
	unsigned char i;

	for( chan = dbi_first(iter); dbi_exists(iter); chan = dbi_next(iter) ) {
		channel->delete(chan);
	}

	dbi_destroy(iter);

	for(i = 0; i < channel->config->colors_count; i++) {
		aFree(channel->config->colors_name[i]);
	}

	if (channel->config->colors_count) {
		aFree(channel->config->colors_name);
		aFree(channel->config->colors);
	}

	db_destroy(channel->db);
}

void channel_defaults(void)
{
	channel = &channel_s;

	channel->db = NULL;
	channel->config = &channel_config;

	channel->init = do_init_channel;
	channel->final = do_final_channel;

	channel->search = channel_search;
	channel->create = channel_create;
	channel->delete = channel_delete;

	channel->set_password = channel_set_password;
	channel->ban = channel_ban;
	channel->unban = channel_unban;
	channel->set_options = channel_set_options;

	channel->send = channel_send;
	channel->join_sub = channel_join_sub;
	channel->join = channel_join;
	channel->leave = channel_leave;
	channel->leave_sub = channel_leave_sub;
	channel->quit = channel_quit;

	channel->map_join = channel_map_join;
	channel->guild_join_alliance = channel_guild_join_alliance;
	channel->guild_leave_alliance = channel_guild_leave_alliance;
	channel->quit_guild = channel_quit_guild;
	channel->irc_join = channel_irc_join;

	channel->config_read = read_channels_config;
}
