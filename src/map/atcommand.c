/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2024 Hercules Dev Team
 * Copyright (C) Athena Dev Teams
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

#include "config/core.h" // AUTOLOOTITEM_SIZE, AUTOTRADE_PERSISTENCY, MAX_SUGGESTIONS, MOB_FLEE(), MOB_HIT(), RENEWAL, RENEWAL_DROP, RENEWAL_EXP
#include "atcommand.h"

#include "map/HPMmap.h"
#include "map/battle.h"
#include "map/channel.h"
#include "map/chat.h"
#include "map/chrif.h"
#include "map/clan.h"
#include "map/clif.h"
#include "map/duel.h"
#include "map/elemental.h"
#include "map/goldpc.h"
#include "map/grader.h"
#include "map/guild.h"
#include "map/homunculus.h"
#include "map/intif.h"
#include "map/itemdb.h"
#include "map/log.h"
#include "map/mail.h"
#include "map/map.h"
#include "map/mapreg.h"
#include "map/mercenary.h"
#include "map/mob.h"
#include "map/npc.h"
#include "map/party.h"
#include "map/pc.h"
#include "map/pc_groups.h" // groupid2name
#include "map/pet.h"
#include "map/quest.h"
#include "map/refine.h"
#include "map/script.h"
#include "map/searchstore.h"
#include "map/skill.h"
#include "map/status.h"
#include "map/storage.h"
#include "map/trade.h"
#include "map/unit.h"
#include "map/achievement.h"
#include "common/cbasetypes.h"
#include "common/conf.h"
#include "common/core.h"
#include "common/memmgr.h"
#include "common/mmo.h" // MAX_CARTS
#include "common/msgtable.h"
#include "common/nullpo.h"
#include "common/packets.h"
#include "common/random.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/sysinfo.h"
#include "common/timer.h"
#include "common/utils.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct atcommand_interface atcommand_s;
struct atcommand_interface *atcommand;

static char atcmd_output[CHAT_SIZE_MAX];
static char atcmd_player_name[NAME_LENGTH];

// @commands (script-based)
static struct atcmd_binding_data *get_atcommandbind_byname(const char *name)
{
	int i = 0;

	nullpo_retr(NULL, name);
	if( *name == atcommand->at_symbol || *name == atcommand->char_symbol )
		name++; // for backwards compatibility

	ARR_FIND( 0, atcommand->binding_count, i, strcmpi(atcommand->binding[i]->command, name) == 0 );

	return ( i < atcommand->binding_count ) ? atcommand->binding[i] : NULL;
}

static const char *atcommand_msgsd(struct map_session_data *sd, int msg_number)
{
	Assert_retr("??", msg_number >= 0 && msg_number < MSGTBL_MAX && atcommand->msg_table[0][msg_number] != NULL);
	if (!sd || sd->lang_id >= atcommand->max_message_table || !atcommand->msg_table[sd->lang_id][msg_number])
		return atcommand->msg_table[0][msg_number];
	return atcommand->msg_table[sd->lang_id][msg_number];
}

static const char *atcommand_msgfd(int fd, int msg_number)
{
	struct map_session_data *sd = sockt->session_is_valid(fd) ? sockt->session[fd]->session_data : NULL;
	Assert_retr("??", msg_number >= 0 && msg_number < MSGTBL_MAX && atcommand->msg_table[0][msg_number] != NULL);
	if (!sd || sd->lang_id >= atcommand->max_message_table || !atcommand->msg_table[sd->lang_id][msg_number])
		return atcommand->msg_table[0][msg_number];
	return atcommand->msg_table[sd->lang_id][msg_number];
}

//-----------------------------------------------------------
// Return the message string of the specified number by [Yor]
//-----------------------------------------------------------
static const char *atcommand_msg(int msg_number)
{
	Assert_retr("??", msg_number >= 0 && msg_number < MSGTBL_MAX);
	if (atcommand->msg_table[map->default_lang_id][msg_number] != NULL && atcommand->msg_table[map->default_lang_id][msg_number][0] != '\0')
		return atcommand->msg_table[map->default_lang_id][msg_number];

	if(atcommand->msg_table[0][msg_number] != NULL && atcommand->msg_table[0][msg_number][0] != '\0')
		return atcommand->msg_table[0][msg_number];

	ShowWarning("atcommand_msg: Invalid message number was specified: %d", msg_number);
	Assert_report(0);
	return "??";
}

/**
 * Reads Message Data
 *
 * @param[in] cfg_name       configuration filename to read.
 * @param[in] allow_override whether to allow duplicate message IDs to override the original value.
 * @return success state.
 */
static bool msg_config_read(const char *cfg_name, bool allow_override)
{
	int msg_number;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	nullpo_retr(false, cfg_name);
	if ((fp = fopen(cfg_name, "r")) == NULL) {
		ShowError("Messages file not found: %s\n", cfg_name);
		return false;
	}

	if( !atcommand->max_message_table )
		atcommand->expand_message_table();

	while(fgets(line, sizeof(line), fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;
		if (sscanf(line, "%1023[^:]: %1023[^\r\n]", w1, w2) != 2)
			continue;

		if (strcmpi(w1, "import") == 0) {
			atcommand->msg_read(w2, true);
		} else {
			msg_number = atoi(w1);
			if (msg_number >= 0 && msg_number < MSGTBL_MAX) {
				if (atcommand->msg_table[0][msg_number] != NULL) {
					if (!allow_override) {
						ShowError("Duplicate message: ID '%d' was already used for '%s'. Message '%s' will be ignored.\n",
						          msg_number, w2, atcommand->msg_table[0][msg_number]);
						continue;
					}
					aFree(atcommand->msg_table[0][msg_number]);
				}
				/* this could easily become consecutive memory like get_str() and save the malloc overhead for over 1k calls [Ind] */
				atcommand->msg_table[0][msg_number] = (char *)aMalloc((strlen(w2) + 1)*sizeof (char));
				strcpy(atcommand->msg_table[0][msg_number],w2);
			}
		}
	}
	fclose(fp);

	return true;
}

/*==========================================
 * Cleanup Message Data
 *------------------------------------------*/
static void do_final_msg(void)
{
	int i, j;

	for(i = 0; i < atcommand->max_message_table; i++) {
		for (j = 0; j < MSGTBL_MAX; j++) {
			if( atcommand->msg_table[i][j] )
				aFree(atcommand->msg_table[i][j]);
		}
		aFree(atcommand->msg_table[i]);
	}

	if( atcommand->msg_table )
		aFree(atcommand->msg_table);
}

/**
 * retrieves the help string associated with a given command.
 */
static inline const char *atcommand_help_string(AtCommandInfo *info)
{
	return info->help;
}

/*==========================================
 * @send (used for testing packet sends from the client)
 *------------------------------------------*/
ACMD(send)
{
	int len=0,type;
	long num;

	// read message type as hex number (without the 0x)
	if (!*message
	 || !((sscanf(message, "len %x", (unsigned int*)&type)==1 && (len=1, true))
	 || sscanf(message, "%x", (unsigned int*)&type)==1)
	) {
		clif->message(fd, msg_fd(fd, MSGTBL_SEND_USAGE)); // Usage:
		clif->message(fd, msg_fd(fd, MSGTBL_SEND_LEN)); // @send len <packet hex number>
		clif->message(fd, msg_fd(fd, MSGTBL_SEND_PACKET)); // @send <packet hex number> {<value>}*
		clif->message(fd, msg_fd(fd, MSGTBL_SEND_VALUE_TYPE)); // Value: <type=B(default),W,L><number> or S<length>"<string>"
		return false;
	}

#define PARSE_ERROR(error,p) do {\
	clif->message(fd, (error));\
	snprintf(atcmd_output, sizeof(atcmd_output), ">%s", (p));\
	clif->message(fd, atcmd_output);\
} while(0) //define PARSE_ERROR

#define CHECK_EOS(p) do { \
	if(*(p) == 0){ \
		clif->message(fd, "Unexpected end of string");\
		return false;\
	} \
} while(0) //define CHECK_EOS

#define SKIP_VALUE(p) do { \
	while(*(p) && !ISSPACE(*(p))) ++(p); /* non-space */\
	while(*(p) && ISSPACE(*(p)))  ++(p); /* space */\
} while(0) //define SKIP_VALUE

#define GET_VALUE(p,num) do { \
	if(sscanf((p), "x%lx", (long unsigned int*)&(num)) < 1 && sscanf((p), "%ld ", &(num)) < 1){\
		PARSE_ERROR("Invalid number in:",(p));\
		return false;\
	}\
} while(0) //define GET_VALUE

	if (type >= MIN_PACKET_DB && type <= MAX_PACKET_DB) {
		int off = 2;
		if (clif->packet(type) == NULL) {
			// unknown packet - ERROR
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_UNKNOWN_PACKET), type); // Unknown packet: 0x%x
			clif->message(fd, atcmd_output);
			return false;
		}

		if (len) {
			// show packet length
			Assert_retr(false, type <= MAX_PACKET_DB && type >= MIN_PACKET_DB);
			len = packets->db[type];
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_PACKET_LENGTH), type, len); // Packet 0x%x length: %d
			clif->message(fd, atcmd_output);
			return true;
		}

		Assert_retr(false, type <= MAX_PACKET_DB && type >= MIN_PACKET_DB);
		len = packets->db[type];

		if (len == -1) {
			// dynamic packet
			len = SHRT_MAX-4; // maximum length
			off = 4;
		}

		WFIFOHEAD(sd->fd, len);
		WFIFOW(sd->fd,0)=TOW(type);

		// parse packet contents
		SKIP_VALUE(message);
		while(*message != 0 && off < len){
			if(ISDIGIT(*message) || *message == '-' || *message == '+')
			{// default (byte)
				GET_VALUE(message,num);
				WFIFOB(sd->fd,off)=TOB(num);
				++off;
			} else if(TOUPPER(*message) == 'B')
			{// byte
				++message;
				GET_VALUE(message,num);
				WFIFOB(sd->fd,off)=TOB(num);
				++off;
			} else if(TOUPPER(*message) == 'W')
			{// word (2 bytes)
				++message;
				GET_VALUE(message,num);
				WFIFOW(sd->fd,off)=TOW(num);
				off+=2;
			} else if(TOUPPER(*message) == 'L')
			{// long word (4 bytes)
				++message;
				GET_VALUE(message,num);
				WFIFOL(sd->fd,off)=TOL(num);
				off+=4;
			} else if(TOUPPER(*message) == 'S')
			{// string - escapes are valid
				// get string length - num <= 0 means not fixed length (default)
				int end;
				++message;
				if(*message == '"'){
					num=0;
				} else {
					GET_VALUE(message,num);
					while(*message != '"')
					{// find start of string
						if(*message == 0 || ISSPACE(*message)){
							PARSE_ERROR(msg_fd(fd, MSGTBL_NOT_A_STRING),message); // Not a string:
							return false;
						}
						++message;
					}
				}

				// parse string
				++message;
				CHECK_EOS(message);
				end=(num<=0? 0: min(off+((int)num),len));
				for(; *message != '"' && (off < end || end == 0); ++off){
					if(*message == '\\'){
						++message;
						CHECK_EOS(message);
						switch(*message){
							case 'a': num=0x07; break; // Bell
							case 'b': num=0x08; break; // Backspace
							case 't': num=0x09; break; // Horizontal tab
							case 'n': num=0x0A; break; // Line feed
							case 'v': num=0x0B; break; // Vertical tab
							case 'f': num=0x0C; break; // Form feed
							case 'r': num=0x0D; break; // Carriage return
							case 'e': num=0x1B; break; // Escape
							default:  num=*message; break;
							case 'x': // Hexadecimal
							{
								++message;
								CHECK_EOS(message);
								if(!ISXDIGIT(*message)){
									PARSE_ERROR(msg_fd(fd, MSGTBL_NOT_HEX_DIGIT),message); // Not a hexadecimal digit:
									return false;
								}
								num=(ISDIGIT(*message)?*message-'0':TOLOWER(*message)-'a'+10);
								if(ISXDIGIT(*message)){
									++message;
									CHECK_EOS(message);
									num<<=8;
									num+=(ISDIGIT(*message)?*message-'0':TOLOWER(*message)-'a'+10);
								}
								WFIFOB(sd->fd,off)=TOB(num);
								++message;
								CHECK_EOS(message);
								continue;
							}
							case '0':
							case '1':
							case '2':
							case '3':
							case '4':
							case '5':
							case '6':
							case '7': // Octal
							{
								num=*message-'0'; // 1st octal digit
								++message;
								CHECK_EOS(message);
								if(ISDIGIT(*message) && *message < '8'){
									num<<=3;
									num+=*message-'0'; // 2nd octal digit
									++message;
									CHECK_EOS(message);
									if(ISDIGIT(*message) && *message < '8'){
										num<<=3;
										num+=*message-'0'; // 3rd octal digit
										++message;
										CHECK_EOS(message);
									}
								}
								WFIFOB(sd->fd,off)=TOB(num);
								continue;
							}
						}
					} else
						num=*message;
					WFIFOB(sd->fd,off)=TOB(num);
					++message;
					CHECK_EOS(message);
				}//for
				while(*message != '"')
				{// ignore extra characters
					++message;
					CHECK_EOS(message);
				}

				// terminate the string
				if(off < end)
				{// fill the rest with 0's
					memset(WFIFOP(sd->fd,off),0,end-off);
					off=end;
				}
			} else
			{// unknown
				PARSE_ERROR(msg_fd(fd, MSGTBL_UNKNOWN_VALUE_TYPE),message); // Unknown type of value in:
				return false;
			}
			SKIP_VALUE(message);
		}

		if (packets->db[type] == -1) { // send dynamic packet
			WFIFOW(sd->fd,2)=TOW(off);
			WFIFOSET(sd->fd,off);
		} else {// send static packet
			if(off < len)
				memset(WFIFOP(sd->fd,off),0,len-off);
			WFIFOSET(sd->fd,len);
		}
	} else {
		clif->message(fd, msg_fd(fd, MSGTBL_SEND_INVALID_PACKET)); // Invalid packet
		return false;
	}
	sprintf (atcmd_output, msg_fd(fd, MSGTBL_SEND_PACKET_SENT), type, type); // Sent packet 0x%x (%d)
	clif->message(fd, atcmd_output);
	return true;
#undef PARSE_ERROR
#undef CHECK_EOS
#undef SKIP_VALUE
#undef GET_VALUE
}

/*==========================================
 * @rura, @warp, @mapmove
 *------------------------------------------*/
ACMD(mapmove)
{
	char map_name[MAP_NAME_LENGTH_EXT];
	unsigned short map_index;
	short x = 0, y = 0;
	int16 m = -1;

	memset(map_name, '\0', sizeof(map_name));

	if (!*message ||
		(sscanf(message, "%15s %5hd %5hd", map_name, &x, &y) < 3 &&
		 sscanf(message, "%15[^,],%5hd,%5hd", map_name, &x, &y) < 1)) {
			clif->message(fd, msg_fd(fd, MSGTBL_ENTER_MAP_NAME)); // Please enter a map (usage: @warp/@rura/@mapmove <mapname> <x> <y>).
			return false;
		}

	map_index = mapindex->name2id(map_name);
	if (map_index)
		m = map->mapindex2mapid(map_index);

	if (!map_index || m < 0) { // m < 0 means on different server or that map is disabled! [Kevin]
		clif->message(fd, msg_fd(fd, MSGTBL_MAP_NOT_FOUND)); // Map not found.
		return false;
	}

	if( sd->bl.m == m && sd->bl.x == x && sd->bl.y == y ) {
		clif->message(fd, msg_fd(fd, MSGTBL_WARP_SAMEPLACE)); // You already are at your destination!
		return false;
	}

	if ((x || y) && map->getcell(m, &sd->bl, x, y, CELL_CHKNOPASS) && pc_get_group_level(sd) < battle_config.gm_ignore_warpable_area) {
		//This is to prevent the pc->setpos call from printing an error.
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_COORDINATES));
		if (map->search_free_cell(NULL, m, &x, &y, 10, 10, SFC_XY_CENTER) != 0)
			x = y = 0; //Invalid cell, use random spot.
	}
	if (map->list[m].flag.nowarpto && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_fd(fd, MSGTBL_CANT_WARP_TO)); // You are not authorized to warp to this map.
		return false;
	}
	if (sd->bl.m >= 0 && map->list[sd->bl.m].flag.nowarp && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_fd(fd, MSGTBL_CANT_WARP_FROM)); // You are not authorized to warp from your current map.
		return false;
	}
	if (pc->setpos(sd, map_index, x, y, CLR_TELEPORT) != 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_MAP_NOT_FOUND)); // Map not found.
		return false;
	}

	clif->message(fd, msg_fd(fd, MSGTBL_WARPED)); // Warped.
	return true;
}

/*==========================================
 * Displays where a character is. Corrected version by Silent. [Skotlex]
 *------------------------------------------*/
ACMD(where)
{
	struct map_session_data* pl_sd;

	memset(atcmd_player_name, '\0', sizeof atcmd_player_name);

	if (!*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_PLAYER_NAME)); // Please enter a player name (usage: @where <char name>).
		return false;
	}

	pl_sd = map->nick2sd(atcmd_player_name, true);
	if (pl_sd == NULL ||
	    strncmp(pl_sd->status.name, atcmd_player_name, NAME_LENGTH) != 0 ||
	    (pc_has_permission(pl_sd, PC_PERM_HIDE_SESSION) && pc_get_group_level(pl_sd) > pc_get_group_level(sd) && !pc_has_permission(sd, PC_PERM_WHO_DISPLAY_AID))
		) {
		clif->message(fd, msg_fd(fd, MSGTBL_CHARACTER_NOT_FOUND)); // Character not found.
		return false;
	}

	snprintf(atcmd_output, sizeof atcmd_output, "%s %s %d %d", pl_sd->status.name, mapindex_id2name(pl_sd->mapindex), pl_sd->bl.x, pl_sd->bl.y);
	clif->message(fd, atcmd_output);

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(jumpto)
{
	struct map_session_data *pl_sd = NULL;

	if (!*message) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_PLAYER_NAME_OR_ID)); // Please enter a player name (usage: @jumpto/@warpto/@goto <char name/ID>).
		return false;
	}

	if (sd->bl.m >= 0 && map->list[sd->bl.m].flag.nowarp && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_fd(fd, MSGTBL_CANT_WARP_FROM)); // You are not authorized to warp from your current map.
		return false;
	}

	if( pc_isdead(sd) ) {
		clif->message(fd, msg_fd(fd, MSGTBL_CANNOT_USE_WHEN_DEAD)); // "You cannot use this command when dead."
		return false;
	}

	if ((pl_sd=map->nick2sd(message, true)) == NULL && (pl_sd=map->charid2sd(atoi(message))) == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_CHARACTER_NOT_FOUND)); // Character not found.
		return false;
	}

	if (pl_sd->bl.m >= 0 && map->list[pl_sd->bl.m].flag.nowarpto && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_fd(fd, MSGTBL_CANT_WARP_TO)); // You are not authorized to warp to this map.
		return false;
	}

	if( pl_sd->bl.m == sd->bl.m && pl_sd->bl.x == sd->bl.x && pl_sd->bl.y == sd->bl.y ) {
		clif->message(fd, msg_fd(fd, MSGTBL_WARP_SAMEPLACE)); // You already are at your destination!
		return false;
	}

	pc->setpos(sd, pl_sd->mapindex, pl_sd->bl.x, pl_sd->bl.y, CLR_TELEPORT);
	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_JUMP_TO_NAME), pl_sd->status.name); // Jumped to %s
	clif->message(fd, atcmd_output);

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(jump)
{
	short x = 0, y = 0;

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	sscanf(message, "%5hd %5hd", &x, &y);

	if (map->list[sd->bl.m].flag.noteleport && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_fd(fd, MSGTBL_CANT_WARP_FROM)); // You are not authorized to warp from your current map.
		return false;
	}

	if( pc_isdead(sd) ) {
		clif->message(fd, msg_fd(fd, MSGTBL_CANNOT_USE_WHEN_DEAD)); // "You cannot use this command when dead."
		return false;
	}

	if ((x || y) && map->getcell(sd->bl.m, &sd->bl, x, y, CELL_CHKNOPASS)) {
		//This is to prevent the pc->setpos call from printing an error.
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_COORDINATES));
		if (map->search_free_cell(NULL, sd->bl.m, &x, &y, 10, 10, SFC_XY_CENTER) != 0)
			x = y = 0; //Invalid cell, use random spot.
	}

	if (x && y && sd->bl.x == x && sd->bl.y == y) {
		clif->message(fd, msg_fd(fd, MSGTBL_WARP_SAMEPLACE)); // You already are at your destination!
		return false;
	}

	pc->setpos(sd, sd->mapindex, x, y, CLR_TELEPORT);
	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_JUMP_TO_XY), sd->bl.x, sd->bl.y); // Jumped to %d %d
	clif->message(fd, atcmd_output);
	return true;
}

/*==========================================
 * Display list of online characters with
 * various info.
 *------------------------------------------*/
ACMD(who)
{
	const struct map_session_data *pl_sd = NULL;
	struct s_mapiterator *iter = NULL;
	char player_name[NAME_LENGTH] = "";
	int count = 0;
	int level = 0;
	StringBuf buf;
	/**
	 * 1 = @who  : Player name, [Title], [Party name], [Guild name]
	 * 2 = @who2 : Player name, [Title], BLvl, JLvl, Job
	 * 3 = @who3 : [CID/AID] Player name [Title], Map, X, Y
	 */
	int display_type = 1;
	int map_id = -1;

	if (stristr(info->command, "map") != NULL) {
		char map_name[MAP_NAME_LENGTH_EXT] = "";
		if (sscanf(message, "%15s %23s", map_name, player_name) < 1 || (map_id = map->mapname2mapid(map_name)) < 0)
			map_id = sd->bl.m;
	} else {
		sscanf(message, "%23s", player_name);
	}

	if (stristr(info->command, "2") != NULL)
		display_type = 2;
	else if (stristr(info->command, "3") != NULL)
		display_type = 3;

	level = pc_get_group_level(sd);
	StrBuf->Init(&buf);

	iter = mapit_getallusers();
	for (pl_sd = BL_UCCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); pl_sd = BL_UCCAST(BL_PC, mapit->next(iter))) {
		if (!((pc_has_permission(pl_sd, PC_PERM_HIDE_SESSION) || pc_isinvisible(pl_sd)) && pc_get_group_level(pl_sd) > level)) { // you can look only lower or same level
			if (stristr(pl_sd->status.name, player_name) == NULL // search with no case sensitive
				|| (map_id >= 0 && pl_sd->bl.m != map_id))
				continue;
			switch (display_type) {
				case 2: {
					StrBuf->Printf(&buf, msg_fd(fd, MSGTBL_WHO_NAME_FORMAT), pl_sd->status.name); // "Name: %s "
					if (pc_get_group_id(pl_sd) > 0) // Player title, if exists
						StrBuf->Printf(&buf, msg_fd(fd, MSGTBL_WHO_TITLE_FORMAT), pcg->get_name(pl_sd->group)); // "(%s) "
					StrBuf->Printf(&buf, msg_fd(fd, MSGTBL_WHO_LEVEL_JOB_FORMAT), pl_sd->status.base_level, pl_sd->status.job_level,
									 pc->job_name(pl_sd->status.class)); // "| Lv:%d/%d | Job: %s"
					break;
				}
				case 3: {
					if (pc_has_permission(sd, PC_PERM_WHO_DISPLAY_AID))
						StrBuf->Printf(&buf, msg_fd(fd, MSGTBL_WHO_PLAYER_INFO), pl_sd->status.char_id, pl_sd->status.account_id); // "(CID:%d/AID:%d) "
					StrBuf->Printf(&buf, msg_fd(fd, MSGTBL_WHO_NAME_FORMAT), pl_sd->status.name); // "Name: %s "
					if (pc_get_group_id(pl_sd) > 0) // Player title, if exists
						StrBuf->Printf(&buf, msg_fd(fd, MSGTBL_WHO_TITLE_FORMAT), pcg->get_name(pl_sd->group)); // "(%s) "
					StrBuf->Printf(&buf, msg_fd(fd, MSGTBL_WHO_LOCATION_FORMAT), mapindex_id2name(pl_sd->mapindex), pl_sd->bl.x, pl_sd->bl.y); // "| Location: %s %d %d"
					break;
				}
				default: {
					struct party_data *p = party->search(pl_sd->status.party_id);
					struct guild *g = pl_sd->guild;

					StrBuf->Printf(&buf, msg_fd(fd, MSGTBL_WHO_NAME_FORMAT), pl_sd->status.name); // "Name: %s "
					if (pc_get_group_id(pl_sd) > 0) // Player title, if exists
						StrBuf->Printf(&buf, msg_fd(fd, MSGTBL_WHO_TITLE_FORMAT), pcg->get_name(pl_sd->group)); // "(%s) "
					if (p != NULL)
						StrBuf->Printf(&buf, msg_fd(fd, MSGTBL_WHO_PARTY_FORMAT), p->party.name); // " | Party: '%s'"
					if (g != NULL)
						StrBuf->Printf(&buf, msg_fd(fd, MSGTBL_WHO_GUILD_FORMAT), g->name); // " | Guild: '%s'"
					break;
				}
			}
			clif->messagecolor_self(fd, COLOR_DEFAULT, StrBuf->Value(&buf));/** for whatever reason clif->message crashes with some patterns, see bugreport:8186 **/
			StrBuf->Clear(&buf);
			count++;
		}
	}
	mapit->free(iter);

	if (map_id < 0) {
		if (count == 0)
			StrBuf->AppendStr(&buf, msg_fd(fd, MSGTBL_NO_PLAYER_FOUND)); // No player found.
		else if (count == 1)
			StrBuf->AppendStr(&buf, msg_fd(fd, MSGTBL_1_PLAYER_FOUND)); // 1 player found.
		else
			StrBuf->Printf(&buf, msg_fd(fd, MSGTBL_N_PLAYERS_FOUND), count); // %d players found.
	} else {
		if (count == 0)
			StrBuf->Printf(&buf, msg_fd(fd, MSGTBL_NO_PLAYER_IN_MAP), map->list[map_id].name); // No player found in map '%s'.
		else if (count == 1)
			StrBuf->Printf(&buf, msg_fd(fd, MSGTBL_1_PLAYER_IN_MAP), map->list[map_id].name); // 1 player found in map '%s'.
		else
			StrBuf->Printf(&buf, msg_fd(fd, MSGTBL_N_PLAYERS_IN_MAP), count, map->list[map_id].name); // %d players found in map '%s'.
	}
	clif->message(fd, StrBuf->Value(&buf));
	StrBuf->Destroy(&buf);
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(whogm)
{
	const struct map_session_data *pl_sd;
	struct s_mapiterator* iter;
	int j, count;
	int level;
	char match_text[CHAT_SIZE_MAX];
	char player_name[NAME_LENGTH];
	struct guild *g;
	struct party_data *p;

	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(match_text, '\0', sizeof(match_text));
	memset(player_name, '\0', sizeof(player_name));

	if (sscanf(message, "%199[^\n]", match_text) < 1)
		strcpy(match_text, "");
	for (j = 0; match_text[j]; j++)
		match_text[j] = TOLOWER(match_text[j]);

	count = 0;
	level = pc_get_group_level(sd);

	iter = mapit_getallusers();
	for (pl_sd = BL_UCCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); pl_sd = BL_UCCAST(BL_PC, mapit->next(iter))) {
		int pl_level = pc_get_group_level(pl_sd);
		if (!pl_level)
			continue;

		if (match_text[0]) {
			memcpy(player_name, pl_sd->status.name, NAME_LENGTH);
			for (j = 0; player_name[j]; j++)
				player_name[j] = TOLOWER(player_name[j]);
			// search with no case sensitive
			if (strstr(player_name, match_text) == NULL)
				continue;
		}
		if (pl_level > level) {
			if (pc_isinvisible(pl_sd))
				continue;
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_WHOGM_NAME_GM), pl_sd->status.name); // Name: %s (GM)
			clif->message(fd, atcmd_output);
			count++;
			continue;
		}

		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_WHOGM_NAME_LOCATION), // Name: %s (GM:%d) | Location: %s %d %d
				pl_sd->status.name, pl_level,
				mapindex_id2name(pl_sd->mapindex), pl_sd->bl.x, pl_sd->bl.y);
		clif->message(fd, atcmd_output);

		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_WHOGM_BLEVEL_JLEVEL), // BLvl: %d | Job: %s (Lvl: %d)
				pl_sd->status.base_level,
				pc->job_name(pl_sd->status.class), pl_sd->status.job_level);
		clif->message(fd, atcmd_output);

		p = party->search(pl_sd->status.party_id);
		g = pl_sd->guild;

		snprintf(atcmd_output, sizeof(atcmd_output),msg_fd(fd, MSGTBL_WHOGM_PARTY_GUILD), // Party: '%s' | Guild: '%s'
				p?p->party.name:msg_fd(fd, MSGTBL_WHOGM_NONE), g?g->name:msg_fd(fd, MSGTBL_WHOGM_NONE)); // None.

		clif->message(fd, atcmd_output);
		count++;
	}
	mapit->free(iter);

	if (count == 0)
		clif->message(fd, msg_fd(fd, MSGTBL_NO_GM_FOUND)); // No GM found.
	else if (count == 1)
		clif->message(fd, msg_fd(fd, MSGTBL_1_GM_FOUND)); // 1 GM found.
	else {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_N_GMS_FOUND), count); // %d GMs found.
		clif->message(fd, atcmd_output);
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(save)
{
	pc->setsavepoint(sd, sd->mapindex, sd->bl.x, sd->bl.y);
	if (sd->status.pet_id > 0 && sd->pd)
		intif->save_petdata(sd->status.account_id, &sd->pd->pet);

	chrif->save(sd,0);

	clif->message(fd, msg_fd(fd, MSGTBL_SAVE_POINT_CHANGED)); // Your save point has been changed.

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(load)
{
	int16 m;

	m = map->mapindex2mapid(sd->status.save_point.map);
	if (m >= 0 && map->list[m].flag.nowarpto && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_fd(fd, MSGTBL_CANT_WARP_TO_SAVE_POINT)); // You are not authorized to warp to your save map.
		return false;
	}
	if (sd->bl.m >= 0 && map->list[sd->bl.m].flag.nowarp && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_fd(fd, MSGTBL_CANT_WARP_FROM)); // You are not authorized to warp from your current map.
		return false;
	}

	pc->setpos(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, CLR_OUTSIGHT);
	clif->message(fd, msg_fd(fd, MSGTBL_WARP_TO_SAVE_POINT)); // Warping to save point..

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(speed)
{
	int speed;

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!*message || sscanf(message, "%12d", &speed) < 1) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_ENTER_SPEED_VALUE), MIN_WALK_SPEED, MAX_WALK_SPEED); // Please enter a speed value (usage: @speed <%d-%d>).
		clif->message(fd, atcmd_output);
		return false;
	}

	sd->state.permanent_speed = 0;

	if (speed < 0)
		sd->base_status.speed = DEFAULT_WALK_SPEED;
	else
		sd->base_status.speed = cap_value(speed, MIN_WALK_SPEED, MAX_WALK_SPEED);

	if( sd->base_status.speed != DEFAULT_WALK_SPEED ) {
		sd->state.permanent_speed = 1; // Set lock when set to non-default speed.
		clif->message(fd, msg_fd(fd, MSGTBL_SPEED_CHANGED)); // Speed changed.
	} else
		clif->message(fd, msg_fd(fd, MSGTBL_SPEED_NORMAL)); //Speed returned to normal.

	status_calc_bl(&sd->bl, SCB_SPEED);

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(storage)
{
	if (sd->npc_id || sd->state.vending || sd->state.prevend || sd->state.buyingstore || sd->state.trading || sd->state.storage_flag)
		return false;

	if (!pc_has_permission(sd, PC_PERM_BYPASS_NOSTORAGE) && (map->list[sd->bl.m].flag.nostorage & 1)) { // mapflag nostorage already defined? can't open :c
		clif->message(fd, msg_fd(fd, MSGTBL_CANNOT_OPEN_STORAGE)); // You currently cannot open your storage.
		return false;
	}

	if (storage->open(sd) == 1) { //Already open.
		clif->message(fd, msg_fd(fd, MSGTBL_STORAGE_ALREADY_OPEN)); // You have already opened your storage. Close it first.
		return false;
	}

	clif->message(fd, msg_fd(fd, MSGTBL_STORAGE_OPENED)); // Storage opened.

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(guildstorage)
{
	int retval;

	if (!sd->status.guild_id) {
		clif->message(fd, msg_fd(fd, MSGTBL_NOT_IN_A_GUILD2)); // You are not in a guild.
		return false;
	}

	if (sd->npc_id || sd->state.vending || sd->state.prevend || sd->state.buyingstore || sd->state.trading)
		return false;

	if (sd->state.storage_flag == STORAGE_FLAG_NORMAL) {
		clif->message(fd, msg_fd(fd, MSGTBL_STORAGE_ALREADY_OPEN)); // You have already opened your storage. Close it first.
		return false;
	}

	if (sd->state.storage_flag == STORAGE_FLAG_GUILD) {
		clif->message(fd, msg_fd(fd, MSGTBL_GSTORAGE_ALREADY_OPEN)); // You have already opened your guild storage. Close it first.
		return false;
	}

	if (!pc_has_permission(sd, PC_PERM_BYPASS_NOSTORAGE) && (map->list[sd->bl.m].flag.nogstorage & 1)) { // mapflag nogstorage already defined? can't open :c
		clif->message(fd, msg_fd(fd, MSGTBL_CANNOT_OPEN_STORAGE)); // You currently cannot open your storage. (there is no other messages...)
		return false;
	}

	if ((retval = gstorage->open(sd)) != 0) {
		if (retval == 2)
			clif->message(fd, msg_fd(fd, MSGTBL_NOT_IN_A_GUILD2)); // You are not in a guild
		else if (retval == 3)
			clif->message(fd, msg_fd(fd, MSGTBL_GUILD_DOES_NOT_HAVE_STORAGE)); // Your guild doesn't have storage!
		else if (retval == 4)
			clif->message(fd, msg_fd(fd, MSGTBL_NOT_AUTHORIZED_TO_USE_GSTORAGE)); // You're not authorized to open your guild storage!
		else // retval == 1 or unknown results
			clif->message(fd, msg_fd(fd, MSGTBL_GUILDSTORAGE_ALREADY_OPENED)); // Your guild's storage has already been opened by another member, try again later.
		return false;
	}

	clif->message(fd, msg_fd(fd, MSGTBL_GUILD_STORAGE_OPENED)); // Guild storage opened.
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(option)
{
	int param1 = 0, param2 = 0, param3 = 0;

	if (!*message || sscanf(message, "%12d %12d %12d", &param1, &param2, &param3) < 1 || param1 < 0 || param2 < 0 || param3 < 0)
	{// failed to match the parameters so inform the user of the options
		const char* text;

		// attempt to find the setting information for this command
		text = atcommand_help_string( info );

		// notify the user of the requirement to enter an option
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_LEAST_ONE_OPTION)); // Please enter at least one option.

		if( text ) {// send the help text associated with this command
			clif->messageln( fd, text );
		}

		return false;
	}

	sd->sc.opt1 = param1;
	sd->sc.opt2 = param2;
	pc->setoption(sd, param3);

	clif->message(fd, msg_fd(fd, MSGTBL_OPTIONS_CHANGED)); // Options changed.

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(hide)
{
	if (pc_isinvisible(sd))
		pc->unhide(sd, true);
	else
		pc->hide(sd, true);

	return true;
}

/*==========================================
 * Changes a character's class
 *------------------------------------------*/
ACMD(jobchange)
{
	int job = 0, upper = 0;
	bool found = false;

	if (*message == '\0') { // No message, just show the list
		found = false;
	} else if (sscanf(message, "%12d %12d", &job, &upper) >= 1) { // Numeric job ID
		found = true;
	} else { // Job name
		int i;

		// Normal Jobs
		for (i = JOB_NOVICE; !found && i < JOB_MAX_BASIC; i++) {
			if (strncmpi(message, pc->job_name(i), 16) == 0) {
				job = i;
				found = true;
				break;
			}
		}

		// High Jobs, Babies and Third
		for (i = JOB_NOVICE_HIGH; !found && i < JOB_MAX; i++) {
			if (strncmpi(message, pc->job_name(i), 16) == 0) {
				job = i;
				found = true;
				break;
			}
		}
	}

	if (!found || !pc->db_checkid(job)) {
		const char *text = atcommand_help_string(info);
		if (text)
			clif->messageln(fd, text);
		return false;
	}

	// Deny direct transformation into dummy jobs
	if (job == JOB_KNIGHT2 || job == JOB_CRUSADER2
	 || job == JOB_WEDDING || job == JOB_XMAS || job == JOB_SUMMER
	 || job == JOB_LORD_KNIGHT2 || job == JOB_PALADIN2
	 || job == JOB_BABY_KNIGHT2 || job == JOB_BABY_CRUSADER2
	 || job == JOB_STAR_GLADIATOR2 || job == JOB_BABY_STAR_GLADIATOR2
	 || (job >= JOB_RUNE_KNIGHT2 && job <= JOB_MECHANIC_T2)
	 || (job >= JOB_BABY_RUNE2 && job <= JOB_BABY_MECHANIC2)
	) {
		/* WHY DO WE LIST THEM THEN? */
		clif->message(fd, msg_fd(fd, MSGTBL_CANNOT_CHANGE_JOB_COMMAND)); //"You can not change to this job by command."
		return true;
	}

	if (pc->jobchange(sd, job, upper) != 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_CANT_CHANGE_JOB)); // You are unable to change your job.
		return false;
	}

	clif->message(fd, msg_fd(fd, MSGTBL_JOB_CHANGED)); // Your job has been changed.
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(kill)
{
	status_kill(&sd->bl);
	clif->message(sd->fd, msg_fd(fd, MSGTBL_YOU_DIED)); // A pity! You've died.
	if (fd != sd->fd)
		clif->message(fd, msg_fd(fd, MSGTBL_CHARACTER_KILLED)); // Character killed.
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(alive)
{
	if (!status->revive(&sd->bl, 100, 100)) {
		clif->message(fd, msg_fd(fd, MSGTBL_NOT_DEAD)); // "You're not dead."
		return false;
	}
	clif->skill_nodamage(&sd->bl,&sd->bl,ALL_RESURRECTION,4,1);
	clif->message(fd, msg_fd(fd, MSGTBL_REVIVED)); // You've been revived! It's a miracle!
	return true;
}

/*==========================================
 * +kamic [LuzZza]
 *------------------------------------------*/
ACMD(kami)
{
	unsigned int color=0;

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if(*(info->command + 4) != 'c' && *(info->command + 4) != 'C') {
		if (!*message) {
			clif->message(fd, msg_fd(fd, MSGTBL_KAMI_ENTER_MSG)); // Please enter a message (usage: @kami <message>).
			return false;
		}

		sscanf(message, "%199[^\n]", atcmd_output);
		if (stristr(info->command, "l") != NULL)
			clif->broadcast(&sd->bl, atcmd_output, (int)strlen(atcmd_output) + 1, BC_DEFAULT, ALL_SAMEMAP);
		else if (info->command[4] == 'b' || info->command[4] == 'B')
			clif->broadcast(NULL, atcmd_output, (int)strlen(atcmd_output) + 1, BC_BLUE, ALL_CLIENT);
		else
			clif->broadcast(NULL, atcmd_output, (int)strlen(atcmd_output) + 1, BC_YELLOW, ALL_CLIENT);
	} else {
		if(!*message || (sscanf(message, "%10u %199[^\n]", &color, atcmd_output) < 2)) {
			clif->message(fd, msg_fd(fd, MSGTBL_KAMI_ENTER_COLOR_MSG)); // Please enter color and message (usage: @kamic <color> <message>).
			return false;
		}

		if(color > 0xFFFFFF) {
			clif->message(fd, msg_fd(fd, MSGTBL_KAMI_INVALID_COLOR)); // Invalid color.
			return false;
		}
		clif->broadcast2(NULL, atcmd_output, (int)strlen(atcmd_output) + 1, color, 0x190, 12, 0, 0, ALL_CLIENT);
	}
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(heal)
{
	int hp = 0, sp = 0; // [Valaris] thanks to fov

	sscanf(message, "%12d %12d", &hp, &sp);

	// some overflow checks
	if( hp == INT_MIN ) hp++;
	if( sp == INT_MIN ) sp++;

	if ( hp == 0 && sp == 0 ) {
		if (!status_percent_heal(&sd->bl, 100, 100))
			clif->message(fd, msg_fd(fd, MSGTBL_HEAL_ALREADY_FULL)); // HP and SP have already been recovered.
		else
			clif->message(fd, msg_fd(fd, MSGTBL_HP_SP_RECOVERED)); // HP, SP recovered.
		return true;
	}

	if ( hp > 0 && sp >= 0 ) {
		if (status->heal(&sd->bl, hp, sp, STATUS_HEAL_DEFAULT) == 0)
			clif->message(fd, msg_fd(fd, MSGTBL_HEAL_ALREADY_FULL)); // HP and SP are already with the good value.
		else
			clif->message(fd, msg_fd(fd, MSGTBL_HP_SP_RECOVERED)); // HP, SP recovered.
		return true;
	}

	if ( hp < 0 && sp <= 0 ) {
		status->damage(NULL, &sd->bl, -hp, -sp, 0, 0);
		clif->damage(&sd->bl,&sd->bl, 0, 0, -hp, 0, BDT_ENDURE, 0);
		clif->message(fd, msg_fd(fd, MSGTBL_HEAL_RECOVERED)); // HP or/and SP modified.
		return true;
	}

	//Opposing signs.
	if ( hp ) {
		if (hp > 0)
			status->heal(&sd->bl, hp, 0, STATUS_HEAL_DEFAULT);
		else {
			status->damage(NULL, &sd->bl, -hp, 0, 0, 0);
			clif->damage(&sd->bl,&sd->bl, 0, 0, -hp, 0, BDT_ENDURE, 0);
		}
	}

	if ( sp ) {
		if (sp > 0)
			status->heal(&sd->bl, 0, sp, STATUS_HEAL_DEFAULT);
		else
			status->damage(NULL, &sd->bl, 0, -sp, 0, 0);
	}

	clif->message(fd, msg_fd(fd, MSGTBL_HEAL_RECOVERED)); // HP or/and SP modified.
	return true;
}

/*==========================================
 * @item command (usage: @item <name/id_of_item> <quantity>) (modified by [Yor] for pet_egg)
 * @itembound command (usage: @itembound <name/id_of_item> <quantity> <bound type>) (revised by [Mhalicot])
 *------------------------------------------*/
ACMD(item)
{
	char item_name[100];
	int number = 0, item_id, flag = 0, bound = 0;
	struct item item_tmp;
	struct item_data *item_data;
	int get_count, i;

	memset(item_name, '\0', sizeof(item_name));

	if (!strcmpi(info->command, "itembound") && (!*message || (
		sscanf(message, "\"%99[^\"]\" %12d %12d", item_name, &number, &bound) < 1 &&
		sscanf(message, "%99s %12d %12d", item_name, &number, &bound) < 1
		))) {
		clif->message(fd, msg_fd(fd, MSGTBL_ITEMBOUND_USAGE)); // Please enter an item name or ID (usage: @itembound <item name/ID> <quantity> <bound_type>).
		return false;
	} else if (!*message
		|| (sscanf(message, "\"%99[^\"]\" %12d", item_name, &number) < 1
			&& sscanf(message, "%99s %12d", item_name, &number) < 1
			)) {
		clif->message(fd, msg_fd(fd, MSGTBL_ITEM_ENTER_NAME_OR_ID)); // Please enter an item name or ID (usage: @item <item name/ID> <quantity>).
		return false;
	}

	if (number <= 0)
		number = 1;

	if ((item_data = itemdb->search_name(item_name)) == NULL &&
		(item_data = itemdb->exists(atoi(item_name))) == NULL)
	{
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_ITEMID_NAME)); // Invalid item ID or name.
		return false;
	}

	if (!strcmpi(info->command, "itembound")) {
		if (!(bound >= IBT_MIN && bound <= IBT_MAX)) {
			clif->message(fd, msg_fd(fd, MSGTBL_ITEMBOUND_INVALID_TYPE)); // Invalid bound type
			return false;
		}
		switch ((enum e_item_bound_type)bound) {
		case IBT_CHARACTER:
		case IBT_ACCOUNT:
			break; /* no restrictions */
		case IBT_PARTY:
			if (!sd->status.party_id) {
				clif->message(fd, msg_fd(fd, MSGTBL_CANNOT_ADD_PARTY_ITEM)); //You can't add a party bound item to a character without party!
				return false;
			}
			break;
		case IBT_GUILD:
			if (!sd->status.guild_id) {
				clif->message(fd, msg_fd(fd, MSGTBL_CANNOT_ADD_GUILD_ITEM)); //You can't add a guild bound item to a character without guild!
				return false;
			}
			break;
		case IBT_NONE:
			break;
		}
	}

	item_id = item_data->nameid;
	get_count = number;
	//Check if it's stackable.
	if (!itemdb->isstackable2(item_data)) {
		if (bound && (item_data->type == IT_PETEGG || item_data->type == IT_PETARMOR)) {
			clif->message(fd, msg_fd(fd, MSGTBL_ITEMBOUND_CANT_CREATE_PET_ITEMS)); // Cannot create bounded pet eggs or pet armors.
			return false;
		}
		get_count = 1;
	}

	for (i = 0; i < number; i += get_count) {
		// if not pet egg
		if (!pet->create_egg(sd, item_id)) {
			memset(&item_tmp, 0, sizeof(item_tmp));
			item_tmp.nameid = item_id;
			item_tmp.identify = 1;
			item_tmp.bound = (unsigned char)bound;

			if ((flag = pc->additem(sd, &item_tmp, get_count, LOG_TYPE_COMMAND)))
				clif->additem(sd, 0, 0, flag);
		}
	}

	if (flag == 0)
		clif->message(fd, msg_fd(fd, MSGTBL_ITEM_CREATED)); // Item created.
	return true;
}

/*==========================================
 * @item2 and @itembound2 command (revised by [Mhalicot])
 *------------------------------------------*/
ACMD(item2)
{
	struct item item_tmp;
	struct item_data *item_data;
	char item_name[100];
	int item_id, number = 0, bound = 0;
	int identify = 1, refine_level = 0, attr = ATTR_NONE;
	int c1 = 0, c2 = 0, c3 = 0, c4 = 0;

	memset(item_name, '\0', sizeof(item_name));

	if (!strcmpi(info->command, "itembound2") && (!*message || (
		sscanf(message, "\"%99[^\"]\" %12d %12d %12d %12d %12d %12d %12d %12d %12d", item_name, &number, &identify, &refine_level, &attr, &c1, &c2, &c3, &c4, &bound) < 10 &&
		sscanf(message, "%99s %12d %12d %12d %12d %12d %12d %12d %12d %12d", item_name, &number, &identify, &refine_level, &attr, &c1, &c2, &c3, &c4, &bound) < 10))) {
		clif->message(fd, msg_fd(fd, MSGTBL_ITEMBOUND2_USAGE)); // Please enter all parameters (usage: @itembound2 <item name/ID> <quantity>
		clif->message(fd, msg_fd(fd, MSGTBL_ITEMBOUND_USAGE2)); //   <identify_flag> <refine> <attribute> <card1> <card2> <card3> <card4> <bound_type>).
		return false;
	} else if (!*message
		|| (sscanf(message, "\"%99[^\"]\" %12d %12d %12d %12d %12d %12d %12d %12d", item_name, &number, &identify, &refine_level, &attr, &c1, &c2, &c3, &c4) < 1
			&& sscanf(message, "%99s %12d %12d %12d %12d %12d %12d %12d %12d", item_name, &number, &identify, &refine_level, &attr, &c1, &c2, &c3, &c4) < 1
			)) {
		clif->message(fd, msg_fd(fd, MSGTBL_ITEM2_ENTER_ALL_PARAM)); // Please enter all parameters (usage: @item2 <item name/ID> <quantity>
		clif->message(fd, msg_fd(fd, MSGTBL_ITEM2_PARAMETERS)); //   <identify_flag> <refine> <attribute> <card1> <card2> <card3> <card4>).
		return false;
	}

	if (number <= 0)
		number = 1;

	if (!strcmpi(info->command, "itembound2") && !(bound >= IBT_MIN && bound <= IBT_MAX)) {
		clif->message(fd, msg_fd(fd, MSGTBL_ITEMBOUND_INVALID_TYPE)); // Invalid bound type
		return false;
	}

	item_id = 0;
	if ((item_data = itemdb->search_name(item_name)) != NULL ||
		(item_data = itemdb->exists(atoi(item_name))) != NULL)
		item_id = item_data->nameid;

	if (item_id > 500) {
		int flag = 0;
		int loop, get_count, i;
		loop = 1;
		get_count = number;
		if (!strcmpi(info->command, "itembound2"))
			bound = IBT_ACCOUNT;
		if (!itemdb->isstackable2(item_data)) {
			if (bound && (item_data->type == IT_PETEGG || item_data->type == IT_PETARMOR)) {
				clif->message(fd, msg_fd(fd, MSGTBL_ITEMBOUND_CANT_CREATE_PET_ITEMS)); // Cannot create bounded pet eggs or pet armors.
				return false;
			}
			loop = number;
			get_count = 1;
			if (item_data->type == IT_PETEGG) {
				identify = 1;
				refine_level = 0;
			}
			if (item_data->type == IT_PETARMOR)
				refine_level = 0;
		} else {
			identify = 1;
			refine_level = 0;
			attr = ATTR_NONE;
		}
		refine_level = cap_value(refine_level, 0, MAX_REFINE);
		for (i = 0; i < loop; i++) {
			memset(&item_tmp, 0, sizeof(item_tmp));
			item_tmp.nameid = item_id;
			item_tmp.identify = identify;
			item_tmp.refine = refine_level;
			item_tmp.attribute = attr;
			item_tmp.bound = (unsigned char)bound;
			item_tmp.card[0] = c1;
			item_tmp.card[1] = c2;
			item_tmp.card[2] = c3;
			item_tmp.card[3] = c4;

			if ((flag = pc->additem(sd, &item_tmp, get_count, LOG_TYPE_COMMAND)))
				clif->additem(sd, 0, 0, flag);
		}

		if (flag == 0)
			clif->message(fd, msg_fd(fd, MSGTBL_ITEM_CREATED)); // Item created.
	} else {
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_ITEMID_NAME)); // Invalid item ID or name.
		return false;
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(itemreset)
{
	for (int i = 0; i < sd->status.inventorySize; i++) {
		if (sd->status.inventory[i].amount && sd->status.inventory[i].equip == 0) {
			pc->delitem(sd, i, sd->status.inventory[i].amount, 0, DELITEM_NORMAL, LOG_TYPE_COMMAND);
		}
	}
	clif->message(fd, msg_fd(fd, MSGTBL_ITEMS_REMOVED)); // All of your items have been removed.

	return true;
}

/*==========================================
 * Atcommand @lvlup
 *------------------------------------------*/
ACMD(baselevelup)
{
	int level=0, i=0, status_point=0;

	if (!*message || !(level = atoi(message))) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_LV_ADJUSTMENT)); // Please enter a level adjustment (usage: @lvup/@blevel/@baselvlup <number of levels>).
		return false;
	}

	if (level > 0) {
		if (sd->status.base_level >= pc->maxbaselv(sd)) { // check for max level by Valaris
			clif->message(fd, msg_fd(fd, MSGTBL_CANT_INCREASE_BASE_LEVEL)); // Base level can't go any higher.
			return false;
		} // End Addition
		if (level > pc->maxbaselv(sd) || level > pc->maxbaselv(sd) - sd->status.base_level) // fix positive overflow
			level = pc->maxbaselv(sd) - sd->status.base_level;
		for (i = 0; i < level; i++)
			status_point += pc->gets_status_point(sd->status.base_level + i);

		sd->status.status_point += status_point;
		sd->status.base_level += level;
		status_calc_pc(sd, SCO_FORCE);
		status_percent_heal(&sd->bl, 100, 100);
		clif->misceffect(&sd->bl, 0);
		clif->message(fd, msg_fd(fd, MSGTBL_BASE_LEVEL_RAISED)); // Base level raised.
	} else {
		if (sd->status.base_level == 1) {
			clif->message(fd, msg_fd(fd, MSGTBL_CANT_DECREASE_BASE_LEVEL)); // Base level can't go any lower.
			return false;
		}
		level*=-1;
		if (level >= sd->status.base_level)
			level = sd->status.base_level-1;
		for (i = 0; i > -level; i--)
			status_point += pc->gets_status_point(sd->status.base_level + i - 1);
		if (sd->status.status_point < status_point)
			pc->resetstate(sd);
		if (sd->status.status_point < status_point)
			sd->status.status_point = 0;
		else
			sd->status.status_point -= status_point;
		sd->status.base_level -= level;
		clif->message(fd, msg_fd(fd, MSGTBL_BASE_LEVEL_LOWERED)); // Base level lowered.
		status_calc_pc(sd, SCO_FORCE);
		level *= -1;
	}
	sd->status.base_exp = 0;
	clif->updatestatus(sd, SP_STATUSPOINT);
	clif->updatestatus(sd, SP_BASELEVEL);
	clif->updatestatus(sd, SP_BASEEXP);
	clif->updatestatus(sd, SP_NEXTBASEEXP);
	pc->baselevelchanged(sd);

	// achievements
	achievement->validate_stats(sd, SP_BASELEVEL, sd->status.base_level);

	if(sd->status.party_id)
		party->send_levelup(sd);

	if (level > 0 && battle_config.atcommand_levelup_events)
		npc->script_event(sd, NPCE_BASELVUP); // Trigger OnPCBaseLvUpEvent

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(joblevelup)
{
	int level=0;

	if (!*message || !(level = atoi(message))) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_JLV_ADJUSTMENT)); // Please enter a level adjustment (usage: @joblvup/@jlevel/@joblvlup <number of levels>).
		return false;
	}
	if (level > 0) {
		if (sd->status.job_level >= pc->maxjoblv(sd)) {
			clif->message(fd, msg_fd(fd, MSGTBL_CANT_INCREASE_JOB_LEVEL)); // Job level can't go any higher.
			return false;
		}
		if (level > pc->maxjoblv(sd) || level > pc->maxjoblv(sd) - sd->status.job_level) // fix positive overflow
			level = pc->maxjoblv(sd) - sd->status.job_level;
		sd->status.job_level += level;
		sd->status.skill_point += level;
		clif->misceffect(&sd->bl, 1);
		clif->message(fd, msg_fd(fd, MSGTBL_JOB_LEVEL_RAISED)); // Job level raised.
	} else {
		if (sd->status.job_level == 1) {
			clif->message(fd, msg_fd(fd, MSGTBL_CANT_DECREASE_JOB_LEVEL)); // Job level can't go any lower.
			return false;
		}
		level *=-1;
		if (level >= sd->status.job_level) // fix negative overflow
			level = sd->status.job_level-1;
		sd->status.job_level -= level;
		if (sd->status.skill_point < level)
			pc->resetskill(sd, PCRESETSKILL_NONE); //Reset skills since we need to subtract more points.
		if (sd->status.skill_point < level)
			sd->status.skill_point = 0;
		else
			sd->status.skill_point -= level;
		clif->message(fd, msg_fd(fd, MSGTBL_JOB_LEVEL_LOWERED)); // Job level lowered.
		level *= -1;
	}
	sd->status.job_exp = 0;
	clif->updatestatus(sd, SP_JOBLEVEL);
	clif->updatestatus(sd, SP_JOBEXP);
	clif->updatestatus(sd, SP_NEXTJOBEXP);
	clif->updatestatus(sd, SP_SKILLPOINT);
	status_calc_pc(sd, SCO_FORCE);

	if (level > 0 && battle_config.atcommand_levelup_events)
		npc->script_event(sd, NPCE_JOBLVUP); // Trigger OnPCJobLvUpEvent

	return true;
}

/*==========================================
 * @help
 *------------------------------------------*/
ACMD(help)
{
	const char *command_name = NULL;
	char *default_command = "help";
	AtCommandInfo *tinfo = NULL;

	if (!*message) {
		command_name = default_command; // If no command_name specified, display help for @help.
	} else {
		if (*message == atcommand->at_symbol || *message == atcommand->char_symbol)
			++message;
		command_name = atcommand->check_alias(message);
	}

	if (!atcommand->can_use2(sd, command_name, COMMAND_ATCOMMAND)) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_UNKNOWN_COMMAND), message); // "%s is Unknown Command"
		clif->message(fd, atcmd_output);
		atcommand->get_suggestions(sd, command_name, true);
		return false;
	}

	tinfo = atcommand->get_info_byname(atcommand->check_alias(command_name));

	if ( !tinfo || tinfo->help == NULL ) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_NO_HELP_FOR), atcommand->at_symbol, command_name); // There is no help for %c%s.
		clif->message(fd, atcmd_output);
		atcommand->get_suggestions(sd, command_name, true);
		return false;
	}

	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_HELP_FOR_COMMAND), atcommand->at_symbol, command_name); // Help for command %c%s:
	clif->message(fd, atcmd_output);

	{   // Display aliases
		struct DBIterator *iter;
		AtCommandInfo *command_info;
		AliasInfo *alias_info = NULL;
		StringBuf buf;
		bool has_aliases = false;

		StrBuf->Init(&buf);
		StrBuf->AppendStr(&buf, msg_fd(fd, MSGTBL_HELP_AVAILABLE_ALIASES)); // Available aliases:
		command_info = atcommand->get_info_byname(command_name);
		iter = db_iterator(atcommand->alias_db);
		for (alias_info = dbi_first(iter); dbi_exists(iter); alias_info = dbi_next(iter)) {
			if (alias_info->command == command_info) {
				StrBuf->Printf(&buf, " %s", alias_info->alias);
				has_aliases = true;
			}
		}
		dbi_destroy(iter);
		if (has_aliases)
			clif->message(fd, StrBuf->Value(&buf));
		StrBuf->Destroy(&buf);
	}

	// Display help contents
	clif->messageln(fd, tinfo->help);
	return true;
}

/**
 * Helper function, used in foreach calls to stop auto-attack timers.
 *
 * @see map_foreachinmap
 *
 * Arglist parameters:
 * - (int) id: If 0, stop any attacks. Otherwise, the target block list id to stop attacking.
 */
static int atcommand_stopattack(struct block_list *bl, va_list ap)
{
	struct unit_data *ud = NULL;
	int id = 0;
	nullpo_ret(bl);

	ud = unit->bl2ud(bl);
	id = va_arg(ap, int);

	if (ud && ud->attacktimer != INVALID_TIMER && (!id || id == ud->target)) {
		unit->stop_attack(bl);
		return 1;
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
static int atcommand_pvpoff_sub(struct block_list *bl, va_list ap)
{
	struct map_session_data *sd = NULL;
	nullpo_ret(bl);
	Assert_ret(bl->type == BL_PC);
	sd = BL_UCAST(BL_PC, bl);

	clif->pvpset(sd, 0, 0, 2);
	if (sd->pvp_timer != INVALID_TIMER) {
		timer->delete(sd->pvp_timer, pc->calc_pvprank_timer);
		sd->pvp_timer = INVALID_TIMER;
	}
	return 0;
}

ACMD(pvpoff)
{
	if (!map->list[sd->bl.m].flag.pvp) {
		clif->message(fd, msg_fd(fd, MSGTBL_PVP_ALREADY_OFF)); // PvP is already Off.
		return false;
	}

	map->zone_change2(sd->bl.m,map->list[sd->bl.m].prev_zone);
	map->list[sd->bl.m].flag.pvp = 0;

	if (!battle_config.pk_mode) {
		clif->map_property_mapall(sd->bl.m, MAPPROPERTY_NOTHING);
		clif->maptypeproperty2(&sd->bl,ALL_SAMEMAP);
	}
	map->foreachinmap(atcommand->pvpoff_sub,sd->bl.m, BL_PC);
	map->foreachinmap(atcommand->stopattack,sd->bl.m, BL_CHAR, 0);
	clif->message(fd, msg_fd(fd, MSGTBL_PVP_OFF)); // PvP: Off.
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
static int atcommand_pvpon_sub(struct block_list *bl, va_list ap)
{
	struct map_session_data *sd = NULL;
	nullpo_ret(bl);
	Assert_ret(bl->type == BL_PC);
	sd = BL_UCAST(BL_PC, bl);

	if (sd->pvp_timer == INVALID_TIMER) {
		if (!map->list[sd->bl.m].flag.pvp_nocalcrank)
			sd->pvp_timer = timer->add(timer->gettick() + 200, pc->calc_pvprank_timer, sd->bl.id, 0);
		sd->pvp_rank = 0;
		sd->pvp_lastusers = 0;
		sd->pvp_point = 5;
		sd->pvp_won = 0;
		sd->pvp_lost = 0;
	}
	return 0;
}

ACMD(pvpon)
{
	if (map->list[sd->bl.m].flag.pvp) {
		clif->message(fd, msg_fd(fd, MSGTBL_PVP_ALREADY_ON)); // PvP is already On.
		return false;
	}

	map->zone_change2(sd->bl.m,strdb_get(map->zone_db, MAP_ZONE_PVP_NAME));
	map->list[sd->bl.m].flag.pvp = 1;

	if (!battle_config.pk_mode) {// display pvp circle and rank
		clif->map_property_mapall(sd->bl.m, MAPPROPERTY_FREEPVPZONE);
		clif->maptypeproperty2(&sd->bl,ALL_SAMEMAP);
		map->foreachinmap(atcommand->pvpon_sub,sd->bl.m, BL_PC);
	}

	clif->message(fd, msg_fd(fd, MSGTBL_PVP_ON)); // PvP: On.

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(gvgoff)
{

	if (!map->list[sd->bl.m].flag.gvg) {
		clif->message(fd, msg_fd(fd, MSGTBL_GVG_ALREADY_OFF)); // GvG is already Off.
		return false;
	}

	map->zone_change2(sd->bl.m,map->list[sd->bl.m].prev_zone);
	map->list[sd->bl.m].flag.gvg = 0;
	clif->map_property_mapall(sd->bl.m, MAPPROPERTY_NOTHING);
	clif->maptypeproperty2(&sd->bl,ALL_SAMEMAP);
	map->foreachinmap(atcommand->stopattack,sd->bl.m, BL_CHAR, 0);
	clif->message(fd, msg_fd(fd, MSGTBL_GVG_OFF)); // GvG: Off.

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(gvgon)
{
	if (map->list[sd->bl.m].flag.gvg) {
		clif->message(fd, msg_fd(fd, MSGTBL_GVG_ALREADY_ON)); // GvG is already On.
		return false;
	}

	map->zone_change2(sd->bl.m,strdb_get(map->zone_db, MAP_ZONE_GVG_NAME));
	map->list[sd->bl.m].flag.gvg = 1;
	clif->map_property_mapall(sd->bl.m, MAPPROPERTY_AGITZONE);
	clif->maptypeproperty2(&sd->bl,ALL_SAMEMAP);
	clif->message(fd, msg_fd(fd, MSGTBL_GVG_ON)); // GvG: On.

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(cvcoff)
{
	if (!map->list[sd->bl.m].flag.cvc) {
		clif->message(fd, msg_fd(fd, MSGTBL_CVC_ALREADY_OFF)); // CvC is already Off.
		return false;
	}

	map->zone_change2(sd->bl.m, map->list[sd->bl.m].prev_zone);
	map->list[sd->bl.m].flag.cvc = 0;
	clif->map_property_mapall(sd->bl.m, MAPPROPERTY_NOTHING);
	clif->maptypeproperty2(&sd->bl, ALL_SAMEMAP);
	map->foreachinmap(atcommand->stopattack, sd->bl.m, BL_CHAR, 0);
	clif->message(fd, msg_fd(fd, MSGTBL_CVC_OFF)); // CvC: Off.

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(cvcon)
{
	if (map->list[sd->bl.m].flag.cvc) {
		clif->message(fd, msg_fd(fd, MSGTBL_CVC_ALREADY_ON)); // CvC is already On.
		return false;
	}

	map->zone_change2(sd->bl.m, strdb_get(map->zone_db, MAP_ZONE_CVC_NAME));
	map->list[sd->bl.m].flag.cvc = 1;
	clif->map_property_mapall(sd->bl.m, MAPPROPERTY_AGITZONE);
	clif->maptypeproperty2(&sd->bl, ALL_SAMEMAP);
	clif->message(fd, msg_fd(fd, MSGTBL_CVC_ON)); // CvC: On.

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(model)
{
	int hair_style = 0, hair_color = 0, cloth_color = 0;

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!*message || sscanf(message, "%12d %12d %12d", &hair_style, &hair_color, &cloth_color) < 1) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_MODEL_ENTER_VALUES), // Please enter at least one value (usage: @model <hair ID: %d-%d> <hair color: %d-%d> <clothes color: %d-%d>).
		        MIN_HAIR_STYLE, MAX_HAIR_STYLE, MIN_HAIR_COLOR, MAX_HAIR_COLOR, MIN_CLOTH_COLOR, MAX_CLOTH_COLOR);
		clif->message(fd, atcmd_output);
		return false;
	}

	if (hair_style >= MIN_HAIR_STYLE && hair_style <= MAX_HAIR_STYLE &&
		hair_color >= MIN_HAIR_COLOR && hair_color <= MAX_HAIR_COLOR &&
		cloth_color >= MIN_CLOTH_COLOR && cloth_color <= MAX_CLOTH_COLOR) {
		pc->changelook(sd, LOOK_HAIR, hair_style);
		pc->changelook(sd, LOOK_HAIR_COLOR, hair_color);
		pc->changelook(sd, LOOK_CLOTHES_COLOR, cloth_color);
		clif->message(fd, msg_fd(fd, MSGTBL_APPEARANCE_CHANGED)); // Appearance changed.
	} else {
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_NUMBER)); // An invalid number was specified.
		return false;
	}

	return true;
}

/*==========================================
 * @bodystyle [Rytech]
 *------------------------------------------*/
ACMD(bodystyle)
{
	int body_style = 0;

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!pc->has_second_costume(sd)) {
		clif->message(fd, msg_fd(fd, MSGTBL_NO_ALTERNATE_BODY_AVAILABLE)); // This job has no alternate body styles.
		return false;
	}

	if (*message == '\0' || sscanf(message, "%d", &body_style) < 1) {
		sprintf(atcmd_output, "Please, enter a body style (usage: @bodystyle <body ID: %d-%d>).", MIN_BODY_STYLE, MAX_BODY_STYLE);
		clif->message(fd, atcmd_output);
		return false;
	}

	if (body_style >= MIN_BODY_STYLE && body_style <= MAX_BODY_STYLE) {
		pc->changelook(sd, LOOK_BODY2, body_style);
		clif->message(fd, msg_fd(fd, MSGTBL_APPEARANCE_CHANGED)); // Appearence changed.
	} else {
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_NUMBER)); // An invalid number was specified.
		return false;
	}

	return true;
}

/*==========================================
 * @dye && @ccolor
 *------------------------------------------*/
ACMD(dye)
{
	int cloth_color = 0;

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!*message || sscanf(message, "%12d", &cloth_color) < 1) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_ENTER_CLOTHES_COLOR), MIN_CLOTH_COLOR, MAX_CLOTH_COLOR); // Please enter a clothes color (usage: @dye/@ccolor <clothes color: %d-%d>).
		clif->message(fd, atcmd_output);
		return false;
	}

	if (cloth_color >= MIN_CLOTH_COLOR && cloth_color <= MAX_CLOTH_COLOR) {
		pc->changelook(sd, LOOK_CLOTHES_COLOR, cloth_color);
		clif->message(fd, msg_fd(fd, MSGTBL_APPEARANCE_CHANGED)); // Appearance changed.
	} else {
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_NUMBER)); // An invalid number was specified.
		return false;
	}

	return true;
}

/*==========================================
 * @hairstyle && @hstyle
 *------------------------------------------*/
ACMD(hair_style)
{
	int hair_style = 0;

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!*message || sscanf(message, "%12d", &hair_style) < 1) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_ENTER_HAIRSTYLE), MIN_HAIR_STYLE, MAX_HAIR_STYLE); // Please enter a hair style (usage: @hairstyle/@hstyle <hair ID: %d-%d>).
		clif->message(fd, atcmd_output);
		return false;
	}

	if (hair_style >= MIN_HAIR_STYLE && hair_style <= MAX_HAIR_STYLE) {
		pc->changelook(sd, LOOK_HAIR, hair_style);
		clif->message(fd, msg_fd(fd, MSGTBL_APPEARANCE_CHANGED)); // Appearance changed.
	} else {
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_NUMBER)); // An invalid number was specified.
		return false;
	}

	return true;
}

/*==========================================
 * @haircolor && @hcolor
 *------------------------------------------*/
ACMD(hair_color)
{
	int hair_color = 0;

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!*message || sscanf(message, "%12d", &hair_color) < 1) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_ENTER_HAIR_COLOR), MIN_HAIR_COLOR, MAX_HAIR_COLOR); // Please enter a hair color (usage: @haircolor/@hcolor <hair color: %d-%d>).
		clif->message(fd, atcmd_output);
		return false;
	}

	if (hair_color >= MIN_HAIR_COLOR && hair_color <= MAX_HAIR_COLOR) {
		pc->changelook(sd, LOOK_HAIR_COLOR, hair_color);
		clif->message(fd, msg_fd(fd, MSGTBL_APPEARANCE_CHANGED)); // Appearance changed.
	} else {
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_NUMBER)); // An invalid number was specified.
		return false;
	}

	return true;
}

ACMD(setzone)
{
	char zone_name[MAP_ZONE_MAPFLAG_LENGTH];
	memset(zone_name, '\0', sizeof(zone_name));

	char fmt_str[8] = "";
	snprintf(fmt_str, 8, "%%%ds", MAP_ZONE_MAPFLAG_LENGTH - 1);

	if (*message == '\0' || sscanf(message, fmt_str, zone_name) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_SETZONE_USAGE)); // usage info
		return false;
	}

	struct map_zone_data *zone = strdb_get(map->zone_db, zone_name);
	const char *prev_zone_name = map->list[sd->bl.m].zone->name;

	// handle special zones:
	if (zone == NULL && strcmp(zone_name, MAP_ZONE_NORMAL_NAME) == 0) {
		zone = &map->zone_all;
	} else if (zone == NULL && strcmp(zone_name, MAP_ZONE_PK_NAME) == 0) {
		zone = &map->zone_pk;
	}

	if (zone != NULL) {
		if (map->list[sd->bl.m].zone != zone) {
			if (strcmp(prev_zone_name, MAP_ZONE_PVP_NAME) == 0) {
				atcommand_pvpoff(fd, sd, command, message, info);
			} else if (strcmp(prev_zone_name, MAP_ZONE_GVG_NAME) == 0) {
				atcommand_gvgoff(fd, sd, command, message, info);
			} else if (strcmp(prev_zone_name, MAP_ZONE_CVC_NAME) == 0) {
				atcommand_cvcoff(fd, sd, command, message, info);
			}
		} else {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_SETZONE_ALREADY_SET), zone_name);
			clif->message(fd, atcmd_output); // nothing to do
			return false;
		}

		if (strcmp(zone_name, MAP_ZONE_PVP_NAME) == 0) {
			atcommand_pvpon(fd, sd, command, message, info);
		} else if (strcmp(zone_name, MAP_ZONE_GVG_NAME) == 0) {
			atcommand_gvgon(fd, sd, command, message, info);
		} else if (strcmp(zone_name, MAP_ZONE_CVC_NAME) == 0) {
			atcommand_cvcon(fd, sd, command, message, info);
		} else {
			map->zone_change2(sd->bl.m, zone);
		}
	} else {
		clif->message(fd, msg_fd(fd, MSGTBL_SETZONE_NOT_FOUND)); // zone not found
		return false;
	}

	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_SETZONE_SUCCESS_CHANGED), prev_zone_name, zone_name);
	clif->message(fd, atcmd_output); // changed successfully
	return true;
}

/*==========================================
 * @go [city_number or city_name] - Updated by Harbin
 *------------------------------------------*/
ACMD(go)
{
	int town = INT_MAX; // Initialized to INT_MAX instead of -1 to avoid conflicts with those who map [-3:-1] to @memo locations.
	char map_name[MAP_NAME_LENGTH];

	const struct {
		char map[MAP_NAME_LENGTH];
		int x, y;
		int min_match; ///< Minimum string length to match
	} data[] = {
		{ MAP_PRONTERA,    156, 191, 3 }, //  0 = Prontera
		{ MAP_MORROC,      156,  93, 4 }, //  1 = Morroc
		{ MAP_GEFFEN,      119,  59, 3 }, //  2 = Geffen
		{ MAP_PAYON,       162, 233, 3 }, //  3 = Payon
		{ MAP_ALBERTA,     192, 147, 3 }, //  4 = Alberta
#ifdef RENEWAL
		{ MAP_IZLUDE,      128, 146, 3 }, //  5 = Izlude (Renewal)
#else
		{ MAP_IZLUDE,      128, 114, 3 }, //  5 = Izlude
#endif
		{ MAP_ALDEBARAN,   140, 131, 3 }, //  6 = Aldebaran
		{ MAP_LUTIE,       147, 134, 3 }, //  7 = Lutie
		{ MAP_COMODO,      209, 143, 3 }, //  8 = Comodo
		{ MAP_YUNO,        157,  51, 3 }, //  9 = Juno
		{ MAP_AMATSU,      198,  84, 3 }, // 10 = Amatsu
		{ MAP_GONRYUN,     160, 120, 3 }, // 11 = Kunlun
		{ MAP_UMBALA,       89, 157, 3 }, // 12 = Umbala
		{ MAP_NIFLHEIM,     21, 153, 3 }, // 13 = Niflheim
		{ MAP_LOUYANG,     217,  40, 3 }, // 14 = Luoyang
		{ MAP_NOVICE,       53, 111, 3 }, // 15 = Training Grounds
		{ MAP_JAIL,         23,  61, 3 }, // 16 = Prison
		{ MAP_JAWAII,      249, 127, 3 }, // 17 = Jawaii
		{ MAP_AYOTHAYA,    151, 117, 3 }, // 18 = Ayothaya
		{ MAP_EINBROCH,     64, 200, 5 }, // 19 = Einbroch
		{ MAP_LIGHTHALZEN, 158,  92, 3 }, // 20 = Lighthalzen
		{ MAP_EINBECH,      70,  95, 5 }, // 21 = Einbech
		{ MAP_HUGEL,        96, 145, 3 }, // 22 = Hugel
		{ MAP_RACHEL,      130, 110, 3 }, // 23 = Rachel
		{ MAP_VEINS,       216, 123, 3 }, // 24 = Veins
		{ MAP_MOSCOVIA,    223, 184, 3 }, // 25 = Moscovia
		{ MAP_MIDCAMP,     180, 240, 3 }, // 26 = Midgard Camp
		{ MAP_MANUK,       282, 138, 3 }, // 27 = Manuk
		{ MAP_SPLENDIDE,   197, 176, 3 }, // 28 = Splendide
		{ MAP_BRASILIS,    182, 239, 3 }, // 29 = Brasilis
		{ MAP_DICASTES,    198, 187, 3 }, // 30 = El Dicastes
		{ MAP_MORA,         44, 151, 4 }, // 31 = Mora
		{ MAP_DEWATA,      200, 180, 3 }, // 32 = Dewata
		{ MAP_MALANGDO,    140, 114, 5 }, // 33 = Malangdo Island
		{ MAP_MALAYA,      242, 211, 5 }, // 34 = Malaya Port
		{ MAP_ECLAGE,      110,  39, 3 }, // 35 = Eclage
	};

	memset(map_name, '\0', sizeof(map_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!*message || sscanf(message, "%11s", map_name) < 1) {
		// no value matched so send the list of locations
		const char* text;

		// attempt to find the text help string
		text = atcommand_help_string( info );

		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_LOCATION_OR_NAME)); // Invalid location number, or name.

		if (text) { // send the text to the client
			clif->messageln( fd, text );
		}

		return false;
	}

	// Numeric entry
	if (ISDIGIT(*message) || (message[0] == '-' && ISDIGIT(message[1]))) {
		town = atoi(message);
	}

	if (town < 0 || town >= ARRAYLENGTH(data)) {
		int i;
		map_name[MAP_NAME_LENGTH-1] = '\0';

		// Match maps on the list
		for (i = 0; i < ARRAYLENGTH(data); i++) {
			if (strncmpi(map_name, data[i].map, data[i].min_match) == 0) {
				town = i;
				break;
			}
		}
	}

	if (town < 0 || town >= ARRAYLENGTH(data)) {
		// Alternate spellings
		if (strncmpi(map_name, "morroc", 4) == 0) { // Correct town name for 'morocc'
			town = 1;
		} else if (strncmpi(map_name, "lutie", 3) == 0) { // Correct town name for 'xmas'
			town = 7;
		} else if (strncmpi(map_name, "juno", 3) == 0) { // Correct town name for 'yuno'
			town = 9;
		} else if (strncmpi(map_name, "kunlun", 3) == 0) { // Original town name for 'gonryun'
			town = 11;
		} else if (strncmpi(map_name, "luoyang", 3) == 0) { // Original town name for 'louyang'
			town = 14;
		} else if (strncmpi(map_name, "startpoint", 3) == 0 // Easy to remember alternatives to 'new_1-1'
		        || strncmpi(map_name, "beginning", 3) == 0) {
			town = 15;
		} else if (strncmpi(map_name, "prison", 3) == 0 // Easy to remember alternatives to 'sec_pri'
		        || strncmpi(map_name, "jail", 3) == 0) {
			town = 16;
		} else if (strncmpi(map_name, "rael", 3) == 0) { // Original town name for 'rachel'
			town = 23;
		}
	}

	if (town >= 0 && town < ARRAYLENGTH(data)) {
		int16 m = map->mapname2mapid(data[town].map);
		if (m >= 0 && map->list[m].flag.nowarpto && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
			clif->message(fd, msg_fd(fd, MSGTBL_CANT_WARP_TO));
			return false;
		}
		if (sd->bl.m >= 0 && map->list[sd->bl.m].flag.nowarp && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
			clif->message(fd, msg_fd(fd, MSGTBL_CANT_WARP_FROM));
			return false;
		}
		if (pc->setpos(sd, mapindex->name2id(data[town].map), data[town].x, data[town].y, CLR_TELEPORT) == 0) {
			clif->message(fd, msg_fd(fd, MSGTBL_WARPED)); // Warped.
		} else {
			clif->message(fd, msg_fd(fd, MSGTBL_MAP_NOT_FOUND)); // Map not found.
			return false;
		}
	} else {
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_LOCATION_OR_NAME)); // Invalid location number or name.
		return false;
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(monster)
{
	char name[NAME_LENGTH];
	char monster[NAME_LENGTH];
	char eventname[EVENT_NAME_LENGTH] = "";
	int mob_id;
	int number = 0;
	int count;
	int i, range;
	short mx, my;
	unsigned int size;

	memset(name, '\0', sizeof(name));
	memset(monster, '\0', sizeof(monster));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!*message) {
		clif->message(fd, msg_fd(fd, MSGTBL_SPECIFY_NAME_OR_ID)); // Please specify a display name or monster name/id.
		return false;
	}
	if (sscanf(message, "\"%23[^\"]\" %23s %12d", name, monster, &number) > 1 ||
		sscanf(message, "%23s \"%23[^\"]\" %12d", monster, name, &number) > 1) {
		//All data can be left as it is.
	} else if ((count=sscanf(message, "%23s %12d %23s", monster, &number, name)) > 1) {
		//Here, it is possible name was not given and we are using monster for it.
		if (count < 3) //Blank mob's name.
			name[0] = '\0';
	} else if (sscanf(message, "%23s %23s %12d", name, monster, &number) > 1) {
		//All data can be left as it is.
	} else if (sscanf(message, "%23s", monster) > 0) {
		//As before, name may be already filled.
		name[0] = '\0';
	} else {
		clif->message(fd, msg_fd(fd, MSGTBL_SPECIFY_NAME_OR_ID)); // Give a display name and monster name/id please.
		return false;
	}

	if ((mob_id = mob->db_searchname(monster)) == 0) // check name first (to avoid possible name beginning by a number)
		mob_id = mob->db_checkid(atoi(monster));

	if (mob_id == 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_MONSTER_ID_NAME)); // Invalid monster ID or name.
		return false;
	}

	if (number <= 0)
		number = 1;

	if (!name[0])
		strcpy(name, DEFAULT_MOB_JNAME);

	// If value of atcommand_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
	if (battle_config.atc_spawn_quantity_limit && number > battle_config.atc_spawn_quantity_limit)
		number = battle_config.atc_spawn_quantity_limit;

	if (strcmpi(info->command, "monstersmall") == 0)
		size = SZ_MEDIUM;
	else if (strcmpi(info->command, "monsterbig") == 0)
		size = SZ_BIG;
	else
		size = SZ_SMALL;

	if (battle_config.etc_log)
		ShowInfo("%s monster='%s' name='%s' id=%d count=%d (%d,%d)\n", command, monster, name, mob_id, number, sd->bl.x, sd->bl.y);

	count = 0;
	range = (int)sqrt((float)number) +2; // calculation of an odd number (+ 4 area around)
	for (i = 0; i < number; i++) {
		int k;
		map->search_free_cell(&sd->bl, 0, &mx,  &my, range, range, SFC_DEFAULT);
		k = mob->once_spawn(sd, sd->bl.m, mx, my, name, mob_id, 1, eventname, size, AI_NONE|(mob_id == MOBID_EMPELIUM?0x200:0x0));
		count += (k != 0) ? 1 : 0;
	}

	if (count != 0)
		if (number == count)
			clif->message(fd, msg_fd(fd, MSGTBL_MONSTERS_SUMMONED)); // All monster summoned!
		else {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_N_MONSTERS_SUMMONED), count); // %d monster(s) summoned!
			clif->message(fd, atcmd_output);
		}
		else {
			clif->message(fd, msg_fd(fd, MSGTBL_INVALID_MONSTER_ID_NAME)); // Invalid monster ID or name.
			return false;
		}

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
static int atkillmonster_sub(struct block_list *bl, va_list ap)
{
	struct mob_data *md = NULL;
	int flag = va_arg(ap, int);
	nullpo_ret(bl);
	Assert_ret(bl->type == BL_MOB);
	md = BL_UCAST(BL_MOB, bl);

	if (md->guardian_data)
		return 0; //Do not touch WoE mobs!

	if (flag)
		status_zap(bl,md->status.hp, 0);
	else
		status_kill(bl);
	return 1;
}

ACMD(killmonster)
{
	int map_id, drop_flag;
	char map_name[MAP_NAME_LENGTH_EXT];

	memset(map_name, '\0', sizeof(map_name));

	if (!*message || sscanf(message, "%15s", map_name) < 1) {
		map_id = sd->bl.m;
	} else {
		if ((map_id = map->mapname2mapid(map_name)) < 0)
			map_id = sd->bl.m;
	}

	drop_flag = strcmpi(info->command, "killmonster2");

	map->foreachinmap(atcommand->atkillmonster_sub, map_id, BL_MOB, -drop_flag);

	clif->message(fd, msg_fd(fd, MSGTBL_ALL_MONSTERS_KILLED)); // All monsters killed!

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(refine)
{
	int j, position = 0, refine_level = 0, current_position, final_refine;
	int count;

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!*message || sscanf(message, "%12d %12d", &position, &refine_level) < 2) {
		clif->message(fd, msg_fd(fd, MSGTBL_REFINE_ENTER_POS_AMOUNT)); // Please enter a position and an amount (usage: @refine <equip position> <+/- amount>).
#if PACKETVER > 20100707
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_ALL_EQP_SHADOW), -3); // %d: Refine All Equip (Shadow)
		clif->message(fd, atcmd_output);
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_ALL_EQP_COSTUME), -2); // %d: Refine All Equip (Costume)
		clif->message(fd, atcmd_output);
#endif
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_ALL_EQP_GENERAL), -1); // %d: Refine All Equip (General)
		clif->message(fd, atcmd_output);
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_HEAD_LOW), EQP_HEAD_LOW); // %d: Headgear (Low)
		clif->message(fd, atcmd_output);
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_HAND_RIGHT), EQP_HAND_R); // Hand (Right)
		clif->message(fd, atcmd_output);
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_GARMENT), EQP_GARMENT); // %d: Garment
		clif->message(fd, atcmd_output);
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_ACC_LEFT), EQP_ACC_L); // Accessory (Left)
		clif->message(fd, atcmd_output);
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_BODY_ARMOR), EQP_ARMOR); // %d: Body Armor
		clif->message(fd, atcmd_output);
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_HAND_LEFT), EQP_HAND_L); // Hand (Left)
		clif->message(fd, atcmd_output);
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_SHOES), EQP_SHOES); // %d: Shoes
		clif->message(fd, atcmd_output);
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_ACC_RIGHT), EQP_ACC_R); // Accessory (Right)
		clif->message(fd, atcmd_output);
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_HEAD_TOP), EQP_HEAD_TOP); // %d: Headgear (Top)
		clif->message(fd, atcmd_output);
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_HEAD_MID), EQP_HEAD_MID); // %d: Headgear (Mid)
#if PACKETVER > 20100707
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_COSTUME_HEAD_TOP), EQP_COSTUME_HEAD_TOP); // %d: Costume Headgear (Top)
		clif->message(fd, atcmd_output);
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_COSTUME_HEAD_MID), EQP_COSTUME_HEAD_MID); // %d: Costume Headgear (Mid)
		clif->message(fd, atcmd_output);
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_COSTUME_HEAD_LOW), EQP_COSTUME_HEAD_LOW); // %d: Costume Headgear (Low)
		clif->message(fd, atcmd_output);
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_COSTUME_GARMENT), EQP_COSTUME_GARMENT); // %d: Costume Garment
		clif->message(fd, atcmd_output);
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_SHADOW_ARMOR), EQP_SHADOW_ARMOR); // %d: Shadow Armor
		clif->message(fd, atcmd_output);
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_SHADOW_WEAPON), EQP_SHADOW_WEAPON); // %d: Shadow Weapon
		clif->message(fd, atcmd_output);
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_SHADOW_SHIELD), EQP_SHADOW_SHIELD); // %d: Shadow Shield
		clif->message(fd, atcmd_output);
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_SHADOW_SHOES), EQP_SHADOW_SHOES); // %d: Shadow Shoes
		clif->message(fd, atcmd_output);
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_SHADOW_ACC_RIGHT), EQP_SHADOW_ACC_R); // %d: Shadow Accessory (Right)
		clif->message(fd, atcmd_output);
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_SHADOW_ACC_LEFT), EQP_SHADOW_ACC_L); // %d: Shadow Accessory (Left)
		clif->message(fd, atcmd_output);
#endif
		clif->message(fd, atcmd_output);
		return false;
	}

	refine_level = cap_value(refine_level, -MAX_REFINE, MAX_REFINE);

	count = 0;
	for (j = 0; j < EQI_MAX; j++) {
		int idx = sd->equip_index[j];
		if (idx < 0)
			continue;
		if (j == EQI_AMMO)
			continue; /* can't equip ammo */
		if (j == EQI_HAND_R && sd->equip_index[EQI_HAND_L] == idx)
			continue;
		if (j == EQI_HEAD_MID && sd->equip_index[EQI_HEAD_LOW] == idx)
			continue;
		if (j == EQI_HEAD_TOP && (sd->equip_index[EQI_HEAD_MID] == idx || sd->equip_index[EQI_HEAD_LOW] == idx))
			continue;
		if (j == EQI_COSTUME_MID && sd->equip_index[EQI_COSTUME_LOW] == idx)
			continue;
		if (j == EQI_COSTUME_TOP && (sd->equip_index[EQI_COSTUME_MID] == idx || sd->equip_index[EQI_COSTUME_LOW] == idx))
			continue;

		if (position == -3 && !itemdb_is_shadowequip(sd->status.inventory[idx].equip))
			continue;
		else if (position == -2 && !itemdb_is_costumeequip(sd->status.inventory[idx].equip))
			continue;
		else if (position == -1 && (itemdb_is_costumeequip(sd->status.inventory[idx].equip) || itemdb_is_shadowequip(sd->status.inventory[idx].equip)))
			continue;
		else if (position && !(sd->status.inventory[idx].equip & position))
			continue;

		final_refine = cap_value(sd->status.inventory[idx].refine + refine_level, 0, MAX_REFINE);
		if (sd->status.inventory[idx].refine != final_refine) {
			sd->status.inventory[idx].refine = final_refine;
			current_position = sd->status.inventory[idx].equip;
			pc->unequipitem(sd, idx, PCUNEQUIPITEM_RECALC | PCUNEQUIPITEM_FORCE);
			clif->refine(fd, 0, idx, sd->status.inventory[idx].refine);
			clif->delitem(sd, idx, 1, DELITEM_MATERIALCHANGE);
			clif->additem(sd, idx, 1, 0);
			pc->equipitem(sd, idx, current_position);
			clif->misceffect(&sd->bl, 3);
			count++;
		}
	}

	if (count == 0)
		clif->message(fd, msg_fd(fd, MSGTBL_REFINE_NOTHING)); // No item has been refined.
	else if (count == 1)
		clif->message(fd, msg_fd(fd, MSGTBL_REFINE_1_ITEM_REFINED)); // 1 item has been refined.
	else {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_N_iTEMS_REFINED), count); // %d items have been refined.
		clif->message(fd, atcmd_output);
	}

	return true;
}

ACMD(grade)
{
	const struct {
		int pos_id;
		enum msgtable_messages msg_id;
	} messages_list[] = {
		{ -3,                   MSGTBL_GRADE_ALL_EQUIP_SHADOW  }, // %d: Grade All Equip (Shadow)
		{ -2,                   MSGTBL_GRADE_ALL_EQUIP_COSTUME }, // %d: Grade All Equip (Costume)
		{ -1,                   MSGTBL_GRADE_ALL_EQUIP_GENERAL }, // %d: Grade All Equip (General)
		{ EQP_HEAD_LOW,         MSGTBL_REFINE_HEAD_LOW         }, // %d: Headgear (Low)
		{ EQP_HAND_R,           MSGTBL_REFINE_HAND_RIGHT       }, // %d: Hand (Right)
		{ EQP_GARMENT,          MSGTBL_REFINE_GARMENT          }, // %d: Garment
		{ EQP_ACC_L,            MSGTBL_REFINE_ACC_LEFT         }, // %d: Accessory (Left)
		{ EQP_ARMOR,            MSGTBL_REFINE_BODY_ARMOR       }, // %d: Body Armor
		{ EQP_HAND_L,           MSGTBL_REFINE_HAND_LEFT        }, // %d: Hand (Left)
		{ EQP_SHOES,            MSGTBL_REFINE_SHOES            }, // %d: Shoes
		{ EQP_ACC_R,            MSGTBL_REFINE_ACC_RIGHT        }, // %d: Accessory (Right)
		{ EQP_HEAD_TOP,         MSGTBL_REFINE_HEAD_TOP         }, // %d: Headgear (Top)
		{ EQP_HEAD_MID,         MSGTBL_REFINE_HEAD_MID         }, // %d: Headgear (Mid)
		{ EQP_COSTUME_HEAD_TOP, MSGTBL_REFINE_COSTUME_HEAD_TOP }, // %d: Costume Headgear (Top)
		{ EQP_COSTUME_HEAD_MID, MSGTBL_REFINE_COSTUME_HEAD_MID }, // %d: Costume Headgear (Mid)
		{ EQP_COSTUME_HEAD_LOW, MSGTBL_REFINE_COSTUME_HEAD_LOW }, // %d: Costume Headgear (Low)
		{ EQP_COSTUME_GARMENT,  MSGTBL_REFINE_COSTUME_GARMENT  }, // %d: Costume Garment
		{ EQP_SHADOW_ARMOR,     MSGTBL_REFINE_SHADOW_ARMOR     }, // %d: Shadow Armor
		{ EQP_SHADOW_WEAPON,    MSGTBL_REFINE_SHADOW_WEAPON    }, // %d: Shadow Weapon
		{ EQP_SHADOW_SHIELD,    MSGTBL_REFINE_SHADOW_SHIELD    }, // %d: Shadow Shield
		{ EQP_SHADOW_SHOES,     MSGTBL_REFINE_SHADOW_SHOES     }, // %d: Shadow Shoes
		{ EQP_SHADOW_ACC_R,     MSGTBL_REFINE_SHADOW_ACC_RIGHT }, // %d: Shadow Accessory (Right)
		{ EQP_SHADOW_ACC_L,     MSGTBL_REFINE_SHADOW_ACC_LEFT  }, // %d: Shadow Accessory (Left)
	};

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	int position = 0;
	int grade_level = 0;
	if (!*message || sscanf(message, "%12d %12d", &position, &grade_level) < 2) {
		clif->message(fd, msg_fd(fd, MSGTBL_GRADE_USAGE)); // Please enter a position and an amount (usage: @grade <equip position> <+/- amount>).
		for (int i = 0; i < ARRAYLENGTH(messages_list); ++i) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, messages_list[i].msg_id), messages_list[i].pos_id);
			clif->message(fd, atcmd_output);
		}
		return false;
	}

	grade_level = cap_value(grade_level, 0, MAX_ITEM_GRADE);

	int count = 0;
	for (int j = 0; j < EQI_MAX; j++) {
		int idx = sd->equip_index[j];
		if (idx < 0)
			continue;
		if (j == EQI_AMMO)
			continue; /* can't equip ammo */
		if (j == EQI_HAND_R && sd->equip_index[EQI_HAND_L] == idx)
			continue;
		if (j == EQI_HEAD_MID && sd->equip_index[EQI_HEAD_LOW] == idx)
			continue;
		if (j == EQI_HEAD_TOP && (sd->equip_index[EQI_HEAD_MID] == idx || sd->equip_index[EQI_HEAD_LOW] == idx))
			continue;
		if (j == EQI_COSTUME_MID && sd->equip_index[EQI_COSTUME_LOW] == idx)
			continue;
		if (j == EQI_COSTUME_TOP && (sd->equip_index[EQI_COSTUME_MID] == idx || sd->equip_index[EQI_COSTUME_LOW] == idx))
			continue;

		if (position == -3 && !itemdb_is_shadowequip(sd->status.inventory[idx].equip))
			continue;
		else if (position == -2 && !itemdb_is_costumeequip(sd->status.inventory[idx].equip))
			continue;
		else if (position == -1 && (itemdb_is_costumeequip(sd->status.inventory[idx].equip) || itemdb_is_shadowequip(sd->status.inventory[idx].equip)))
			continue;
		else if (position && !(sd->status.inventory[idx].equip & position))
			continue;

		int final_grade = cap_value(sd->status.inventory[idx].grade + grade_level, 0, MAX_ITEM_GRADE);
		if (sd->status.inventory[idx].grade != final_grade) {
			sd->status.inventory[idx].grade = final_grade;
			const int current_position = sd->status.inventory[idx].equip;
			pc->unequipitem(sd, idx, PCUNEQUIPITEM_RECALC | PCUNEQUIPITEM_FORCE);
			clif->delitem(sd, idx, 1, DELITEM_MATERIALCHANGE);
			clif->additem(sd, idx, 1, 0);
			pc->equipitem(sd, idx, current_position);
			clif->misceffect(&sd->bl, 3);
			count++;
		}
	}

	if (count == 0)
		clif->message(fd, msg_fd(fd, MSGTBL_NO_ITEM_GRADED)); // No item has been graded.
	else if (count == 1)
		clif->message(fd, msg_fd(fd, MSGTBL_ONE_ITEM_GRADED)); // 1 item has been graded.
	else {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REFINE_N_iTEMS_REFINED), 1530); // %d items have been graded.
		clif->message(fd, atcmd_output);
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(produce)
{
	char item_name[100];
	int item_id, attribute = 0, star = 0;
	struct item_data *item_data;
	struct item tmp_item;

	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(item_name, '\0', sizeof(item_name));

	if (!*message || (
								  sscanf(message, "\"%99[^\"]\" %12d %12d", item_name, &attribute, &star) < 1 &&
								  sscanf(message, "%99s %12d %12d", item_name, &attribute, &star) < 1
								  )) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_PRODUCE_ITEM)); // Please enter at least one item name/ID (usage: @produce <equip name/ID> <element> <# of very's>).
		return false;
	}

	if ( (item_data = itemdb->search_name(item_name)) == NULL &&
		(item_data = itemdb->exists(atoi(item_name))) == NULL ) {
		clif->message(fd, msg_fd(fd, MSGTBL_PRODUCE_NOT_EQUIPPABLE)); //This item is not an equipment.
		return false;
	}

	item_id = item_data->nameid;

	if (itemdb->isequip2(item_data)) {
		int flag = 0;
		if (attribute < MIN_ATTRIBUTE || attribute > MAX_ATTRIBUTE)
			attribute = ATTRIBUTE_NORMAL;
		if (star < MIN_STAR || star > MAX_STAR)
			star = 0;
		memset(&tmp_item, 0, sizeof tmp_item);
		tmp_item.nameid = item_id;
		tmp_item.amount = 1;
		tmp_item.identify = 1;
		tmp_item.card[0] = CARD0_FORGE;
		tmp_item.card[1] = item_data->type==IT_WEAPON?
		((star*5) << 8) + attribute:0;
		tmp_item.card[2] = GetWord(sd->status.char_id, 0);
		tmp_item.card[3] = GetWord(sd->status.char_id, 1);
		clif->produce_effect(sd, 0, item_id);
		clif->misceffect(&sd->bl, 3);

		if ((flag = pc->additem(sd, &tmp_item, 1, LOG_TYPE_COMMAND)))
			clif->additem(sd, 0, 0, flag);
	} else {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_PRODUCE_NOT_EQUIPPABLE_NAME), item_id, item_data->name); // The item (%d: '%s') is not equipable.
		clif->message(fd, atcmd_output);
		return false;
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(memo)
{
	int position = 0;

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!*message || sscanf(message, "%d", &position) < 1)
	{
		int i;
		clif->message(sd->fd,  msg_fd(fd, MSGTBL_CURRENT_MEMO_POSITION)); // "Your current memo positions are:"
		for( i = 0; i < MAX_MEMOPOINTS; i++ )
		{
			if( sd->status.memo_point[i].map )
				snprintf(atcmd_output, sizeof(atcmd_output), "%d - %s (%d,%d)", i, mapindex_id2name(sd->status.memo_point[i].map), sd->status.memo_point[i].x, sd->status.memo_point[i].y);
			else
				snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_MEMO_VOID), i); // %d - void
			clif->message(sd->fd, atcmd_output);
		}
		return true;
	}

	if( position < 0 || position >= MAX_MEMOPOINTS )
	{
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_ENTER_MEMO_POSITION), 0, MAX_MEMOPOINTS-1); // Please enter a valid position (usage: @memo <memo_position:%d-%d>).
		clif->message(fd, atcmd_output);
		return false;
	}

	pc->memo(sd, position);
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(gat)
{
	int y;

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	for (y = 2; y >= -2; y--) {
		snprintf(atcmd_output, sizeof(atcmd_output), "%s (x= %d, y= %d) %02X %02X %02X %02X %02X",
				map->list[sd->bl.m].name, sd->bl.x - 2, sd->bl.y + y,
				(unsigned int)map->getcell(sd->bl.m, &sd->bl, sd->bl.x - 2, sd->bl.y + y, CELL_GETTYPE),
				(unsigned int)map->getcell(sd->bl.m, &sd->bl, sd->bl.x - 1, sd->bl.y + y, CELL_GETTYPE),
				(unsigned int)map->getcell(sd->bl.m, &sd->bl, sd->bl.x,     sd->bl.y + y, CELL_GETTYPE),
				(unsigned int)map->getcell(sd->bl.m, &sd->bl, sd->bl.x + 1, sd->bl.y + y, CELL_GETTYPE),
				(unsigned int)map->getcell(sd->bl.m, &sd->bl, sd->bl.x + 2, sd->bl.y + y, CELL_GETTYPE));

		clif->message(fd, atcmd_output);
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(displaystatus)
{
	int i, type, flag, tick, val1 = 0, val2 = 0, val3 = 0;

	if (!*message || (i = sscanf(message, "%d %d %d %d %d %d", &type, &flag, &tick, &val1, &val2, &val3)) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_DISPLAY_STATUS)); // Please enter a status type/flag (usage: @displaystatus <status type> <flag> <tick> {<val1> {<val2> {<val3>}}}).
		return false;
	}
	if (i < 2) flag = 1;
	if (i < 3) tick = 0;

	if( flag == 0 )
		clif->sc_end(&sd->bl,sd->bl.id,AREA,type);
	else
		clif->status_change(&sd->bl, type, BL_PC, flag, tick, val1, val2, val3);

	return true;
}

/*==========================================
 * @stpoint (Rewritten by [Yor])
 *------------------------------------------*/
ACMD(statuspoint)
{
	int point;
	int new_status_point;

	if (!*message || (point = atoi(message)) == 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_STAT_POINT)); // Please enter a number (usage: @stpoint <number of points>).
		return false;
	}

	if (point < 0 && sd->status.status_point + point < 0) {
		new_status_point = 0;
	} else if (point > 0 && (int64)sd->status.status_point + point > INT_MAX) {
		new_status_point = INT_MAX;
	} else {
		new_status_point = sd->status.status_point + point;
	}

	if (new_status_point != sd->status.status_point) {
		sd->status.status_point = new_status_point;
		clif->updatestatus(sd, SP_STATUSPOINT);
		clif->message(fd, msg_fd(fd, MSGTBL_STATS_POINTS_CHANGED)); // Number of status points changed.
	} else {
		if (point < 0)
			clif->message(fd, msg_fd(fd, MSGTBL_UNABLE_TO_DECREASE_VALUE)); // Unable to decrease the number/value.
		else
			clif->message(fd, msg_fd(fd, MSGTBL_IMPOSSIBLE_TO_INCREASE_VALUE)); // Unable to increase the number/value.
		return false;
	}

	return true;
}

/*==========================================
 * @skpoint (Rewritten by [Yor])
 *------------------------------------------*/
ACMD(skillpoint)
{
	int point;
	int new_skill_point;

	if (!*message || (point = atoi(message)) == 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_SKILL_POINT)); // Please enter a number (usage: @skpoint <number of points>).
		return false;
	}

	if (point < 0 && sd->status.skill_point + point < 0) {
		new_skill_point = 0;
	} else if (point > 0 && (int64)sd->status.skill_point + point > INT_MAX) {
		new_skill_point = INT_MAX;
	} else {
		new_skill_point = sd->status.skill_point + point;
	}

	if (new_skill_point != sd->status.skill_point) {
		sd->status.skill_point = new_skill_point;
		clif->updatestatus(sd, SP_SKILLPOINT);
		clif->message(fd, msg_fd(fd, MSGTBL_SKILL_POINTS_CHANGED)); // Number of skill points changed.
	} else {
		if (point < 0)
			clif->message(fd, msg_fd(fd, MSGTBL_UNABLE_TO_DECREASE_VALUE)); // Unable to decrease the number/value.
		else
			clif->message(fd, msg_fd(fd, MSGTBL_IMPOSSIBLE_TO_INCREASE_VALUE)); // Unable to increase the number/value.
		return false;
	}

	return true;
}

/*==========================================
 * @zeny
 *------------------------------------------*/
ACMD(zeny)
{
	int zeny=0, ret=-1;

	if (!*message || (zeny = atoi(message)) == 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_ZENY_AMOUNT)); // Please enter an amount (usage: @zeny <amount>).
		return false;
	}

	if(zeny > 0) {
	    if((ret=pc->getzeny(sd,zeny,LOG_TYPE_COMMAND,NULL)) == 1)
			clif->message(fd, msg_fd(fd, MSGTBL_IMPOSSIBLE_TO_INCREASE_VALUE)); // Unable to increase the number/value.
	}
	else {
	    if( sd->status.zeny < -zeny ) zeny = -sd->status.zeny;
	    if((ret=pc->payzeny(sd,-zeny,LOG_TYPE_COMMAND,NULL)) == 1)
			clif->message(fd, msg_fd(fd, MSGTBL_UNABLE_TO_DECREASE_VALUE)); // Unable to decrease the number/value.
	}

	if( ret ) //ret != 0 means cmd failure
		return false;

	clif->message(fd, msg_fd(fd, MSGTBL_ZENY_CHANGED));
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(param)
{
	int i, value = 0, new_value, max;
	const char* param[] = { "str", "agi", "vit", "int", "dex", "luk" };
	short* stats[6];
	//we don't use direct initialization because it isn't part of the c standard.

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!*message || sscanf(message, "%d", &value) < 1 || value == 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_PARAM_ADJUSTMENT)); // Please enter a valid value (usage: @str/@agi/@vit/@int/@dex/@luk <+/-adjustment>).
		return false;
	}

	ARR_FIND( 0, ARRAYLENGTH(param), i, strcmpi(info->command, param[i]) == 0 );

	if( i == ARRAYLENGTH(param) || i > MAX_STATUS_TYPE) { // normally impossible...
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_PARAM_ADJUSTMENT)); // Please enter a valid value (usage: @str/@agi/@vit/@int/@dex/@luk <+/-adjustment>).
		return false;
	}

	stats[0] = &sd->status.str;
	stats[1] = &sd->status.agi;
	stats[2] = &sd->status.vit;
	stats[3] = &sd->status.int_;
	stats[4] = &sd->status.dex;
	stats[5] = &sd->status.luk;

	if( battle_config.atcommand_max_stat_bypass )
		max = SHRT_MAX;
	else
		max = pc_maxstats(sd);

	if(value < 0 && *stats[i] <= -value) {
		new_value = 1;
	} else if(max - *stats[i] < value) {
		new_value = max;
	} else {
		new_value = *stats[i] + value;
	}

	if (new_value != *stats[i]) {
		*stats[i] = new_value;
		clif->updatestatus(sd, SP_STR + i);
		clif->updatestatus(sd, SP_USTR + i);
		status_calc_pc(sd, SCO_FORCE);
		clif->message(fd, msg_fd(fd, MSGTBL_STAT_CHANGED)); // Stat changed.
		achievement->validate_stats(sd, SP_STR + i, new_value); // Achievements [Smokexyz/Hercules]
	} else {
		if (value < 0)
			clif->message(fd, msg_fd(fd, MSGTBL_UNABLE_TO_DECREASE_VALUE)); // Unable to decrease the number/value.
		else
			clif->message(fd, msg_fd(fd, MSGTBL_IMPOSSIBLE_TO_INCREASE_VALUE)); // Unable to increase the number/value.
		return false;
	}

	return true;
}

/*==========================================
 * Stat all by fritz (rewritten by [Yor])
 *------------------------------------------*/
ACMD(stat_all)
{
	int index, count, value, max, new_value;
	short* stats[6];
	//we don't use direct initialization because it isn't part of the c standard.

	stats[0] = &sd->status.str;
	stats[1] = &sd->status.agi;
	stats[2] = &sd->status.vit;
	stats[3] = &sd->status.int_;
	stats[4] = &sd->status.dex;
	stats[5] = &sd->status.luk;

	if (!*message || sscanf(message, "%d", &value) < 1 || value == 0) {
		value = pc_maxstats(sd);
		max = pc_maxstats(sd);
	} else {
		if( battle_config.atcommand_max_stat_bypass )
			max = SHRT_MAX;
		else
			max = pc_maxstats(sd);
	}

	count = 0;
	for (index = 0; index < ARRAYLENGTH(stats); index++) {
		if (value > 0 && *stats[index] > max - value)
			new_value = max;
		else if (value < 0 && *stats[index] <= -value)
			new_value = 1;
		else
			new_value = *stats[index] +value;

		if (new_value != (int)*stats[index]) {
			*stats[index] = new_value;
			clif->updatestatus(sd, SP_STR + index);
			clif->updatestatus(sd, SP_USTR + index);
			count++;
		}
	}

	if (count > 0) { // if at least 1 stat modified
		status_calc_pc(sd, SCO_FORCE);
		clif->message(fd, msg_fd(fd, MSGTBL_ALL_STATS_CHANGED)); // All stats changed!
	} else {
		if (value < 0)
			clif->message(fd, msg_fd(fd, MSGTBL_STATS_CANT_DECREASE)); // You cannot decrease that stat anymore.
		else
			clif->message(fd, msg_fd(fd, MSGTBL_STATS_CANT_INCREASE)); // You cannot increase that stat anymore.
		return false;
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(guildlevelup)
{
	int level = 0;
	int16 added_level;
	struct guild *guild_info;

	if (!*message || sscanf(message, "%d", &level) < 1 || level == 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_GUILD_LEVEL)); // Please enter a valid level (usage: @guildlvup/@guildlvlup <# of levels>).
		return false;
	}

	if (sd->status.guild_id <= 0 || (guild_info = sd->guild) == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_NOT_IN_A_GUILD)); // You're not in a guild.
		return false;
	}
#if 0 // By enabling this, only the guild leader can use this command
	if (strcmp(sd->status.name, guild_info->master) != 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_NOT_GUILDMASTER)); // You're not the master of your guild.
		return false;
	}
#endif // 0

	if (level > INT16_MAX || (level > 0 && level > MAX_GUILDLEVEL - guild_info->guild_lv)) // fix positive overflow
		level = MAX_GUILDLEVEL - guild_info->guild_lv;
	else if (level < INT16_MIN || (level < 0 && level < 1 - guild_info->guild_lv)) // fix negative overflow
		level = 1 - guild_info->guild_lv;
	added_level = (int16)level;

	if (added_level != 0) {
		intif->guild_change_basicinfo(guild_info->guild_id, GBI_GUILDLV, &added_level, sizeof(added_level));
		clif->message(fd, msg_fd(fd, MSGTBL_GUILD_LEVEL_CHANGED)); // Guild level changed.
	} else {
		clif->message(fd, msg_fd(fd, MSGTBL_GUILD_LEVEL_CHANGE_FAILED)); // Guild level change failed.
		return false;
	}

	return true;
}

/**
 * Creates a pet egg in the character's inventory.
 *
 * @code{.herc}
 *	@makeegg <pet>
 * @endcode
 *
 **/
ACMD(makeegg)
{
	if (*message == '\0') {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_MAKE_EGG)); // Please enter a monster/egg name/ID (usage: @makeegg <pet>).
		return false;
	}

	struct item_data *item_data = itemdb->search_name(message);
	int id;

	if (item_data != NULL) { // Egg name.
		id = item_data->nameid;
	} else {
		id = mob->db_searchname(message); // Monster name.

		if (id == 0)
			id = atoi(message); // Egg/monster ID.
	}

	int pet_id = pet->search_petDB_index(id, PET_CLASS);

	if (pet_id == INDEX_NOT_FOUND)
		pet_id = pet->search_petDB_index(id, PET_EGG);

	if (pet_id == INDEX_NOT_FOUND) {
		clif->message(fd, msg_fd(fd, MSGTBL_MAKEEGG_INVALID_EGG)); // The monster/egg name/ID doesn't exist.
		return false;
	}

	sd->catch_target_class = pet->db[pet_id].class_;
	intif->create_pet(sd->status.account_id, sd->status.char_id, pet->db[pet_id].class_,
			  mob->db(pet->db[pet_id].class_)->lv, pet->db[pet_id].EggID, 0,
			  (short)pet->db[pet_id].intimate, PET_HUNGER_STUFFED,
			  0, 1,pet->db[pet_id].jname);

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(hatch)
{
	if (sd->status.pet_id <= 0)
		clif->sendegg(sd);
	else {
		clif->message(fd, msg_fd(fd, MSGTBL_ALREADY_HAVE_PET)); // You already have a pet.
		return false;
	}

	return true;
}

/**
 * Sets a pet's intimacy value.
 *
 * @code{.herc}
 *	@petfriendly <0-1000>
 * @endcode
 *
 **/
ACMD(petfriendly)
{
	if (*message == '\0' || (atoi(message) == 0 && isdigit(*message) == 0)) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_PET_FRIENDLY_VALUE)); // Please enter a valid value (usage: @petfriendly <0-1000>).
		return false;
	}

	int friendly = atoi(message);

	if (friendly < PET_INTIMACY_NONE || friendly > PET_INTIMACY_MAX) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_PET_FRIENDLY_VALUE)); // Please enter a valid value (usage: @petfriendly <0-1000>).
		return false;
	}

	struct pet_data *pd = sd->pd;

	if (sd->status.pet_id == 0 || pd == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_NO_PET)); // Sorry, but you have no pet.
		return false;
	}

	if (friendly == pd->pet.intimate && friendly == PET_INTIMACY_MAX) {
		clif->message(fd, msg_fd(fd, MSGTBL_PET_INTIMACY_MAXED)); // Pet intimacy is already at maximum.
		return false;
	}

	if (friendly != pd->pet.intimate) // No need to update the pet's status if intimacy value won't change.
		pet->set_intimate(pd, friendly);

	clif->message(fd, msg_fd(fd, MSGTBL_PET_INTIMACY_CHANGED)); // Pet intimacy changed. (Send message regardless of value has changed or not.)

	return true;
}

/**
 * Sets a pet's hunger value.
 *
 * @code{.herc}
 *	@pethungry <0-100>
 * @endcode
 *
 **/
ACMD(pethungry)
{
	if (*message == '\0' || (atoi(message) == 0 && isdigit(*message) == 0)) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_PET_HUNGRY_VALUE)); // Please enter a valid number (usage: @pethungry <0-100>).
		return false;
	}

	int hungry = atoi(message);

	if (hungry < PET_HUNGER_STARVING || hungry > PET_HUNGER_STUFFED) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_PET_HUNGRY_VALUE)); // Please enter a valid number (usage: @pethungry <0-100>).
		return false;
	}

	struct pet_data *pd = sd->pd;

	if (sd->status.pet_id == 0 || pd == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_NO_PET)); // Sorry, but you have no pet.
		return false;
	}

	if (hungry == pd->pet.hungry && hungry == PET_HUNGER_STUFFED) {
		clif->message(fd, msg_fd(fd, MSGTBL_PET_HUNTER_MAXED)); // Pet hunger is already at maximum.
		return false;
	}

	if (hungry != pd->pet.hungry) // No need to update the pet's status if hunger value won't change.
		pet->set_hunger(pd, hungry);

	clif->message(fd, msg_fd(fd, MSGTBL_PET_HUNGER_CHANGED)); // Pet hunger changed. (Send message regardless of value has changed or not.)

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(petrename)
{
	struct pet_data *pd;
	if (!sd->status.pet_id || !sd->pd) {
		clif->message(fd, msg_fd(fd, MSGTBL_NO_PET)); // Sorry, but you have no pet.
		return false;
	}
	pd = sd->pd;
	if (!pd->pet.rename_flag) {
		clif->message(fd, msg_fd(fd, MSGTBL_PET_CAN_BE_RENAMED_ALREADY)); // You can already rename your pet.
		return false;
	}

	pd->pet.rename_flag = 0;

	int i;

	ARR_FIND(0, sd->status.inventorySize, i, sd->status.inventory[i].card[0] == CARD0_PET
		 && pd->pet.pet_id == MakeDWord(sd->status.inventory[i].card[1], sd->status.inventory[i].card[2]));

	if (i != sd->status.inventorySize)
		sd->status.inventory[i].card[3] = pet->get_card4_value(pd->pet.rename_flag, pd->pet.intimate);

	intif->save_petdata(sd->status.account_id, &pd->pet);
	clif->send_petstatus(sd);
	clif->message(fd, msg_fd(fd, MSGTBL_CAN_RENAME_PET)); // You can now rename your pet.

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(recall)
{
	struct map_session_data *pl_sd = NULL;

	if (!*message) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_RECALL_PLAYER_NAME)); // Please enter a player name (usage: @recall <char name/ID>).
		return false;
	}

	if ((pl_sd=map->nick2sd(message, true)) == NULL && (pl_sd=map->charid2sd(atoi(message))) == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_CHARACTER_NOT_FOUND)); // Character not found.
		return false;
	}

	if ( pc_get_group_level(sd) < pc_get_group_level(pl_sd) )
	{
		clif->message(fd, msg_fd(fd, MSGTBL_GM_LEVEL_UNAUTHORIZED)); // Your GM level doesn't authorize you to preform this action on the specified player.
		return false;
	}

	if (sd->bl.m >= 0 && map->list[sd->bl.m].flag.nowarpto && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_fd(fd, MSGTBL_NOT_AUTHORIZED_WARP_TO_MAP)); // You are not authorized to warp someone to this map.
		return false;
	}
	if (pl_sd->bl.m >= 0 && map->list[pl_sd->bl.m].flag.nowarp && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_fd(fd, MSGTBL_NOT_AUTHORIZED_WARP_FROM_MAP)); // You are not authorized to warp this player from their map.
		return false;
	}
	if (pl_sd->bl.m == sd->bl.m && pl_sd->bl.x == sd->bl.x && pl_sd->bl.y == sd->bl.y) {
		return false;
	}
	pc->setpos(pl_sd, sd->mapindex, sd->bl.x, sd->bl.y, CLR_RESPAWN);
	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_RECALLED_NAME), pl_sd->status.name); // %s recalled!
	clif->message(fd, atcmd_output);

	return true;
}

/*==========================================
 * charblock command (usage: charblock <player_name>)
 * This command do a definitiv ban on a player
 *------------------------------------------*/
ACMD(char_block)
{

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_BLOCK_PLAYER_NAME)); // Please enter a player name (usage: @block <char name>).
		return false;
	}

	chrif->char_ask_name(sd->status.account_id, atcmd_player_name, CHAR_ASK_NAME_BLOCK, 0, 0, 0, 0, 0, 0);
	clif->message(fd, msg_fd(fd, MSGTBL_SENDING_REQUEST_TO_LOGIN)); // Character name sent to char-server to ask it.

	return true;
}

/*==========================================
 * charban command (usage: charban <time> <player_name>)
 * This command do a limited ban on a player
 * Time is done as follows:
 *   Adjustment value (-1, 1, +1, etc...)
 *   Modified element:
 *     a or y: year
 *     m:  month
 *     j or d: day
 *     h:  hour
 *     mn: minute
 *     s:  second
 * <example> @ban +1m-2mn1s-6y test_player
 *           this example adds 1 month and 1 second, and subtracts 2 minutes and 6 years at the same time.
 *------------------------------------------*/
ACMD(char_ban)
{
	char *modif_p;
	int year, month, day, hour, minute, second;
	time_t timestamp;
	struct tm *tmtime;

	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!*message || sscanf(message, "%255s %23[^\n]", atcmd_output, atcmd_player_name) < 2) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_BAN_TIME_PLAYER_NAME)); // Please enter ban time and a player name (usage: @ban <time> <char name>).
		return false;
	}

	atcmd_output[sizeof(atcmd_output)-1] = '\0';

	modif_p = atcmd_output;
	year = month = day = hour = minute = second = 0;
	while (modif_p[0] != '\0') {
		int value = atoi(modif_p);
		if (value == 0)
			modif_p++;
		else {
			if (modif_p[0] == '-' || modif_p[0] == '+')
				modif_p++;
			while (modif_p[0] >= '0' && modif_p[0] <= '9')
				modif_p++;
			if (modif_p[0] == 's') {
				second = value;
				modif_p++;
			} else if (modif_p[0] == 'n') {
				minute = value;
				modif_p++;
			} else if (modif_p[0] == 'm' && modif_p[1] == 'n') {
				minute = value;
				modif_p = modif_p + 2;
			} else if (modif_p[0] == 'h') {
				hour = value;
				modif_p++;
			} else if (modif_p[0] == 'd' || modif_p[0] == 'j') {
				day = value;
				modif_p++;
			} else if (modif_p[0] == 'm') {
				month = value;
				modif_p++;
			} else if (modif_p[0] == 'y' || modif_p[0] == 'a') {
				year = value;
				modif_p++;
			} else if (modif_p[0] != '\0') {
				modif_p++;
			}
		}
	}
	if (year == 0 && month == 0 && day == 0 && hour == 0 && minute == 0 && second == 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_BAN_TIME)); // Invalid time for ban command.
		return false;
	}
	/**
	 * We now check if you can adjust the ban to negative (and if this is the case)
	 **/
	timestamp = time(NULL);
	tmtime = localtime(&timestamp);
	tmtime->tm_year = tmtime->tm_year + year;
	tmtime->tm_mon  = tmtime->tm_mon + month;
	tmtime->tm_mday = tmtime->tm_mday + day;
	tmtime->tm_hour = tmtime->tm_hour + hour;
	tmtime->tm_min  = tmtime->tm_min + minute;
	tmtime->tm_sec  = tmtime->tm_sec + second;
	timestamp = mktime(tmtime);
	if( timestamp <= time(NULL) && !pc->can_use_command(sd, "@unban") ) {
		clif->message(fd,msg_fd(fd, MSGTBL_NOT_REDUCE_BAN_LENGTH)); // You are not allowed to reduce the length of a ban.
		return false;
	}

	chrif->char_ask_name(sd->status.account_id, atcmd_player_name,
	                     !strcmpi(info->command,"charban") ? CHAR_ASK_NAME_CHARBAN : CHAR_ASK_NAME_BAN, year, month, day, hour, minute, second);
	clif->message(fd, msg_fd(fd, MSGTBL_SENDING_REQUEST_TO_LOGIN)); // Character name sent to char-server to ask it.

	return true;
}

/*==========================================
 * charunblock command (usage: charunblock <player_name>)
 *------------------------------------------*/
ACMD(char_unblock)
{
	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_UNBLOCK_PLAYER_NAME)); // Please enter a player name (usage: @unblock <char name>).
		return false;
	}

	// send answer to login server via char-server
	chrif->char_ask_name(sd->status.account_id, atcmd_player_name, CHAR_ASK_NAME_UNBLOCK, 0, 0, 0, 0, 0, 0);
	clif->message(fd, msg_fd(fd, MSGTBL_SENDING_REQUEST_TO_LOGIN)); // Character name sent to char-server to ask it.

	return true;
}

/*==========================================
 * charunban command (usage: charunban <player_name>)
 *------------------------------------------*/
ACMD(char_unban)
{
	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_UNBAN_PLAYER_NAME)); // Please enter a player name (usage: @unban <char name>).
		return false;
	}

	// send answer to login server via char-server
	chrif->char_ask_name(sd->status.account_id, atcmd_player_name,
	                     !strcmpi(info->command,"charunban") ? CHAR_ASK_NAME_CHARUNBAN : CHAR_ASK_NAME_UNBAN, 0, 0, 0, 0, 0, 0);
	clif->message(fd, msg_fd(fd, MSGTBL_SENDING_REQUEST_TO_LOGIN)); // Character name sent to char-server to ask it.

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(night)
{
	if (map->night_flag != 1) {
		pc->map_night_timer(pc->night_timer_tid, 0, 0, 1);
	} else {
		clif->message(fd, msg_fd(fd, MSGTBL_NIGHT_MODE_ALREADY_ENABLED)); // Night mode is already enabled.
		return false;
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(day)
{
	if (map->night_flag != 0) {
		pc->map_day_timer(pc->day_timer_tid, 0, 0, 1);
	} else {
		clif->message(fd, msg_fd(fd, MSGTBL_DAY_MODE_ALREADY_ENABLED)); // Day mode is already enabled.
		return false;
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(doom)
{
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;

	iter = mapit_getallusers();
	for (pl_sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); pl_sd = BL_UCAST(BL_PC, mapit->next(iter))) {
		if (pl_sd->fd != fd && pc_get_group_level(sd) >= pc_get_group_level(pl_sd)) {
			status_kill(&pl_sd->bl);
			clif->specialeffect(&pl_sd->bl,450,AREA);
			clif->message(pl_sd->fd, msg_fd(fd, MSGTBL_HOLY_MESSENGER_JUDGMENT)); // The holy messenger has given judgment.
		}
	}
	mapit->free(iter);

	clif->message(fd, msg_fd(fd, MSGTBL_JUDGMENT_PASSED)); // Judgment was made.

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(doommap)
{
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;

	iter = mapit_getallusers();
	for (pl_sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); pl_sd = BL_UCAST(BL_PC, mapit->next(iter))) {
		if (pl_sd->fd != fd && sd->bl.m == pl_sd->bl.m && pc_get_group_level(sd) >= pc_get_group_level(pl_sd)) {
			status_kill(&pl_sd->bl);
			clif->specialeffect(&pl_sd->bl,450,AREA);
			clif->message(pl_sd->fd, msg_fd(fd, MSGTBL_HOLY_MESSENGER_JUDGMENT)); // The holy messenger has given judgment.
		}
	}
	mapit->free(iter);

	clif->message(fd, msg_fd(fd, MSGTBL_JUDGMENT_PASSED)); // Judgment was made.

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
static void atcommand_raise_sub(struct map_session_data *sd)
{
	nullpo_retv(sd);
	status->revive(&sd->bl, 100, 100);

	clif->skill_nodamage(&sd->bl,&sd->bl,ALL_RESURRECTION,4,1);
	clif->message(sd->fd, msg_sd(sd, MSGTBL_MERCY_SHOWN)); // Mercy has been shown.
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(raise)
{
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;

	iter = mapit_getallusers();
	for (pl_sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); pl_sd = BL_UCAST(BL_PC, mapit->next(iter)))
		if( pc_isdead(pl_sd) )
			atcommand->raise_sub(pl_sd);
	mapit->free(iter);

	clif->message(fd, msg_fd(fd, MSGTBL_MERCY_GRANTED)); // Mercy has been granted.

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(raisemap)
{
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;

	iter = mapit_getallusers();
	for (pl_sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); pl_sd = BL_UCAST(BL_PC, mapit->next(iter)))
		if (sd->bl.m == pl_sd->bl.m && pc_isdead(pl_sd) )
			atcommand->raise_sub(pl_sd);
	mapit->free(iter);

	clif->message(fd, msg_fd(fd, MSGTBL_MERCY_GRANTED)); // Mercy has been granted.

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(kick)
{
	struct map_session_data *pl_sd;

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!*message) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_KICK_PLAYER_NAME)); // Please enter a player name (usage: @kick <char name/ID>).
		return false;
	}

	if ((pl_sd=map->nick2sd(message, true)) == NULL && (pl_sd=map->charid2sd(atoi(message))) == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_CHARACTER_NOT_FOUND)); // Character not found.
		return false;
	}

	if (pc_get_group_level(sd) < pc_get_group_level(pl_sd))
	{
		clif->message(fd, msg_fd(fd, MSGTBL_GM_LEVEL_UNAUTHORIZED)); // Your GM level don't authorize you to do this action on this player.
		return false;
	}

	clif->GM_kick(sd, pl_sd);

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(kickall)
{
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;

	iter = mapit_getallusers();
	for (pl_sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); pl_sd = BL_UCAST(BL_PC, mapit->next(iter))) {
		if (pc_get_group_level(sd) >= pc_get_group_level(pl_sd)) { // you can kick only lower or same gm level
			if (sd->status.account_id != pl_sd->status.account_id)
				clif->GM_kick(NULL, pl_sd);
		}
	}
	mapit->free(iter);

	clif->message(fd, msg_fd(fd, MSGTBL_ALL_PLAYERS_KICKED)); // All players have been kicked!

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(allskill)
{
	pc->allskillup(sd); // all skills
	sd->status.skill_point = 0; // 0 skill points
	clif->updatestatus(sd, SP_SKILLPOINT); // update
	clif->message(fd, msg_fd(fd, MSGTBL_ALL_SKILLS_ADDED_TO_SKILL_TREE)); // All skills have been added to your skill tree.

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(questskill)
{
	uint16 skill_id, index;

	if (!*message || (skill_id = atoi(message)) <= 0)
	{ // also send a list of skills applicable to this command
		const char* text;

		// attempt to find the text corresponding to this command
		text = atcommand_help_string( info );

		// send the error message as always
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_QUEST_SKILL_ID)); // Please enter a quest skill number.

		if( text ) {// send the skill ID list associated with this command
			clif->messageln( fd, text );
		}

		return false;
	}
	if( !(index = skill->get_index(skill_id)) ) {
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_SKILL_ID)); // This skill number doesn't exist.
		return false;
	}
	if (!(skill->get_inf2(skill_id) & INF2_QUEST_SKILL)) {
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_QUEST_SKILL_ID)); // This skill number doesn't exist or isn't a quest skill.
		return false;
	}
	if (pc->checkskill2(sd, index) > 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_ALREADY_HAVE_QUEST_SKILL)); // You already have this quest skill.
		return false;
	}

	pc->skill(sd, skill_id, 1, SKILL_GRANT_PERMANENT);
	clif->message(fd, msg_fd(fd, MSGTBL_LEARNED_SKILL)); // You have learned the skill.

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(lostskill)
{
	uint16 skill_id, index;

	if (!*message || (skill_id = atoi(message)) <= 0)
	{ // also send a list of skills applicable to this command
		const char* text;

		// attempt to find the text corresponding to this command
		text = atcommand_help_string( info );

		// send the error message as always
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_QUEST_SKILL_ID)); // Please enter a quest skill number.

		if( text ) {// send the skill ID list associated with this command
			clif->messageln( fd, text );
		}

		return false;
	}
	if (!( index = skill->get_index(skill_id))) {
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_SKILL_ID)); // This skill number doesn't exist.
		return false;
	}
	if (!(skill->get_inf2(skill_id) & INF2_QUEST_SKILL)) {
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_QUEST_SKILL_ID)); // This skill number doesn't exist or isn't a quest skill.
		return false;
	}
	if (pc->checkskill2(sd, index) == 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_QUEST_SKILL_NOT_LEARNED)); // You don't have this quest skill.
		return false;
	}

	sd->status.skill[index].lv = 0;
	sd->status.skill[index].flag = 0;
	clif->deleteskill(sd, skill_id, false);
	clif->message(fd, msg_fd(fd, MSGTBL_FORGOTTEN_SKILL)); // You have forgotten the skill.

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(spiritball)
{
	int max_spiritballs;
	int number;

	max_spiritballs = min(ARRAYLENGTH(sd->spirit_timer), 0x7FFF);

	if (!*message || (number = atoi(message)) < 0 || number > max_spiritballs)
	{
		char msg[CHAT_SIZE_MAX];
		snprintf(msg, sizeof(msg), msg_fd(fd, MSGTBL_ENTER_SPIRITBALL_AMOUNT), max_spiritballs); // Please enter an amount (usage: @spiritball <number: 0-%d>).
		clif->message(fd, msg);
		return false;
	}

	if( sd->spiritball > 0 )
		pc->delspiritball(sd, sd->spiritball, 1);
	sd->spiritball = number;
	clif->spiritballs(&sd->bl, sd->spiritball, AREA);
	// no message, player can look the difference

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
 ACMD(soulball)
{
	int number;

	if (!*message || (number = atoi(message)) < 0 || number > MAX_SOUL_BALL) {
		char msg[CHAT_SIZE_MAX];
		snprintf(msg, sizeof(msg), "Usage: @soulball <number: 0-%d>", MAX_SOUL_BALL);
		clif->message(fd, msg);
		return false;
	}

	if (sd->soulball > 0)
		pc->delsoulball(sd, sd->soulball, true);

	for (int i = 0; i < number; i++)
		pc->addsoulball(sd, MAX_SOUL_BALL);

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(party)
{
	char party_name[NAME_LENGTH];

	memset(party_name, '\0', sizeof(party_name));

	if (!*message || sscanf(message, "%23[^\n]", party_name) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_PARTY_NAME)); // Please enter a party name (usage: @party <party_name>).
		return false;
	}

	party->create(sd, party_name, 0, 0);

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(guild)
{
	char guild_name[NAME_LENGTH];
	int prev;

	memset(guild_name, '\0', sizeof(guild_name));

	if (!*message || sscanf(message, "%23[^\n]", guild_name) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_GUILD_NAME)); // Please enter a guild name (usage: @guild <guild_name>).
		return false;
	}

	prev = battle_config.guild_emperium_check;
	battle_config.guild_emperium_check = 0;
	guild->create(sd, guild_name);
	battle_config.guild_emperium_check = prev;

	return true;
}

ACMD(breakguild)
{
	if (sd->status.guild_id) { // Check if the player has a guild
		struct guild *g = sd->guild; // Search the guild
		if (g) { // Check if guild was found
			if (sd->state.gmaster_flag) { // Check if player is guild master
				int ret = 0;
				ret = guild->dobreak(sd, g->name); // Break guild
				if (ret) { // Check if anything went wrong
					return true; // Guild was broken
				} else {
					return false; // Something went wrong
				}
			} else { // Not guild master
				clif->message(fd, msg_fd(fd, MSGTBL_CHANGEGM_GM_REQUIRED)); // You need to be a Guild Master to use this command.
				return false;
			}
		} else { // Guild was not found. HOW?
			clif->message(fd, msg_fd(fd, MSGTBL_NOT_IN_A_GUILD2)); // You are not in a guild.
			return false;
		}
	} else { // Player does not have a guild
		clif->message(fd, msg_fd(fd, MSGTBL_NOT_IN_A_GUILD2)); // You are not in a guild.
		return false;
	}
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(agitstart)
{
	if (map->agit_flag == 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_AGITSTART_ALREADY_IN_PROGRESS)); // War of Emperium is currently in progress.
		return false;
	}

	map->agit_flag = 1;
	guild->agit_start();
	clif->message(fd, msg_fd(fd, MSGTBL_AGITSTART_INITIATED)); // War of Emperium has been initiated.

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(agitstart2)
{
	if (map->agit2_flag == 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_AGITSTART2_ALREADY_IN_PROGRESS)); // "War of Emperium SE is currently in progress."
		return false;
	}

	map->agit2_flag = 1;
	guild->agit2_start();
	clif->message(fd, msg_fd(fd, MSGTBL_AGITSTART2_INITIATED)); // "War of Emperium SE has been initiated."

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(agitend)
{
	if (map->agit_flag == 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_AGITEND_NOT_IN_PROGRESS)); // War of Emperium is currently not in progress.
		return false;
	}

	map->agit_flag = 0;
	guild->agit_end();
	clif->message(fd, msg_fd(fd, MSGTBL_AGITEND_ENDED)); // War of Emperium has been ended.

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(agitend2)
{
	if (map->agit2_flag == 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_AGITEND2_NOT_IN_PROGRESS)); // "War of Emperium SE is currently not in progress."
		return false;
	}

	map->agit2_flag = 0;
	guild->agit2_end();
	clif->message(fd, msg_fd(fd, MSGTBL_AGITEND2_ENDED)); // "War of Emperium SE has been ended."

	return true;
}

/*==========================================
 * @mapexit - shuts down the map server
 *------------------------------------------*/
ACMD(mapexit)
{
	map->do_shutdown();
	return true;
}

/*==========================================
 * idsearch <part_of_name>: rewrite by [Yor]
 *------------------------------------------*/
ACMD(idsearch)
{
	char item_name[100];
	unsigned int i, match;
	struct item_data *item_array[MAX_SEARCH];

	memset(item_name, '\0', sizeof(item_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!*message || sscanf(message, "%99s", item_name) < 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_IDSEARCH_PART)); // Please enter part of an item name (usage: @idsearch <part_of_item_name>).
		return false;
	}

	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_SEARCH_RESULTS_NAME), item_name); // Search results for '%s' (name: id):
	clif->message(fd, atcmd_output);
	match = itemdb->search_name_array(item_array, MAX_SEARCH, item_name, IT_SEARCH_NAME_PARTIAL);
	if (match > MAX_SEARCH) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_SEARCH_RESULT_OFFSET), MAX_SEARCH, match);
		clif->message(fd, atcmd_output);
		match = MAX_SEARCH;
	}
	for(i = 0; i < match; i++) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_SEARCH_RESULT_NAME_ID), item_array[i]->jname, item_array[i]->nameid); // %s: %d
		clif->message(fd, atcmd_output);
	}
	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_N_RESULTS_FOUND), match); // %d results found.
	clif->message(fd, atcmd_output);

	return true;
}

/*==========================================
 * Recall All Characters Online To Your Location
 *------------------------------------------*/
ACMD(recallall)
{
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;
	int count;

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (sd->bl.m >= 0 && map->list[sd->bl.m].flag.nowarpto && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_fd(fd, MSGTBL_NOT_AUTHORIZED_WARP_TO)); // You are not authorized to warp someone to your current map.
		return false;
	}

	count = 0;
	iter = mapit_getallusers();
	for (pl_sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); pl_sd = BL_UCAST(BL_PC, mapit->next(iter))) {
		if (sd->status.account_id != pl_sd->status.account_id && pc_get_group_level(sd) >= pc_get_group_level(pl_sd)) {
			if (pl_sd->bl.m == sd->bl.m && pl_sd->bl.x == sd->bl.x && pl_sd->bl.y == sd->bl.y)
				continue; // Don't waste time warping the character to the same place.
			if (pl_sd->bl.m >= 0 && map->list[pl_sd->bl.m].flag.nowarp && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE))
				count++;
			else {
				if (pc_isdead(pl_sd)) { //Wake them up
					pc->setstand(pl_sd);
					pc->setrestartvalue(pl_sd,1);
				}
				pc->setpos(pl_sd, sd->mapindex, sd->bl.x, sd->bl.y, CLR_RESPAWN);
			}
		}
	}
	mapit->free(iter);

	clif->message(fd, msg_fd(fd, MSGTBL_RECALL_CHARACTERS_RECALLED)); // All characters recalled!
	if (count) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_NOT_AUTHORIZED_WARP_FROM), count); // Because you are not authorized to warp from some maps, %d player(s) have not been recalled.
		clif->message(fd, atcmd_output);
	}

	return true;
}

/*==========================================
 * Recall online characters of a guild to your location
 *------------------------------------------*/
ACMD(guildrecall)
{
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;
	int count;
	char guild_name[NAME_LENGTH];
	struct guild *g;

	memset(guild_name, '\0', sizeof(guild_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!*message || sscanf(message, "%23[^\n]", guild_name) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_GUILDRECALL_NAME_ID)); // Please enter a guild name/ID (usage: @guildrecall <guild_name/ID>).
		return false;
	}

	if (sd->bl.m >= 0 && map->list[sd->bl.m].flag.nowarpto && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_fd(fd, MSGTBL_NOT_AUTHORIZED_WARP_TO)); // You are not authorized to warp someone to your current map.
		return false;
	}

	if ((g = guild->searchname(guild_name)) == NULL && // name first to avoid error when name begin with a number
	    (g = guild->search(atoi(message))) == NULL)
	{
		clif->message(fd, msg_fd(fd, MSGTBL_RECALL_INVALID_GUILD_NAME)); // Incorrect name/ID, or no one from the guild is online.
		return false;
	}

	count = 0;

	iter = mapit_getallusers();
	for (pl_sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); pl_sd = BL_UCAST(BL_PC, mapit->next(iter))) {
		if (sd->status.account_id != pl_sd->status.account_id && pl_sd->status.guild_id == g->guild_id) {
			if (pc_get_group_level(pl_sd) > pc_get_group_level(sd) || (pl_sd->bl.m == sd->bl.m && pl_sd->bl.x == sd->bl.x && pl_sd->bl.y == sd->bl.y))
				continue; // Skip GMs greater than you...             or chars already on the cell
			if (pl_sd->bl.m >= 0 && map->list[pl_sd->bl.m].flag.nowarp && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE))
				count++;
			else
				pc->setpos(pl_sd, sd->mapindex, sd->bl.x, sd->bl.y, CLR_RESPAWN);
		}
	}
	mapit->free(iter);

	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_RECALL_GUILD_RECALLED), g->name); // All online characters of the %s guild have been recalled to your position.
	clif->message(fd, atcmd_output);
	if (count) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_NOT_AUTHORIZED_WARP_FROM), count); // Because you are not authorized to warp from some maps, %d player(s) have not been recalled.
		clif->message(fd, atcmd_output);
	}

	return true;
}

/*==========================================
 * Recall online characters of a party to your location
 *------------------------------------------*/
ACMD(partyrecall)
{
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;
	char party_name[NAME_LENGTH];
	struct party_data *p;
	int count;

	memset(party_name, '\0', sizeof(party_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!*message || sscanf(message, "%23[^\n]", party_name) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_PARTYRECALL_NAME_ID)); // Please enter a party name/ID (usage: @partyrecall <party_name/ID>).
		return false;
	}

	if (sd->bl.m >= 0 && map->list[sd->bl.m].flag.nowarpto && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_fd(fd, MSGTBL_NOT_AUTHORIZED_WARP_TO)); // You are not authorized to warp someone to your current map.
		return false;
	}

	if ((p = party->searchname(party_name)) == NULL && // name first to avoid error when name begin with a number
	    (p = party->search(atoi(message))) == NULL)
	{
		clif->message(fd, msg_fd(fd, MSGTBL_RECALL_INVALID_PARTY_NAME)); // Incorrect name or ID, or no one from the party is online.
		return false;
	}

	count = 0;

	iter = mapit_getallusers();
	for (pl_sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); pl_sd = BL_UCAST(BL_PC, mapit->next(iter))) {
		if (sd->status.account_id != pl_sd->status.account_id && pl_sd->status.party_id == p->party.party_id) {
			if (pc_get_group_level(pl_sd) > pc_get_group_level(sd) || (pl_sd->bl.m == sd->bl.m && pl_sd->bl.x == sd->bl.x && pl_sd->bl.y == sd->bl.y))
				continue; // Skip GMs greater than you...             or chars already on the cell
			if (pl_sd->bl.m >= 0 && map->list[pl_sd->bl.m].flag.nowarp && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE))
				count++;
			else
				pc->setpos(pl_sd, sd->mapindex, sd->bl.x, sd->bl.y, CLR_RESPAWN);
		}
	}
	mapit->free(iter);

	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_RECALL_PARTY_RECALLED), p->party.name); // All online characters of the %s party have been recalled to your position.
	clif->message(fd, atcmd_output);
	if (count) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_NOT_AUTHORIZED_WARP_FROM), count); // Because you are not authorized to warp from some maps, %d player(s) have not been recalled.
		clif->message(fd, atcmd_output);
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(reloaditemdb)
{
	itemdb->reload();
	clif->message(fd, msg_fd(fd, MSGTBL_RELOAD_ITEMDB_RELOADED)); // Item database has been reloaded.

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(reloadmobdb)
{
	mob->reload();
	pet->read_db();
	homun->reload();
	mercenary->read_db();
	mercenary->read_skilldb();
	elemental->reload_db();
	clif->message(fd, msg_fd(fd, MSGTBL_RELOAD_MOBDB_RELOADED)); // Monster database has been reloaded.

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(reloadskilldb)
{
	skill->reload();
	status->load_sc_type();
	homun->reload_skill();
	elemental->reload_skilldb();
	mercenary->read_skilldb();
	clif->message(fd, msg_fd(fd, MSGTBL_RELOAD_SKILLDB_RELOADED)); // Skill database has been reloaded.

	return true;
}

/*==========================================
 * @reloadatcommand - reloads conf/atcommand.conf conf/groups.conf
 *------------------------------------------*/
ACMD(reloadatcommand)
{
	struct config_t run_test;

	if (!libconfig->load_file(&run_test, "conf/groups.conf")) {
		clif->message(fd, msg_fd(fd, MSGTBL_ERROR_READ_GROUPS_CONF)); // Error reading groups.conf, reload failed.
		return false;
	}

	libconfig->destroy(&run_test);

	if (!libconfig->load_file(&run_test, map->ATCOMMAND_CONF_FILENAME)) {
		clif->message(fd, msg_fd(fd, MSGTBL_ERROR_READ_ATCOMMAND_CONF)); // Error reading atcommand.conf, reload failed.
		return false;
	}

	libconfig->destroy(&run_test);

	atcommand->doload();
	pcg->reload();
	clif->message(fd, msg_fd(fd, MSGTBL_RELOAD_ATCOMMAND_RELOADED));
	return true;
}
/*==========================================
 * @reloadbattleconf - reloads /conf/battle.conf
 *------------------------------------------*/
ACMD(reloadbattleconf)
{
	struct Battle_Config prev_config;
	memcpy(&prev_config, &battle_config, sizeof(prev_config));

	battle->config_read(map->BATTLE_CONF_FILENAME, false);
	if (prev_config.feature_roulette == 0 && battle_config.feature_roulette == 1 && !clif->parse_roulette_db())
		battle_config.feature_roulette = 0;

	if( prev_config.item_rate_mvp              != battle_config.item_rate_mvp
	   ||  prev_config.item_rate_common        != battle_config.item_rate_common
	   ||  prev_config.item_rate_common_boss   != battle_config.item_rate_common_boss
	   ||  prev_config.item_rate_card          != battle_config.item_rate_card
	   ||  prev_config.item_rate_card_boss     != battle_config.item_rate_card_boss
	   ||  prev_config.item_rate_equip         != battle_config.item_rate_equip
	   ||  prev_config.item_rate_equip_boss    != battle_config.item_rate_equip_boss
	   ||  prev_config.item_rate_heal          != battle_config.item_rate_heal
	   ||  prev_config.item_rate_heal_boss     != battle_config.item_rate_heal_boss
	   ||  prev_config.item_rate_use           != battle_config.item_rate_use
	   ||  prev_config.item_rate_use_boss      != battle_config.item_rate_use_boss
	   ||  prev_config.item_rate_treasure      != battle_config.item_rate_treasure
	   ||  prev_config.item_rate_adddrop       != battle_config.item_rate_adddrop
	   ||  prev_config.item_rate_add_chain     != battle_config.item_rate_add_chain
	   ||  prev_config.logarithmic_drops       != battle_config.logarithmic_drops
	   ||  prev_config.item_drop_common_min    != battle_config.item_drop_common_min
	   ||  prev_config.item_drop_common_max    != battle_config.item_drop_common_max
	   ||  prev_config.item_drop_card_min      != battle_config.item_drop_card_min
	   ||  prev_config.item_drop_card_max      != battle_config.item_drop_card_max
	   ||  prev_config.item_drop_equip_min     != battle_config.item_drop_equip_min
	   ||  prev_config.item_drop_equip_max     != battle_config.item_drop_equip_max
	   ||  prev_config.item_drop_mvp_min       != battle_config.item_drop_mvp_min
	   ||  prev_config.item_drop_mvp_max       != battle_config.item_drop_mvp_max
	   ||  prev_config.item_drop_add_chain_min != battle_config.item_drop_add_chain_min
	   ||  prev_config.item_drop_add_chain_max != battle_config.item_drop_add_chain_max
	   ||  prev_config.item_drop_heal_min      != battle_config.item_drop_heal_min
	   ||  prev_config.item_drop_heal_max      != battle_config.item_drop_heal_max
	   ||  prev_config.item_drop_use_min       != battle_config.item_drop_use_min
	   ||  prev_config.item_drop_use_max       != battle_config.item_drop_use_max
	   ||  prev_config.item_drop_treasure_min  != battle_config.item_drop_treasure_min
	   ||  prev_config.item_drop_treasure_max  != battle_config.item_drop_treasure_max
	   ||  prev_config.base_exp_rate           != battle_config.base_exp_rate
	   ||  prev_config.job_exp_rate            != battle_config.job_exp_rate
	) { // Exp or Drop rates changed.
		itemdb->read_chains();
		mob->reload(); //Needed as well so rate changes take effect.
		chrif->ragsrvinfo(battle_config.base_exp_rate, battle_config.job_exp_rate, battle_config.item_rate_common);
	}
	clif->message(fd, msg_fd(fd, MSGTBL_RELOAD_BATTLE_RELOADED));
	return true;
}
/*==========================================
 * @reloadstatusdb - reloads job_db1.txt job_db2.txt job_db2-2.txt refine_db.txt size_fix.txt
 *------------------------------------------*/
ACMD(reloadstatusdb)
{
	status->readdb();
	clif->message(fd, msg_fd(fd, MSGTBL_RELOAD_STATUS_RELOADED));
	return true;
}
/*==========================================
 * @reloadpcdb - reloads exp_group_db.conf skill_tree.txt attr_fix.txt statpoint.txt
 *------------------------------------------*/
ACMD(reloadpcdb)
{
	pc->readdb();
	clif->message(fd, msg_fd(fd, MSGTBL_RELOAD_PLAYER_RELOADED));
	return true;
}

/*==========================================
 * @reloadscript - reloads all scripts (npcs, warps, mob spawns, ...)
 *------------------------------------------*/
ACMD(reloadscript)
{
	struct s_mapiterator* iter;
	struct map_session_data* pl_sd;

	//atcommand_broadcast( fd, sd, "@broadcast", "Server is reloading scripts..." );
	//atcommand_broadcast( fd, sd, "@broadcast", "You will feel a bit of lag at this point !" );

	iter = mapit_getallusers();
	for (pl_sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); pl_sd = BL_UCAST(BL_PC, mapit->next(iter))) {
		if (pl_sd->npc_id || pl_sd->npc_shopid) {
			if (pl_sd->state.using_fake_npc) {
				clif->clearunit_single(pl_sd->npc_id, CLR_OUTSIGHT, pl_sd->fd);
				pl_sd->state.using_fake_npc = 0;
			}
			if (pl_sd->state.menu_or_input)
				pl_sd->state.menu_or_input = 0;
			if (pl_sd->npc_menu)
				pl_sd->npc_menu = 0;

			pl_sd->npc_id = 0;
			pl_sd->npc_shopid = 0;
			if (pl_sd->st && pl_sd->st->state != END)
				pl_sd->st->state = END;
		}
	}
	mapit->free(iter);

	sockt->flush_fifos();
	map->reloadnpc(true); // reload config files seeking for npcs
	script->reload();
	npc->reload();

	clif->message(fd, msg_fd(fd, MSGTBL_RELOAD_SCRIPTS_RELOADED)); // Scripts have been reloaded.

	return true;
}

/*==========================================
 * @mapinfo [0-3] <map name> by MC_Cameri
 * => Shows information about the map [map name]
 * 0 = no additional information
 * 1 = Show users in that map and their location
 * 2 = Shows NPCs in that map
 * 3 = Shows the chats in that map
 * TODO# add the missing mapflags e.g. adjust_skill_damage to display
 *------------------------------------------*/
ACMD(mapinfo)
{
	const struct map_session_data *pl_sd;
	struct s_mapiterator *iter;
	const struct chat_data *cd = NULL;
	char direction[12];
	int i, m_id, chat_num = 0, list = 0, vend_num = 0;
	unsigned short m_index;
	char mapname[24];

	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(mapname, '\0', sizeof(mapname));
	memset(direction, '\0', sizeof(direction));

	sscanf(message, "%12d %23[^\n]", &list, mapname);

	if (list < 0 || list > 3) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_VALID_LIST_NUMBER)); // Please enter at least one valid list number (usage: @mapinfo <0-3> <map>).
		return false;
	}

	if (mapname[0] == '\0') {
		safestrncpy(mapname, mapindex_id2name(sd->mapindex), MAP_NAME_LENGTH);
		m_id = map->mapindex2mapid(sd->mapindex);
	} else {
		m_id = map->mapname2mapid(mapname);
	}

	if (m_id < 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_MAP_NOT_FOUND)); // Map not found.
		return false;
	}
	m_index = mapindex->name2id(mapname); //This one shouldn't fail since the previous seek did not.

	clif->message(fd, msg_fd(fd, MSGTBL_MAP_INFO_SEPARATOR)); // ------ Map Info ------

	// count chats (for initial message)
	chat_num = 0;
	iter = mapit_getallusers();
	for (pl_sd = BL_UCCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); pl_sd = BL_UCCAST(BL_PC, mapit->next(iter))) {
		if (pl_sd->mapindex == m_index) {
			if (pl_sd->state.vending != 0)
				vend_num++;
			else if ((cd = map->id2cd(pl_sd->chat_id)) != NULL && cd->usersd[0] == pl_sd)
				chat_num++;
		}
	}
	mapit->free(iter);

	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_MAP_INFO), mapname, map->list[m_id].zone->name, map->list[m_id].users, map->list[m_id].npc_num, chat_num, vend_num); // Map: %s (Zone:%s) | Players: %d | NPCs: %d | Chats: %d | Vendings: %d
	clif->message(fd, atcmd_output);
	clif->message(fd, msg_fd(fd, MSGTBL_MAP_FLAGS)); // ------ Map Flags ------
	if (map->list[m_id].flag.town != 0)
		clif->message(fd, msg_fd(fd, MSGTBL_MAPINFO_TOWN_MAP)); // Town Map

	if (battle_config.autotrade_mapflag == map->list[m_id].flag.autotrade)
		clif->message(fd, msg_fd(fd, MSGTBL_MAPINFO_AUTOTRADE_ENABLED)); // Autotrade Enabled
	else
		clif->message(fd, msg_fd(fd, MSGTBL_MAPINFO_AUTOTRADE_DISABLED)); // Autotrade Disabled

	if (map->list[m_id].flag.battleground != 0)
		clif->message(fd, msg_fd(fd, MSGTBL_MAPINFO_BATTLEGROUND_ON)); // Battlegrounds ON

	if (map->list[m_id].flag.cvc != 0)
		clif->message(fd, msg_fd(fd, MSGTBL_MAPINFO_CVC_ON)); // CvC ON

	strcpy(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_PVP_FLAGS)); // PvP Flags:
	if (map->list[m_id].flag.pvp != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_PVP_ON)); // Pvp ON |
	if (map->list[m_id].flag.pvp_noguild != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_GUILD)); // NoGuild |
	if (map->list[m_id].flag.pvp_noparty != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_PARTY)); // NoParty |
	if (map->list[m_id].flag.pvp_nightmaredrop != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NIGHTMARE_DROP)); // NightmareDrop |
	if (map->list[m_id].flag.pvp_nocalcrank != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_CALC_RANK)); // NoCalcRank |
	clif->message(fd, atcmd_output);

	strcpy(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_GVG_FLAGS)); // GvG Flags:
	if (map->list[m_id].flag.gvg != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_GVG_ON)); // GvG ON |
	if (map->list[m_id].flag.gvg_dungeon != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_GVG_DUNGEON)); // GvG Dungeon |
	if (map->list[m_id].flag.gvg_castle != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_GVG_CASTLE)); // GvG Castle |
	if (map->list[m_id].flag.gvg_noparty != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_PARTY_GVG)); // NoParty |
	clif->message(fd, atcmd_output);

	strcpy(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_TELEPORT_FLAGS)); // Teleport Flags:
	if (map->list[m_id].flag.noteleport != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_TELEPORT)); // NoTeleport |
	if (map->list[m_id].flag.monster_noteleport != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_MONSTER_NO_TELEPORT)); // Monster NoTeleport |
	if (map->list[m_id].flag.nowarp != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_WARP)); // NoWarp |
	if (map->list[m_id].flag.nowarpto != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_WARP_TO)); // NoWarpTo |
	if (map->list[m_id].flag.noreturn != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_RETURN)); // NoReturn |
	if (map->list[m_id].flag.nomemo != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_MEMO)); // NoMemo |
	clif->message(fd, atcmd_output);

	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_MAPINFO_NO_PENALTY),  // No Exp Penalty: %s | No Zeny Penalty: %s
		(map->list[m_id].flag.noexppenalty != 0) ? msg_fd(fd, MSGTBL_MAPINFO_ON) : msg_fd(fd, MSGTBL_MAPINFO_OFF),
		(map->list[m_id].flag.nozenypenalty != 0) ? msg_fd(fd, MSGTBL_MAPINFO_ON) : msg_fd(fd, MSGTBL_MAPINFO_OFF)); // On / Off
	clif->message(fd, atcmd_output);

	if (map->list[m_id].flag.nosave != 0) {
		if (!map->list[m_id].save.map)
			clif->message(fd, msg_fd(fd, MSGTBL_MAPINFO_NO_SAVE)); // No Save (Return to last Save Point)
		else if (map->list[m_id].save.x == -1 || map->list[m_id].save.y == -1) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_MAPINFO_NO_SAVE_RANDOM), mapindex_id2name(map->list[m_id].save.map)); // No Save, Save Point: %s,Random
			clif->message(fd, atcmd_output);
		} else {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_MAPINFO_NO_SAVE_SPECIFIC), // No Save, Save Point: %s,%d,%d
				mapindex_id2name(map->list[m_id].save.map), map->list[m_id].save.x, map->list[m_id].save.y);
			clif->message(fd, atcmd_output);
		}
	}

	strcpy(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_WEATHER_FLAGS)); // Weather Flags:
	if (map->list[m_id].flag.snow != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_SNOW)); // Snow |
	if (map->list[m_id].flag.fog != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_FOG)); // Fog |
	if (map->list[m_id].flag.sakura != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_SAKURA)); // Sakura |
	if (map->list[m_id].flag.clouds != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_CLOUDS)); // Clouds |
	if (map->list[m_id].flag.clouds2 != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_CLOUDS2)); // Clouds2 |
	if (map->list[m_id].flag.fireworks != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_FIREWORKS)); // Fireworks |
	if (map->list[m_id].flag.leaves != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_LEAVES)); // Leaves |
	if (map->list[m_id].flag.nightenabled != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_DISPLAYS_NIGHT)); // Displays Night |
	clif->message(fd, atcmd_output);

	strcpy(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_OTHER_FLAGS)); // Other Flags:
	if (map->list[m_id].flag.nobranch != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_BRANCH)); // NoBranch |
	if (map->list[m_id].flag.notrade != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_TRADE)); // NoTrade |
	if (map->list[m_id].flag.novending != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_VENDING)); // NoVending |
	if (map->list[m_id].flag.nodrop != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_DROP)); // NoDrop |
	if (map->list[m_id].flag.noskill != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_SKILL)); // NoSkill |
	if (map->list[m_id].flag.noicewall != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_ICEWALL)); // NoIcewall |
	if (map->list[m_id].flag.allowks != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_ALLOW_KS)); // AllowKS |
	if (map->list[m_id].flag.reset != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_RESET)); // Reset |
	if (map->list[m_id].flag.src4instance != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_KNOCKBACK)); // No Knockback |
	clif->message(fd, atcmd_output);

	strcpy(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_OTHER_FLAGS_2)); // Other Flags:
	if (map->list[m_id].nocommand != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_COMMAND)); // NoCommand |
	if (map->list[m_id].flag.nobaseexp != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_BASE_EXP)); // NoBaseEXP |
	if (map->list[m_id].flag.nojobexp != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_JOB_EXP)); // NoJobEXP |
	if (map->list[m_id].flag.nomobloot != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_MOB_LOOT)); // NoMobLoot |
	if (map->list[m_id].flag.nomvploot != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_MVP_LOOT)); // NoMVPLoot |
	if (map->list[m_id].flag.partylock != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_PARTY_LOCK)); // PartyLock |
	if (map->list[m_id].flag.guildlock != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_GUILD_LOCK)); // GuildLock |
	if (map->list[m_id].flag.noautoloot != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_AUTOLOOT)); // NoAutoloot |
	if (map->list[m_id].flag.noviewid != EQP_NONE)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_VIEW_ID)); // NoViewID |
	if (map->list[m_id].flag.pairship_startable != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_PRIVATE_AIRSHIP_STARTABLE)); // PrivateAirshipStartable |
	if (map->list[m_id].flag.pairship_endable != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_PRIVATE_AIRSHIP_ENDABLE)); // PrivateAirshipEndable |
	if (map->list[m_id].flag.noknockback != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_KNOCKBACK)); // No Knockback |
	if (map->list[m_id].flag.src4instance != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_SOURCE_FOR_INSTANCE)); // Source For Instance |
	if (map->list[m_id].flag.chsysnolocalaj != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_CHANNEL_AUTOJOIN)); // No Map Channel Auto Join |
	if (map->list[m_id].flag.nopet != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_NO_PET)); // NoPet |
	if (map->list[m_id].flag.specialpopup != 0)
		strcat(atcmd_output, msg_fd(fd, MSGTBL_MAPINFO_SPECIAL_POPUP)); // SpecialPopup |

	clif->message(fd, atcmd_output);

	switch (list) {
	case 0:
		// Do nothing. It's list 0, no additional display.
		break;
	case 1:
		clif->message(fd, msg_fd(fd, MSGTBL_PLAYERS_IN_MAP)); // ----- Players in Map -----
		iter = mapit_getallusers();
		for (pl_sd = BL_UCCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); pl_sd = BL_UCCAST(BL_PC, mapit->next(iter))) {
			if (pl_sd->mapindex == m_index) {
				snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_PLAYER_INFO), // Player '%s' (session #%d) | Location: %d,%d
					pl_sd->status.name, pl_sd->fd, pl_sd->bl.x, pl_sd->bl.y);
				clif->message(fd, atcmd_output);
			}
		}
		mapit->free(iter);
		break;
	case 2:
		clif->message(fd, msg_fd(fd, MSGTBL_NPCS_IN_MAP)); // ----- NPCs in Map -----
		for (i = 0; i < map->list[m_id].npc_num;) {
			struct npc_data *nd = map->list[m_id].npc[i];
			switch (nd->dir) {
			case UNIT_DIR_NORTH:
				strcpy(direction, msg_fd(fd, MSGTBL_NORTH)); // North
				break;
			case UNIT_DIR_NORTHWEST:
				strcpy(direction, msg_fd(fd, MSGTBL_NORTH_WEST)); // North West
				break;
			case UNIT_DIR_WEST:
				strcpy(direction, msg_fd(fd, MSGTBL_WEST)); // West
				break;
			case UNIT_DIR_SOUTHWEST:
				strcpy(direction, msg_fd(fd, MSGTBL_SOUTH_WEST)); // South West
				break;
			case UNIT_DIR_SOUTH:
				strcpy(direction, msg_fd(fd, MSGTBL_SOUTH)); // South
				break;
			case UNIT_DIR_SOUTHEAST:
				strcpy(direction, msg_fd(fd, MSGTBL_SOUTH_EAST)); // South East
				break;
			case UNIT_DIR_EAST:
				strcpy(direction, msg_fd(fd, MSGTBL_EAST)); // East
				break;
			case UNIT_DIR_NORTHEAST:
				strcpy(direction, msg_fd(fd, MSGTBL_NORTH_EAST)); // North East
				break;
			case UNIT_DIR_9: // is this actually used? [skyleo]
				strcpy(direction, msg_fd(fd, MSGTBL_NORTH_2)); // North
				break;
			case UNIT_DIR_UNDEFINED:
			case UNIT_DIR_MAX:
			default:
				strcpy(direction, msg_fd(fd, MSGTBL_MAPINFO_UNKNOWN)); // Unknown
				break;
			}
			if (strcmp(nd->name, nd->exname) == 0)
				snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_NPC_INFO), // NPC %d: %s | Direction: %s | Sprite: %d | Location: %d %d
					++i, nd->name, direction, nd->class_, nd->bl.x, nd->bl.y);
			else
				snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_NPC_EXTENDED_INFO), // NPC %d: %s::%s | Direction: %s | Sprite: %d | Location: %d %d
					++i, nd->name, nd->exname, direction, nd->class_, nd->bl.x, nd->bl.y);
			clif->message(fd, atcmd_output);
		}
		break;
	case 3:
		clif->message(fd, msg_fd(fd, MSGTBL_CHATS_IN_MAP)); // ----- Chats in Map -----
		iter = mapit_getallusers();
		for (pl_sd = BL_UCCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); pl_sd = BL_UCCAST(BL_PC, mapit->next(iter))) {
			if ((cd = map->id2cd(pl_sd->chat_id)) != NULL && pl_sd->mapindex == m_index && cd->usersd[0] == pl_sd) {
				snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHAT_INFO), // Chat: %s | Player: %s | Location: %d %d
					cd->title, pl_sd->status.name, cd->bl.x, cd->bl.y);
				clif->message(fd, atcmd_output);
				snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHAT_ADDITIONAL_INFO), //    Users: %d/%d | Password: %s | Public: %s
					cd->users, cd->limit, cd->pass, (cd->pub) ? msg_fd(fd, MSGTBL_YES) : msg_fd(fd, MSGTBL_NO)); // Yes / No
				clif->message(fd, atcmd_output);
			}
		}
		mapit->free(iter);
		break;
	default: // normally impossible to arrive here
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_VALID_LIST_NUMBER_2)); // Please enter at least one valid list number (usage: @mapinfo <0-3> <map>).
		return false;
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(mount_peco)
{
	if (sd->disguise != -1) {
		clif->message(fd, msg_fd(fd, MSGTBL_CANT_MOUNT_IN_DISGUISE)); // Cannot mount while in disguise.
		return false;
	}

	if (sd->sc.data[SC_ALL_RIDING]) {
		clif->message(fd, msg_fd(fd, MSGTBL_ALREADY_MOUNTED)); // You are already mounting something else
		return false;
	}

	if ((sd->job & MAPID_THIRDMASK) == MAPID_RUNE_KNIGHT) {
		if (!pc->checkskill(sd,RK_DRAGONTRAINING)) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REQUIREMENT_FOR_MOUNT), skill->get_desc(RK_DRAGONTRAINING)); // You need %s to mount!
			clif->message(fd, atcmd_output);
			return false;
		}
		if (!pc_isridingdragon(sd)) {
			clif->message(sd->fd,msg_fd(fd, MSGTBL_MOUNTED_DRAGON)); // You have mounted your Dragon.
			pc->setridingdragon(sd, OPTION_DRAGON1);
		} else {
			clif->message(sd->fd,msg_fd(fd, MSGTBL_RELEASED_DRAGON)); // You have released your Dragon.
			pc->setridingdragon(sd, 0);
		}
		return true;
	}
	if ((sd->job & MAPID_THIRDMASK) == MAPID_RANGER) {
		if (!pc->checkskill(sd,RA_WUGRIDER)) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REQUIREMENT_FOR_MOUNT), skill->get_desc(RA_WUGRIDER)); // You need %s to mount!
			clif->message(fd, atcmd_output);
			return false;
		}
		if (!pc_isridingwug(sd)) {
			clif->message(sd->fd,msg_fd(fd, MSGTBL_MOUNTED_WARG)); // You have mounted your Warg.
			pc->setridingwug(sd, true);
		} else {
			clif->message(sd->fd,msg_fd(fd, MSGTBL_RELEASED_WARG)); // You have released your Warg.
			pc->setridingwug(sd, false);
		}
		return true;
	}
	if ((sd->job & MAPID_THIRDMASK) == MAPID_MECHANIC) {
		int mtype = MADO_ROBOT;
		if (!*message)
			sscanf(message, "%d", &mtype);
		if (mtype < MADO_ROBOT || mtype >= MADO_MAX) {
			clif->message(fd, msg_fd(fd, MSGTBL_MOUNT_ENTER_VALID_MADO_TYPE)); // Please enter a valid madogear type.
			return false;
		}
		if (!pc_ismadogear(sd)) {
			clif->message(sd->fd,msg_fd(fd, MSGTBL_MOUNTED_MADO_GEAR)); // You have mounted your Mado Gear.
			pc->setmadogear(sd, true, (enum mado_type)mtype);
		} else {
			clif->message(sd->fd,msg_fd(fd, MSGTBL_RELEASED_MADO_GEAR)); // You have released your Mado Gear.
			pc->setmadogear(sd, false, (enum mado_type)mtype);
		}
		return true;
	}
	if ((sd->job & MAPID_BASEMASK) == MAPID_SWORDMAN && (sd->job & JOBL_2) != 0) {
		if (!pc_isridingpeco(sd)) { // if actually no peco
			if (!pc->checkskill(sd, KN_RIDING)) {
				snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REQUIREMENT_FOR_MOUNT), skill->get_desc(KN_RIDING)); // You need %s to mount!
				clif->message(fd, atcmd_output);
				return false;
			}
			pc->setridingpeco(sd, true);
			clif->message(fd, msg_fd(fd, MSGTBL_MOUNTED_PECOPECO)); // You have mounted a Peco Peco.
		} else {//Dismount
			pc->setridingpeco(sd, false);
			clif->message(fd, msg_fd(fd, MSGTBL_PECO_RELEASED)); // You have released your Peco Peco.
		}
		return true;
	}
	clif->message(fd, msg_fd(fd, MSGTBL_NO_MOUNT_FOR_CLASS)); // Your class can't mount!
	return false;
}

/*==========================================
 *Spy Commands by Syrus22
 *------------------------------------------*/
ACMD(guildspy)
{
	char guild_name[NAME_LENGTH];
	struct guild *g;

	memset(guild_name, '\0', sizeof(guild_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!map->enable_spy)
	{
		clif->message(fd, msg_fd(fd, MSGTBL_SPY_SUPPORT_DISABLED)); // The mapserver has spy command support disabled.
		return false;
	}
	if (!*message || sscanf(message, "%23[^\n]", guild_name) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_GUILD_NAME_OR_ID)); // Please enter a guild name/ID (usage: @guildspy <guild_name/ID>).
		return false;
	}

	if ((g = guild->searchname(guild_name)) != NULL || // name first to avoid error when name begin with a number
	    (g = guild->search(atoi(message))) != NULL) {
		if (sd->guildspy == g->guild_id) {
			sd->guildspy = 0;
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_GUILDSPY_OFF), g->name); // No longer spying on the %s guild.
			clif->message(fd, atcmd_output);
		} else {
			sd->guildspy = g->guild_id;
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_GUILDSPY_ON), g->name); // Spying on the %s guild.
			clif->message(fd, atcmd_output);
		}
	} else {
		clif->message(fd, msg_fd(fd, MSGTBL_RECALL_INVALID_GUILD_NAME)); // Incorrect name/ID, or no one from the specified guild is online.
		return false;
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(partyspy)
{
	char party_name[NAME_LENGTH];
	struct party_data *p;

	memset(party_name, '\0', sizeof(party_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!map->enable_spy)
	{
		clif->message(fd, msg_fd(fd, MSGTBL_SPY_SUPPORT_DISABLED)); // The mapserver has spy command support disabled.
		return false;
	}

	if (!*message || sscanf(message, "%23[^\n]", party_name) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_PARTY_NAME_OR_ID)); // Please enter a party name/ID (usage: @partyspy <party_name/ID>).
		return false;
	}

	if ((p = party->searchname(party_name)) != NULL || // name first to avoid error when name begin with a number
	    (p = party->search(atoi(message))) != NULL) {
		if (sd->partyspy == p->party.party_id) {
			sd->partyspy = 0;
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_PARTYSPY_OFF), p->party.name); // No longer spying on the %s party.
			clif->message(fd, atcmd_output);
		} else {
			sd->partyspy = p->party.party_id;
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_PARTYSPY_ON), p->party.name); // Spying on the %s party.
			clif->message(fd, atcmd_output);
		}
	} else {
		clif->message(fd, msg_fd(fd, MSGTBL_RECALL_INVALID_PARTY_NAME)); // Incorrect name/ID, or no one from the specified party is online.
		return false;
	}

	return true;
}

/*==========================================
 * @repairall [Valaris]
 *------------------------------------------*/
ACMD(repairall)
{
	int count = 0;
	for (int i = 0; i < sd->status.inventorySize; i++) {
		if (sd->status.inventory[i].card[0] == CARD0_PET)
			continue;
		if (sd->status.inventory[i].nameid && (sd->status.inventory[i].attribute & ATTR_BROKEN) != 0) {
			sd->status.inventory[i].attribute |= ATTR_BROKEN;
			sd->status.inventory[i].attribute ^= ATTR_BROKEN;
			clif->produce_effect(sd, 0, sd->status.inventory[i].nameid);
			count++;
		}
	}

	if (count > 0) {
		clif->misceffect(&sd->bl, 3);
		clif->equipList(sd);
		clif->message(fd, msg_fd(fd, MSGTBL_ALL_ITEMS_REPAIRED)); // All items have been repaired.
	} else {
		clif->message(fd, msg_fd(fd, MSGTBL_NOTHING_TO_REPAIR)); // No item need to be repaired.
		return false;
	}

	return true;
}

/*==========================================
 * @nuke [Valaris]
 *------------------------------------------*/
ACMD(nuke)
{
	struct map_session_data *pl_sd;

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_PLAYER_NAME_FOR_NUKE)); // Please enter a player name (usage: @nuke <char name>).
		return false;
	}

	if ((pl_sd = map->nick2sd(atcmd_player_name, true)) != NULL) {
		if (pc_get_group_level(sd) >= pc_get_group_level(pl_sd)) { // you can kill only lower or same GM level
			skill->castend_nodamage_id(&pl_sd->bl, &pl_sd->bl, NPC_SELFDESTRUCTION, 99, timer->gettick(), 0);
			clif->message(fd, msg_fd(fd, MSGTBL_PLAYER_NUKED)); // Player has been nuked!
		} else {
			clif->message(fd, msg_fd(fd, MSGTBL_GM_LEVEL_UNAUTHORIZED)); // Your GM level don't authorize you to do this action on this player.
			return false;
		}
	} else {
		clif->message(fd, msg_fd(fd, MSGTBL_CHARACTER_NOT_FOUND)); // Character not found.
		return false;
	}

	return true;
}

/*==========================================
 * @tonpc
 *------------------------------------------*/
ACMD(tonpc)
{
	char npcname[NAME_LENGTH+1];
	struct npc_data *nd;

	memset(npcname, 0, sizeof(npcname));

	if (!*message || sscanf(message, "%23[^\n]", npcname) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_NPC_NAME_FOR_TONPC)); // Please enter a NPC name (usage: @tonpc <NPC_name>).
		return false;
	}

	if ((nd = npc->name2id(npcname)) != NULL) {
		if (nd->bl.m != -1 && pc->setpos(sd, map_id2index(nd->bl.m), nd->bl.x, nd->bl.y, CLR_TELEPORT) == 0)
			clif->message(fd, msg_fd(fd, MSGTBL_WARPED)); // Warped.
		else
			return false;
	} else {
		clif->message(fd, msg_fd(fd, MSGTBL_NPC_DOESNT_EXIST)); // This NPC doesn't exist.
		return false;
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(shownpc)
{
	char NPCname[NAME_LENGTH+1];

	memset(NPCname, '\0', sizeof(NPCname));

	if (!*message || sscanf(message, "%23[^\n]", NPCname) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_NPC_NAME_FOR_ENABLENPC)); // Please enter a NPC name (usage: @enablenpc <NPC_name>).
		return false;
	}

	if (npc->name2id(NPCname) != NULL) {
		npc->enable(NPCname, 1);
		clif->message(fd, msg_fd(fd, MSGTBL_NPC_ENABLED)); // Npc Enabled.
	} else {
		clif->message(fd, msg_fd(fd, MSGTBL_NPC_DOESNT_EXIST)); // This NPC doesn't exist.
		return false;
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(hidenpc)
{
	char NPCname[NAME_LENGTH+1];

	memset(NPCname, '\0', sizeof(NPCname));

	if (!*message || sscanf(message, "%23[^\n]", NPCname) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_NPC_NAME_FOR_HIDENPC)); // Please enter a NPC name (usage: @hidenpc <NPC_name>).
		return false;
	}

	if (npc->name2id(NPCname) == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_NPC_DOESNT_EXIST)); // This NPC doesn't exist.
		return false;
	}

	npc->enable(NPCname, 0);
	clif->message(fd, msg_fd(fd, MSGTBL_NPC_DISABLED)); // Npc Disabled.
	return true;
}

ACMD(loadnpc)
{
	FILE *fp;

	if (!*message) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_SCRIPT_FOR_LOADNPC)); // Please enter a script file name (usage: @loadnpc <file name>).
		return false;
	}

	// check if script file exists
	if ((fp = fopen(message, "r")) == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_COULD_NOT_LOAD_SCRIPT));
		return false;
	}
	fclose(fp);

	// add to list of script sources and run it
	npc->addsrcfile(message);
	npc->parsesrcfile(message,true);
	npc->motd = npc->name2id("HerculesMOTD");
	npc->read_event_script();

	clif->message(fd, msg_fd(fd, MSGTBL_SCRIPT_LOADED));

	return true;
}

/**
 * Unloads a specific NPC.
 *
 * @code{.herc}
 *	@unloadnpc <NPC_name> {<flag>}
 * @endcode
 *
 **/
ACMD(unloadnpc)
{
	char npc_name[NAME_LENGTH + 1] = {'\0'};
	int flag = 1;

	if (*message == '\0' || sscanf(message, "%24s %1d", npc_name, &flag) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_NPC_NAME_FOR_UNLOADNPC)); /// Please enter a NPC name (Usage: @unloadnpc <NPC_name> {<flag>}).
		return false;
	}

	struct npc_data *nd = npc->name2id(npc_name);

	if (nd == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_NPC_DOESNT_EXIST)); /// This NPC doesn't exist.
		return false;
	}

	npc->unload_duplicates(nd, (flag != 0));
	npc->unload(nd, true, (flag != 0));
	npc->motd = npc->name2id("HerculesMOTD");
	npc->read_event_script();
	clif->message(fd, msg_fd(fd, MSGTBL_NPC_DISABLED)); /// Npc Disabled.
	return true;
}

/**
 * Unloads a script file and reloads it.
 * Note: Be aware that some changes made by NPC are not reverted on unload. See doc/atcommands.txt for details.
 *
 * @code{.herc}
 *	@reloadnpc <path> {<flag>}
 * @endcode
 *
 **/
ACMD(reloadnpc)
{
	char format[20];

	snprintf(format, sizeof(format), "%%%ds %%1d", MAX_DIR_PATH);

	char file_path[MAX_DIR_PATH + 1] = {'\0'};
	int flag = 1;

	if (*message == '\0' || (sscanf(message, format, file_path, &flag) < 1)) {
		clif->message(fd, msg_fd(fd, MSGTBL_RELOADNPC_USAGE)); /// Usage: @reloadnpc <path> {<flag>}
		return false;
	}

	if (!exists(file_path)) {
		clif->message(fd, msg_fd(fd, MSGTBL_UNLOADNPCFILE_NOT_FOUND)); /// File not found.
		return false;
	}

	if (!is_file(file_path)) {
		clif->message(fd, msg_fd(fd, MSGTBL_NOT_A_FILE)); /// Not a file.
		return false;
	}

	FILE *fp = fopen(file_path, "r");

	if (fp == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_CANT_OPEN_FILE)); /// Can't open file.
		return false;
	}

	fclose(fp);

	if (!npc->unloadfile(file_path, (flag != 0))) {
		clif->message(fd, msg_fd(fd, MSGTBL_RELOADNPC_NOT_UNLOADED)); /// Script could not be unloaded.
		return false;
	}

	clif->message(fd, msg_fd(fd, MSGTBL_UNLOADNPCFILE_SUCCESS)); /// File unloaded. Be aware that...
	npc->addsrcfile(file_path);
	npc->parsesrcfile(file_path, true);
	npc->motd = npc->name2id("HerculesMOTD");
	npc->read_event_script();
	clif->message(fd, msg_fd(fd, MSGTBL_SCRIPT_LOADED)); /// Script loaded.
	return true;
}

/*==========================================
 * time in txt for time command (by [Yor])
 *------------------------------------------*/
static char *txt_time(int fd, unsigned int duration)
{
	int days, hours, minutes, seconds;
	static char temp1[CHAT_SIZE_MAX];
	int tlen = 0;

	memset(temp1, '\0', sizeof(temp1));

	days = duration / (60 * 60 * 24);
	duration = duration - (60 * 60 * 24 * days);
	hours = duration / (60 * 60);
	duration = duration - (60 * 60 * hours);
	minutes = duration / 60;
	seconds = duration - (60 * minutes);

	if (days == 1)
		tlen += sprintf(tlen + temp1, msg_fd(fd, MSGTBL_DAY), days); // %d day
	else if (days > 1)
		tlen += sprintf(tlen + temp1, msg_fd(fd, MSGTBL_DAYS), days); // %d days
	if (hours == 1)
		tlen += sprintf(tlen + temp1, msg_fd(fd, MSGTBL_HOUR), hours); // %d hour
	else if (hours > 1)
		tlen += sprintf(tlen + temp1, msg_fd(fd, MSGTBL_HOURS), hours); // %d hours
	if (minutes < 2)
		tlen += sprintf(tlen + temp1, msg_fd(fd, MSGTBL_MINUTE), minutes); // %d minute
	else
		tlen += sprintf(tlen + temp1, msg_fd(fd, MSGTBL_MINUTES), minutes); // %d minutes
	if (seconds == 1)
		sprintf(tlen + temp1, msg_fd(fd, MSGTBL_SECOND), seconds); // and %d second
	else if (seconds > 1)
		sprintf(tlen + temp1, msg_fd(fd, MSGTBL_SENDS), seconds); // and %d seconds

	return temp1;
}

/*==========================================
 * @time/@date/@serverdate/@servertime: Display the date/time of the server (by [Yor]
 * Calculation management of GM modification (@day/@night GM commands) is done
 *------------------------------------------*/
ACMD(servertime)
{
	time_t time_server;  // variable for number of seconds (used with time() function)
	struct tm *datetime; // variable for time in structure ->tm_mday, ->tm_sec, ...
	char temp[CHAT_SIZE_MAX];

	memset(temp, '\0', sizeof(temp));

	time(&time_server);  // get time in seconds since 1/1/1970
	datetime = localtime(&time_server); // convert seconds in structure
	// like sprintf, but only for date/time (Sunday, November 02 2003 15:12:52)
	strftime(temp, sizeof(temp)-1, msg_fd(fd, MSGTBL_SERVER_TIME), datetime); // Server time (normal time): %A, %B %d %Y %X.
	clif->message(fd, temp);

	if (pc->day_timer_tid != INVALID_TIMER && pc->night_timer_tid != INVALID_TIMER) {
		const struct TimerData * timer_data = timer->get(pc->night_timer_tid);
		const struct TimerData * timer_data2 = timer->get(pc->day_timer_tid);

		if (map->night_flag == 0) {
			sprintf(temp, msg_fd(fd, MSGTBL_GAMETIME_DAY_TIME), // Game time: The game is actually in daylight for %s.
			        txt_time(fd,(unsigned int)(DIFF_TICK(timer_data->tick,timer->gettick())/1000)));
			clif->message(fd, temp);
			if (DIFF_TICK(timer_data->tick, timer_data2->tick) > 0)
				sprintf(temp, msg_fd(fd, MSGTBL_GAMETIME_FUTURE_NIGHT_TIME), // Game time: After, the game will be in night for %s.
				        txt_time(fd,(unsigned int)(DIFF_TICK(timer_data->interval,DIFF_TICK(timer_data->tick,timer_data2->tick)) / 1000)));
			else
				sprintf(temp, msg_fd(fd, MSGTBL_GAMETIME_FUTURE_NIGHT_TIME), // Game time: After, the game will be in night for %s.
				        txt_time(fd,(unsigned int)(DIFF_TICK(timer_data2->tick,timer_data->tick)/1000)));
			clif->message(fd, temp);
		} else {
			sprintf(temp, msg_fd(fd, MSGTBL_GAMETIME_NIGHT_TIME), // Game time: The game is actually in night for %s.
			        txt_time(fd,(unsigned int)(DIFF_TICK(timer_data2->tick,timer->gettick()) / 1000)));
			clif->message(fd, temp);
			if (DIFF_TICK(timer_data2->tick,timer_data->tick) > 0)
				sprintf(temp, msg_fd(fd, MSGTBL_GAMETIME_FUTURE_DAY_TIME), // Game time: After, the game will be in daylight for %s.
				        txt_time(fd,(unsigned int)((timer_data2->interval - DIFF_TICK(timer_data2->tick, timer_data->tick)) / 1000)));
			else
				sprintf(temp, msg_fd(fd, MSGTBL_GAMETIME_FUTURE_DAY_TIME), // Game time: After, the game will be in daylight for %s.
				        txt_time(fd,(unsigned int)(DIFF_TICK(timer_data->tick, timer_data2->tick) / 1000)));
			clif->message(fd, temp);
		}
		sprintf(temp, msg_fd(fd, MSGTBL_GAMETIME_DAY_CYCLE), txt_time(fd,timer_data2->interval / 1000)); // Game time: A day cycle has a normal duration of %s.
		clif->message(fd, temp);
	} else {
		if (map->night_flag == 0)
			clif->message(fd, msg_fd(fd, MSGTBL_GAMETIME_ALWAYS_DAY)); // Game time: The game is in permanent daylight.
		else
			clif->message(fd, msg_fd(fd, MSGTBL_GAMETIME_ALWAYS_NIGHT)); // Game time: The game is in permanent night.
	}

	return true;
}

//Added by Coltaro
//We're using this function here instead of using time_t so that it only counts player's jail time when he/she's online (and since the idea is to reduce the amount of minutes one by one in status->change_timer...).
//Well, using time_t could still work but for some reason that looks like more coding x_x
static void get_jail_time(int jailtime, int *year, int *month, int *day, int *hour, int *minute)
{
	const int factor_year = 518400; //12*30*24*60 = 518400
	const int factor_month = 43200; //30*24*60 = 43200
	const int factor_day = 1440; //24*60 = 1440
	const int factor_hour = 60;

	nullpo_retv(year);
	nullpo_retv(month);
	nullpo_retv(day);
	nullpo_retv(hour);
	nullpo_retv(minute);
	*year = jailtime/factor_year;
	jailtime -= *year*factor_year;
	*month = jailtime/factor_month;
	jailtime -= *month*factor_month;
	*day = jailtime/factor_day;
	jailtime -= *day*factor_day;
	*hour = jailtime/factor_hour;
	jailtime -= *hour*factor_hour;
	*minute = jailtime;

	*year = *year > 0? *year : 0;
	*month = *month > 0? *month : 0;
	*day = *day > 0? *day : 0;
	*hour = *hour > 0? *hour : 0;
	*minute = *minute > 0? *minute : 0;
	return;
}

/*==========================================
 * @jail <char_name> by [Yor]
 * Special warp! No check with nowarp and nowarpto flag
 *------------------------------------------*/
ACMD(jail)
{
	struct map_session_data *pl_sd;
	int x, y;
	unsigned short m_index;

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_NAME_FOR_JAIL)); // Please enter a player name (usage: @jail <char_name>).
		return false;
	}

	if ((pl_sd = map->nick2sd(atcmd_player_name, true)) == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_CHARACTER_NOT_FOUND)); // Character not found.
		return false;
	}

	if (pc_get_group_level(sd) < pc_get_group_level(pl_sd)) {
		// you can jail only lower or same GM
		clif->message(fd, msg_fd(fd, MSGTBL_GM_LEVEL_UNAUTHORIZED)); // Your GM level don't authorize you to do this action on this player.
		return false;
	}

	if (pl_sd->sc.data[SC_JAILED])
	{
		clif->message(fd, msg_fd(fd, MSGTBL_WARPED_TO_JAIL)); // Player warped in jails.
		return false;
	}

	switch(rnd() % 2) { //Jail Locations
		case 0:
			m_index = mapindex->name2id(MAP_JAIL);
			x = 24;
			y = 75;
			break;
		default:
			m_index = mapindex->name2id(MAP_JAIL);
			x = 49;
			y = 75;
			break;
	}

	//Duration of INT_MAX to specify infinity.
	sc_start4(NULL, &pl_sd->bl, SC_JAILED,100, INT_MAX, m_index, x, y, 1000, 0);
	clif->message(pl_sd->fd, msg_fd(fd, MSGTBL_JAILED_BY_GM)); // You have been jailed by a GM.
	clif->message(fd, msg_fd(fd, MSGTBL_WARPED_TO_JAIL)); // Player warped in jails.
	return true;
}

/*==========================================
 * @unjail/@discharge <char_name> by [Yor]
 * Special warp! No check with nowarp and nowarpto flag
 *------------------------------------------*/
ACMD(unjail)
{
	struct map_session_data *pl_sd;

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_NAME_FOR_UNJAIL)); // Please enter a player name (usage: @unjail/@discharge <char_name>).
		return false;
	}

	if ((pl_sd = map->nick2sd(atcmd_player_name, true)) == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_CHARACTER_NOT_FOUND)); // Character not found.
		return false;
	}

	if (pc_get_group_level(sd) < pc_get_group_level(pl_sd)) { // you can jail only lower or same GM

		clif->message(fd, msg_fd(fd, MSGTBL_GM_LEVEL_UNAUTHORIZED)); // Your GM level don't authorize you to do this action on this player.
		return false;
	}

	if (!pl_sd->sc.data[SC_JAILED])
	{
		clif->message(fd, msg_fd(fd, MSGTBL_PLAYER_NOT_IN_JAIL)); // This player is not in jails.
		return false;
	}

	//Reset jail time to 1 sec.
	sc_start(NULL, &pl_sd->bl, SC_JAILED, 100, 1, 1000, 0);
	clif->message(pl_sd->fd, msg_fd(fd, MSGTBL_UNJAILED_BY_GM)); // A GM has discharged you from jail.
	clif->message(fd, msg_fd(fd, MSGTBL_UNJAILED)); // Player unjailed.
	return true;
}

ACMD(jailfor)
{
	struct map_session_data *pl_sd = NULL;
	int year, month, day, hour, minute;
	char * modif_p;
	int jailtime = 0,x,y;
	short m_index = 0;

	if (!*message || sscanf(message, "%255s %23[^\n]",atcmd_output,atcmd_player_name) < 2) {
		clif->message(fd, msg_fd(fd, MSGTBL_JAILFOR_USAGE)); //Usage: @jailfor <time> <character name>
		return false;
	}

	atcmd_output[sizeof(atcmd_output)-1] = '\0';

	modif_p = atcmd_output;
	year = month = day = hour = minute = 0;
	while (modif_p[0] != '\0') {
		int value = atoi(modif_p);
		if (value == 0)
			modif_p++;
		else {
			if (modif_p[0] == '-' || modif_p[0] == '+')
				modif_p++;
			while (modif_p[0] >= '0' && modif_p[0] <= '9')
				modif_p++;
			if (modif_p[0] == 'n') {
				minute = value;
				modif_p++;
			} else if (modif_p[0] == 'm' && modif_p[1] == 'n') {
				minute = value;
				modif_p = modif_p + 2;
			} else if (modif_p[0] == 'h') {
				hour = value;
				modif_p++;
			} else if (modif_p[0] == 'd' || modif_p[0] == 'j') {
				day = value;
				modif_p++;
			} else if (modif_p[0] == 'm') {
				month = value;
				modif_p++;
			} else if (modif_p[0] == 'y' || modif_p[0] == 'a') {
				year = value;
				modif_p++;
			} else if (modif_p[0] != '\0') {
				modif_p++;
			}
		}
	}

	if (year == 0 && month == 0 && day == 0 && hour == 0 && minute == 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_TIME_FOR_JAIL)); // Invalid time for jail command.
		return false;
	}

	if ((pl_sd = map->nick2sd(atcmd_player_name, true)) == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_CHARACTER_NOT_FOUND)); // Character not found.
		return false;
	}

	if (pc_get_group_level(pl_sd) > pc_get_group_level(sd)) {
		clif->message(fd, msg_fd(fd, MSGTBL_GM_LEVEL_UNAUTHORIZED)); // Your GM level don't authorize you to do this action on this player.
		return false;
	}

	jailtime = year*12*30*24*60 + month*30*24*60 + day*24*60 + hour*60 + minute; //In minutes

	if(jailtime==0) {
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_TIME_FOR_JAIL)); // Invalid time for jail command.
		return false;
	}

	//Added by Coltaro
	if (pl_sd->sc.data[SC_JAILED] && pl_sd->sc.data[SC_JAILED]->val1 != INT_MAX) {
		//Update the player's jail time
		jailtime += pl_sd->sc.data[SC_JAILED]->val1;
		if (jailtime <= 0) {
			jailtime = 0;
			clif->message(pl_sd->fd, msg_fd(fd, MSGTBL_UNJAILED_BY_GM)); // GM has discharge you.
			clif->message(fd, msg_fd(fd, MSGTBL_UNJAILED)); // Player unjailed
		} else {
			atcommand->get_jail_time(jailtime,&year,&month,&day,&hour,&minute);
			snprintf(atcmd_output, sizeof(atcmd_output),msg_fd(fd, MSGTBL_JAILFOR_TIME),msg_fd(fd, MSGTBL_YOU_ARE_NOW),year,month,day,hour,minute); //%s in jail for %d years, %d months, %d days, %d hours and %d minutes
			clif->message(pl_sd->fd, atcmd_output);
			snprintf(atcmd_output, sizeof(atcmd_output),msg_fd(fd, MSGTBL_JAILFOR_TIME),msg_fd(fd, MSGTBL_PLAYER_IS_NOW),year,month,day,hour,minute); //This player is now in jail for %d years, %d months, %d days, %d hours and %d minutes
			clif->message(fd, atcmd_output);
		}
	} else if (jailtime < 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_TIME_FOR_JAIL));
		return false;
	}

	//Jail locations, add more as you wish.
	switch(rnd()%2)
	{
		case 1: //Jail #1
			m_index = mapindex->name2id(MAP_JAIL);
			x = 49; y = 75;
			break;
		default: //Default Jail
			m_index = mapindex->name2id(MAP_JAIL);
			x = 24; y = 75;
			break;
	}

	sc_start4(NULL, &pl_sd->bl, SC_JAILED, 100, jailtime, m_index, x, y, jailtime ? 60000 : 1000, 0); // jailtime = 0: Time was reset to 0. Wait 1 second to warp player out (since it's done in status->change_timer).
	return true;
}

//By Coltaro
ACMD(jailtime)
{
	int year, month, day, hour, minute;

	if (!sd->sc.data[SC_JAILED]) {
		clif->message(fd, msg_fd(fd, MSGTBL_NOT_IN_JAIL)); // You are not in jail.
		return false;
	}

	if (sd->sc.data[SC_JAILED]->val1 == INT_MAX) {
		clif->message(fd, msg_fd(fd, MSGTBL_JAILED_INDEFINITELY)); // You have been jailed indefinitely.
		return true;
	}

	if (sd->sc.data[SC_JAILED]->val1 <= 0) { // Was not jailed with @jailfor (maybe @jail? or warped there? or got recalled?)
		clif->message(fd, msg_fd(fd, MSGTBL_JAILED_UNKNOWN_TIME)); // You have been jailed for an unknown amount of time.
		return false;
	}

	//Get remaining jail time
	atcommand->get_jail_time(sd->sc.data[SC_JAILED]->val1,&year,&month,&day,&hour,&minute);
	snprintf(atcmd_output, sizeof(atcmd_output),msg_fd(fd, MSGTBL_JAILFOR_TIME),msg_fd(fd, MSGTBL_WILL_REMAIN),year,month,day,hour,minute); // You will remain in jail for %d years, %d months, %d days, %d hours and %d minutes

	clif->message(fd, atcmd_output);

	return true;
}

/*==========================================
 * @disguise <mob_id> by [Valaris] (simplified by [Yor])
 *------------------------------------------*/
ACMD(disguise)
{
	int id = 0;

	if (!*message) {
		clif->message(fd, msg_fd(fd, MSGTBL_DISGUISE_ENTER_TARGET)); // Please enter a Monster/NPC name/ID (usage: @disguise <name/ID>).
		return false;
	}

	if ((id = atoi(message)) > 0) {
		//Acquired an ID
		if (!mob->db_checkid(id) && !npc->db_checkid(id))
			id = 0; //Invalid id for either mobs or npcs.
	} else {
		//Acquired a Name
		if ((id = mob->db_searchname(message)) == 0)
		{
			struct npc_data* nd = npc->name2id(message);
			if (nd != NULL)
				id = nd->class_;
		}
	}

	if (id == 0)
	{
		clif->message(fd, msg_fd(fd, MSGTBL_DISGUISE_INVALID_ID)); // Invalid Monster/NPC name/ID specified.
		return false;
	}

	if (pc_hasmount(sd)) {
		clif->message(fd, msg_fd(fd, MSGTBL_NOT_DISGUISE_MOUNTED)); // Character cannot be disguised while mounted.
		return false;
	}

	if (sd->sc.data[SC_MONSTER_TRANSFORM] != NULL || sd->sc.data[SC_ACTIVE_MONSTER_TRANSFORM] != NULL)
	{
		clif->message(fd, msg_fd(fd, MSGTBL_NOT_DISGUISE_WHILE_TRANSFORMED)); // Character cannot be disguised while in monster form.
		return false;
	}

	pc->disguise(sd, id);
	clif->message(fd, msg_fd(fd, MSGTBL_DISGUISE_ON)); // Disguise applied.

	return true;
}

/*==========================================
 * DisguiseAll
 *------------------------------------------*/
ACMD(disguiseall)
{
	int mob_id=0;
	struct map_session_data *pl_sd;
	struct s_mapiterator* iter;

	if (!*message) {
		clif->message(fd, msg_fd(fd, MSGTBL_DISGUISE_ALL_ENTER_TARGET)); // Please enter a Monster/NPC name/ID (usage: @disguiseall <name/ID>).
		return false;
	}

	if ((mob_id = mob->db_searchname(message)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = atoi(message);

	if (!mob->db_checkid(mob_id) && !npc->db_checkid(mob_id)) { //if mob or npc...
		clif->message(fd, msg_fd(fd, MSGTBL_DISGUISE_INVALID_ID)); // Monster/NPC name/id not found.
		return false;
	}

	iter = mapit_getallusers();
	for (pl_sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); pl_sd = BL_UCAST(BL_PC, mapit->next(iter)))
		pc->disguise(pl_sd, mob_id);
	mapit->free(iter);

	clif->message(fd, msg_fd(fd, MSGTBL_DISGUISE_ON)); // Disguise applied.
	return true;
}

/*==========================================
 * DisguiseGuild
 *------------------------------------------*/
ACMD(disguiseguild)
{
	int id = 0, i;
	char monster[NAME_LENGTH], guild_name[NAME_LENGTH];
	struct guild *g;

	memset(monster, '\0', sizeof(monster));
	memset(guild_name, '\0', sizeof(guild_name));

	if (!*message || sscanf(message, "%23[^,], %23[^\r\n]", monster, guild_name) < 2) {
		clif->message(fd, msg_fd(fd, MSGTBL_DISGUISE_GUILD_ENTER_TARGET)); // Please enter a mob name/ID and guild name/ID (usage: @disguiseguild <mob name/ID>, <guild name/ID>).
		return false;
	}

	if( (id = atoi(monster)) > 0 ) {
		if( !mob->db_checkid(id) && !npc->db_checkid(id) )
			id = 0;
	} else {
		if( (id = mob->db_searchname(monster)) == 0 ) {
			struct npc_data* nd = npc->name2id(monster);
			if( nd != NULL )
				id = nd->class_;
		}
	}

	if( id == 0 ) {
		clif->message(fd, msg_fd(fd, MSGTBL_DISGUISE_INVALID_ID)); // Monster/NPC name/id hasn't been found.
		return false;
	}

	if( (g = guild->searchname(guild_name)) == NULL && (g = guild->search(atoi(guild_name))) == NULL ) {
		clif->message(fd, msg_fd(fd, MSGTBL_RECALL_INVALID_GUILD_NAME)); // Incorrect name/ID, or no one from the guild is online.
		return false;
	}

	for (i = 0; i < g->max_member; i++) {
		struct map_session_data *pl_sd = g->member[i].sd;
		if (pl_sd && !pc_hasmount(pl_sd))
			pc->disguise(pl_sd, id);
	}

	clif->message(fd, msg_fd(fd, MSGTBL_DISGUISE_ON)); // Disguise applied.
	return true;
}

/*==========================================
 * @undisguise by [Yor]
 *------------------------------------------*/
ACMD(undisguise)
{
	if (sd->disguise != -1) {
		pc->disguise(sd, -1);
		clif->message(fd, msg_fd(fd, MSGTBL_DISGUISE_OFF)); // Disguise removed.
	} else {
		clif->message(fd, msg_fd(fd, MSGTBL_DISGUISE_NOT_DISGUISE)); // You're not disguised.
		return false;
	}

	return true;
}

/*==========================================
 * UndisguiseAll
 *------------------------------------------*/
ACMD(undisguiseall)
{
	struct map_session_data *pl_sd;
	struct s_mapiterator* iter;

	iter = mapit_getallusers();
	for (pl_sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); pl_sd = BL_UCAST(BL_PC, mapit->next(iter)))
		if( pl_sd->disguise != -1 )
			pc->disguise(pl_sd, -1);
	mapit->free(iter);

	clif->message(fd, msg_fd(fd, MSGTBL_DISGUISE_OFF)); // Disguise removed.

	return true;
}

/*==========================================
 * UndisguiseGuild
 *------------------------------------------*/
ACMD(undisguiseguild)
{
	char guild_name[NAME_LENGTH];
	struct guild *g;
	int i;

	memset(guild_name, '\0', sizeof(guild_name));

	if (!*message || sscanf(message, "%23[^\n]", guild_name) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_UNDISGUISE_GUILD_ENTER_TARGET)); // Please enter guild name/ID (usage: @undisguiseguild <guild name/ID>).
		return false;
	}

	if ((g = guild->searchname(guild_name)) == NULL && (g = guild->search(atoi(message))) == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_RECALL_INVALID_GUILD_NAME)); // Incorrect name/ID, or no one from the guild is online.
		return false;
	}

	for (i = 0; i < g->max_member; i++) {
		struct map_session_data *pl_sd = g->member[i].sd;
		if (pl_sd && pl_sd->disguise != -1)
			pc->disguise(pl_sd, -1);
	}

	clif->message(fd, msg_fd(fd, MSGTBL_DISGUISE_OFF)); // Disguise removed.

	return true;
}

/*==========================================
 * @exp by [Skotlex]
 *------------------------------------------*/
ACMD(exp)
{
	double percentb = 0.0, percentj = 0.0;
	uint64 nextb, nextj;

	nextb = pc->nextbaseexp(sd);
	if (nextb != 0)
		percentb = sd->status.base_exp * 100.0 / nextb;

	nextj = pc->nextjobexp(sd);
	if (nextj != 0)
		percentj = sd->status.job_exp * 100.0 / nextj;

	sprintf(atcmd_output, msg_fd(fd, MSGTBL_EXP_INFO), sd->status.base_level, percentb, sd->status.job_level, percentj); // Base Level: %d (%.3f%%) | Job Level: %d (%.3f%%)
	clif->message(fd, atcmd_output);
	return true;
}

/*==========================================
 * @broadcast by [Valaris]
 *------------------------------------------*/
ACMD(broadcast)
{
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!*message) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_BROADCAST_MSG)); // Please enter a message (usage: @broadcast <message>).
		return false;
	}

	snprintf(atcmd_output, sizeof(atcmd_output), "%s: %s", sd->status.name, message);
	clif->broadcast(NULL, atcmd_output, (int)strlen(atcmd_output) + 1, BC_DEFAULT, ALL_CLIENT);

	return true;
}

/*==========================================
 * @localbroadcast by [Valaris]
 *------------------------------------------*/
ACMD(localbroadcast)
{
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!*message) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_LOCAL_BROADCAST_MSG)); // Please enter a message (usage: @localbroadcast <message>).
		return false;
	}

	snprintf(atcmd_output, sizeof(atcmd_output), "%s: %s", sd->status.name, message);

	clif->broadcast(&sd->bl, atcmd_output, (int)strlen(atcmd_output) + 1, BC_DEFAULT, ALL_SAMEMAP);

	return true;
}

/*==========================================
 * @email <actual@email> <new@email> by [Yor]
 *------------------------------------------*/
ACMD(email)
{
	char actual_email[100];
	char new_email[100];

	memset(actual_email, '\0', sizeof(actual_email));
	memset(new_email, '\0', sizeof(new_email));

	if (!*message || sscanf(message, "%99s %99s", actual_email, new_email) < 2) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_EMAIL_ADDRESSES)); // Please enter two e-mail addresses (usage: @email <current@email> <new@email>).
		return false;
	}

	if (e_mail_check(actual_email) == 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_EMAIL_INVALID_EMAIL)); // Invalid e-mail. If your email hasn't been set, use a@a.com.
		return false;
	} else if (e_mail_check(new_email) == 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_EMAIL_INVALID_NEW_EMAIL)); // Invalid new email. Please enter a real e-mail address.
		return false;
	} else if (strcmpi(new_email, "a@a.com") == 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_EMAIL_MUST_BE_REAL)); // New email must be a real e-mail address.
		return false;
	} else if (strcmpi(actual_email, new_email) == 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_EMAIL_MUST_BE_DIFFERENT)); // New e-mail must be different from the current e-mail address.
		return false;
	}

	chrif->changeemail(sd->status.account_id, actual_email, new_email);
	clif->message(fd, msg_fd(fd, MSGTBL_EMAIL_SENT_TO_LOGIN)); // Information sended to login-server via char-server.
	return true;
}

/*==========================================
 *@effect
 *------------------------------------------*/
ACMD(effect)
{
	int type = 0, flag = 0;

	if (!*message || sscanf(message, "%d", &type) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENTER_EFFECT_NUMBER)); // Please enter an effect number (usage: @effect <effect number>).
		return false;
	}

	clif->specialeffect(&sd->bl, type, (send_target)flag);
	clif->message(fd, msg_fd(fd, MSGTBL_EFFECT_CHANGED)); // Your effect has changed.
	return true;
}

/*==========================================
 * @killer by MouseJstr
 * enable killing players even when not in pvp
 *------------------------------------------*/
ACMD(killer)
{
	sd->state.killer = !sd->state.killer;

	if (sd->state.killer)
		clif->message(fd, msg_fd(fd, MSGTBL_KILLER_ON));
	else {
		clif->message(fd, msg_fd(fd, MSGTBL_KILLER_OFF));
		pc_stop_attack(sd);
	}
	return true;
}

/*==========================================
 * @killable by MouseJstr
 * enable other people killing you
 *------------------------------------------*/
ACMD(killable)
{
	sd->state.killable = !sd->state.killable;

	if (sd->state.killable) {
		clif->message(fd, msg_fd(fd, MSGTBL_KILLABLE_ON));
	} else {
		clif->message(fd, msg_fd(fd, MSGTBL_KILLABLE_OFF));
		map->foreachinrange(atcommand->stopattack,&sd->bl, AREA_SIZE, BL_CHAR, sd->bl.id);
	}
	return true;
}

/*==========================================
 * @skillon by MouseJstr
 * turn skills on for the map
 *------------------------------------------*/
ACMD(skillon)
{
	map->list[sd->bl.m].flag.noskill = 0;
	clif->message(fd, msg_fd(fd, MSGTBL_SKILLON));
	return true;
}

/*==========================================
 * @skilloff by MouseJstr
 * Turn skills off on the map
 *------------------------------------------*/
ACMD(skilloff)
{
	map->list[sd->bl.m].flag.noskill = 1;
	clif->message(fd, msg_fd(fd, MSGTBL_SKILLOFF));
	return true;
}

/*==========================================
 * @npcmove by MouseJstr
 * move a npc
 *------------------------------------------*/
ACMD(npcmove)
{
	int x = 0, y = 0, m;
	struct npc_data *nd = 0;

	memset(atcmd_player_name, '\0', sizeof atcmd_player_name);

	if (!*message || sscanf(message, "%12d %12d %23[^\n]", &x, &y, atcmd_player_name) < 3) {
		clif->message(fd, msg_fd(fd, MSGTBL_NPC_MOVE_USAGE)); // Usage: @npcmove <X> <Y> <npc_name>
		return false;
	}

	if ((nd = npc->name2id(atcmd_player_name)) == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_NPC_DOESNT_EXIST)); // This NPC doesn't exist.
		return false;
	}

	if ((m=nd->bl.m) < 0 || nd->bl.prev == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_NPC_NOT_IN_MAP)); // NPC is not in this map.
		return false; //Not on a map.
	}

	x = cap_value(x, 0, map->list[m].xs-1);
	y = cap_value(y, 0, map->list[m].ys-1);
	map->foreachinrange(clif->outsight, &nd->bl, AREA_SIZE, BL_PC, &nd->bl);
	map->moveblock(&nd->bl, x, y, timer->gettick());
	map->foreachinrange(clif->insight, &nd->bl, AREA_SIZE, BL_PC, &nd->bl);
	clif->message(fd, msg_fd(fd, MSGTBL_NPC_MOVED)); // NPC moved.

	return true;
}

/*==========================================
 * @addwarp by MouseJstr
 * Create a new static warp point.
 *------------------------------------------*/
ACMD(addwarp)
{
	char mapname[32], warpname[NAME_LENGTH+1];
	int x,y;
	unsigned short m;
	struct npc_data* nd;

	memset(warpname, '\0', sizeof(warpname));

	if (!*message || sscanf(message, "%31s %12d %12d %23[^\n]", mapname, &x, &y, warpname) < 4) {
		clif->message(fd, msg_fd(fd, MSGTBL_ADDWARP_USAGE)); // Usage: @addwarp <mapname> <X> <Y> <npc name>
		return false;
	}

	m = mapindex->name2id(mapname);
	if( m == 0 )
	{
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_ADDWARP_UNKNOWN_MAP), mapname); // Unknown map '%s'.
		clif->message(fd, atcmd_output);
		return false;
	}

	nd = npc->add_warp(warpname, sd->bl.m, sd->bl.x, sd->bl.y, 2, 2, m, x, y);
	if( nd == NULL )
		return false;

	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_ADDWARP_NPC_CREATED), nd->exname); // New warp NPC '%s' created.
	clif->message(fd, atcmd_output);
	return true;
}

/*==========================================
 * @follow by [MouseJstr]
 * Follow a player .. staying no more then 5 spaces away
 *------------------------------------------*/
ACMD(follow)
{
	struct map_session_data *pl_sd = NULL;

	if (!*message) {
		if (sd->followtarget == -1)
			return false;
		pc->stop_following (sd);
		clif->message(fd, msg_fd(fd, MSGTBL_FOLLOW_MODE_OFF)); // Follow mode OFF.
		return true;
	}

	if ((pl_sd = map->nick2sd(message, true)) == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_CHARACTER_NOT_FOUND)); // Character not found.
		return false;
	}

	if (sd->followtarget == pl_sd->bl.id) {
		pc->stop_following (sd);
		clif->message(fd, msg_fd(fd, MSGTBL_FOLLOW_MODE_OFF)); // Follow mode OFF.
	} else {
		pc->follow(sd, pl_sd->bl.id);
		clif->message(fd, msg_fd(fd, MSGTBL_FOLLOW_MODE_ON)); // Follow mode ON.
	}

	return true;
}

/*==========================================
 * @dropall by [MouseJstr] and [Xantara]
 * Drop all your possession on the ground based on item type
 *------------------------------------------*/
ACMD(dropall)
{
	int type = -1;

	if (message[0] != '\0') {
		type = atoi(message);
		if (!((type >= IT_HEALING && type <= IT_DELAYCONSUME) || type == IT_CASH || type == -1)) {
			clif->message(fd, msg_fd(fd, MSGTBL_DROPALL_USAGE));
			clif->message(fd, msg_fd(fd, MSGTBL_DROPALL_TYPE_LIST));
			return false;
		}
	}

	int count = 0, count_skipped = 0;
	for (int i = 0; i < sd->status.inventorySize; i++) {
		if (sd->status.inventory[i].amount > 0) {
			struct item_data *item_data = itemdb->exists(sd->status.inventory[i].nameid);
			if (item_data == NULL) {
				ShowWarning("Non-existant item %d on dropall list (account_id: %d, char_id: %d)\n", sd->status.inventory[i].nameid, sd->status.account_id, sd->status.char_id);
				continue;
			}

			if (!pc->candrop(sd, &sd->status.inventory[i]))
				continue;

			if (type == -1 || type == item_data->type) {
				if (sd->status.inventory[i].equip != 0)
					pc->unequipitem(sd, i, PCUNEQUIPITEM_RECALC | PCUNEQUIPITEM_FORCE);

				int amount = sd->status.inventory[i].amount;
				if (pc->dropitem(sd, i, amount) != 0)
					count += amount;
				else
					count_skipped += amount;
			}
		}
	}

	sprintf(atcmd_output, msg_fd(fd, MSGTBL_DROPALL_ITEMS_DROPPED), count, count_skipped); // %d items are dropped (%d skipped)!
	clif->message(fd, atcmd_output);
	return true;
}

/*==========================================
 * @storeall by [MouseJstr]
 * Put everything into storage
 *------------------------------------------*/
ACMD(storeall)
{
	if (sd->state.storage_flag != STORAGE_FLAG_NORMAL) {
		//Open storage.
		if (storage->open(sd) == 1) {
			clif->message(fd, msg_fd(fd, MSGTBL_CANNOT_OPEN_STORAGE)); // You currently cannot open your storage.
			return false;
		}
	}

	if (sd->storage.received == false) {
		clif->message(fd, msg_fd(fd, MSGTBL_STORAGE_NOT_LOADED)); // "Storage has not been loaded yet"
		return false;
	}

	for (int i = 0; i < sd->status.inventorySize; i++) {
		if (sd->status.inventory[i].amount) {
			if(sd->status.inventory[i].equip != 0)
				pc->unequipitem(sd, i, PCUNEQUIPITEM_RECALC|PCUNEQUIPITEM_FORCE);
			storage->add(sd,  i, sd->status.inventory[i].amount);
		}
	}
	storage->close(sd);

	clif->message(fd, msg_fd(fd, MSGTBL_ALL_ITEMS_STORED)); // All items stored.
	return true;
}

ACMD(clearstorage)
{
	int i;

	if (sd->state.storage_flag == STORAGE_FLAG_NORMAL) {
		clif->message(fd, msg_fd(fd, MSGTBL_STORAGE_ALREADY_OPEN));
		return false;
	}

	if (sd->storage.received == false) {
		clif->message(fd, msg_fd(fd, MSGTBL_STORAGE_NOT_LOADED)); // "Storage has not been loaded yet"
		return false;
	}

	for (i = 0; i < VECTOR_LENGTH(sd->storage.item); ++i) {
		if (VECTOR_INDEX(sd->storage.item, i).nameid == 0)
			continue; // we skip the already deleted items.

		storage->delitem(sd, i, VECTOR_INDEX(sd->storage.item, i).amount);
	}

	storage->close(sd);

	clif->message(fd, msg_fd(fd, MSGTBL_CLEARSTORAGE_SUCCESS)); // Your storage was cleaned.
	return true;
}

ACMD(cleargstorage)
{
	int i, j;
	struct guild *g;
	struct guild_storage *guild_storage;

	g = sd->guild;

	if (g == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_NOT_IN_A_GUILD));
		return false;
	}

	if (sd->state.storage_flag == STORAGE_FLAG_NORMAL) {
		clif->message(fd, msg_fd(fd, MSGTBL_STORAGE_ALREADY_OPEN));
		return false;
	}

	if (sd->state.storage_flag == STORAGE_FLAG_GUILD) {
		clif->message(fd, msg_fd(fd, MSGTBL_GSTORAGE_ALREADY_OPEN));
		return false;
	}

	guild_storage = idb_get(gstorage->db,sd->status.guild_id);
	if (guild_storage == NULL) {// Doesn't have opened @gstorage yet, so we skip the deletion since *shouldn't* have any item there.
		return false;
	}

	j = guild_storage->items.capacity;
	guild_storage->locked = true; // Lock @gstorage: do not allow any item to be retrieved or stored from any guild member
	for (i = 0; i < j; ++i) {
		gstorage->delitem(sd, guild_storage, i, guild_storage->items.data[i].amount);
	}
	gstorage->close(sd);
	guild_storage->locked = false; // Cleaning done, release lock

	clif->message(fd, msg_fd(fd, MSGTBL_CLEARGUILDSTORAGE_SUCCESS)); // Your guild storage was cleaned.
	return true;
}

ACMD(clearcart)
{
	int i;

	if (pc_iscarton(sd) == 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_CLEARCART_NO_CART)); // You do not have a cart to be cleaned.
		return false;
	}

	if (sd->state.vending || sd->state.prevend) {
		clif->message(fd, msg_fd(fd, MSGTBL_CLEARCART_FAIL_VENDING)); // You can't clean a cart while vending!
		return false;
	}

	for( i = 0; i < MAX_CART; i++ )
		if(sd->status.cart[i].nameid > 0)
			pc->cart_delitem(sd, i, sd->status.cart[i].amount, 1, LOG_TYPE_COMMAND);

	clif->clearcart(fd);
	clif->updatestatus(sd,SP_CARTINFO);

	clif->message(fd, msg_fd(fd, MSGTBL_CLEARCART_SUCCESS)); // Your cart was cleaned.
	return true;
}

/*==========================================
 * @skillid by [MouseJstr]
 * lookup a skill by name
 *------------------------------------------*/
#define MAX_SKILLID_PARTIAL_RESULTS 5
#define MAX_SKILLID_PARTIAL_RESULTS_LEN 74 /* "skill " (6) + "%d:" (up to 5) + "%s" (up to 30) + " (%s)" (up to 33) */
ACMD(skillid)
{
	int i, found = 0;
	size_t skillen;
	struct DBIterator *iter;
	union DBKey key;
	struct DBData *data;
	char partials[MAX_SKILLID_PARTIAL_RESULTS][MAX_SKILLID_PARTIAL_RESULTS_LEN];

	if (!*message) {
		clif->message(fd, msg_fd(fd, MSGTBL_SKILL_ID_ENTER_NAME)); // Please enter a skill name to look up (usage: @skillid <skill name>).
		return false;
	}

	skillen = strlen(message);

	iter = db_iterator(skill->name2id_db);

	for (data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key)) {
		int skill_id = DB->data2i(data);
		const char *skill_desc = skill->get_desc(skill_id);
		if (strnicmp(key.str, message, skillen) == 0 || strnicmp(skill_desc, message, skillen) == 0) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_SKILL_ID_INFO), skill_id, skill_desc, key.str); // skill %d: %s (%s)
			clif->message(fd, atcmd_output);
		} else if (found < MAX_SKILLID_PARTIAL_RESULTS && (stristr(key.str, message) != NULL || stristr(skill_desc, message) != NULL)) {
			snprintf(partials[found], MAX_SKILLID_PARTIAL_RESULTS_LEN, msg_fd(fd, MSGTBL_SKILL_ID_INFO), skill_id, skill_desc, key.str);
			found++;
		}
	}

	dbi_destroy(iter);

	if( found ) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_SKILLID_PARTIAL_MATCHES), found); // -- Displaying first %d partial matches
		clif->message(fd, atcmd_output);
	}

	for(i = 0; i < found; i++) { /* partials */
		clif->message(fd, partials[i]);
	}

	return true;
}

/*==========================================
 * @useskill by [MouseJstr]
 * A way of using skills without having to find them in the skills menu
 *------------------------------------------*/
ACMD(useskill)
{
	struct map_session_data *pl_sd = NULL;
	struct block_list *bl;
	uint16 skill_id;
	uint16 skill_lv;
	char target[100];

	if (!*message || sscanf(message, "%5hu %5hu %23[^\n]", &skill_id, &skill_lv, target) != 3) {
		clif->message(fd, msg_fd(fd, MSGTBL_USESKILL_USAGE)); // Usage: @useskill <skill ID> <skill level> <target>
		return false;
	}

	if (!strcmp(target,"self"))
		pl_sd = sd; //quick keyword
	else if ((pl_sd = map->nick2sd(target, true)) == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_CHARACTER_NOT_FOUND)); // Character not found.
		return false;
	}

	if (pc_get_group_level(sd) < pc_get_group_level(pl_sd))
	{
		clif->message(fd, msg_fd(fd, MSGTBL_GM_LEVEL_UNAUTHORIZED)); // Your GM level don't authorized you to do this action on this player.
		return false;
	}

	pc->autocast_clear(sd);

	if (skill_id >= HM_SKILLBASE && skill_id < HM_SKILLBASE+MAX_HOMUNSKILL
		&& homun_alive(sd->hd)) // (If used with @useskill, put the homunc as dest)
		bl = &sd->hd->bl;
	else
		bl = &sd->bl;

	pc->delinvincibletimer(sd);

	if (skill->get_inf(skill_id)&INF_GROUND_SKILL)
		unit->skilluse_pos(bl, pl_sd->bl.x, pl_sd->bl.y, skill_id, skill_lv);
	else
		unit->skilluse_id(bl, pl_sd->bl.id, skill_id, skill_lv);

	return true;
}

/*==========================================
 * @displayskill by [Skotlex]
 *  Debug command to locate new skill IDs. It sends the
 *  three possible skill-effect packets to the area.
 *------------------------------------------*/
ACMD(displayskill)
{
	struct status_data *st;
	int64 tick;
	uint16 skill_id;
	uint16 skill_lv = 1;

	if (!*message || sscanf(message, "%5hu %5hu", &skill_id, &skill_lv) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_DISPLAYSKILL_USAGE)); // Usage: @displayskill <skill ID> {<skill level>}
		return false;
	}
	st = status->get_status_data(&sd->bl);
	tick = timer->gettick();
	clif->skill_damage(&sd->bl,&sd->bl, tick, st->amotion, st->dmotion, 1, 1, skill_id, skill_lv, BDT_SPLASH);
	clif->skill_nodamage(&sd->bl, &sd->bl, skill_id, skill_lv, 1);
	clif->skill_poseffect(&sd->bl, skill_id, skill_lv, sd->bl.x, sd->bl.y, tick);
	return true;
}

/*==========================================
 * @skilltree by [MouseJstr]
 * prints the skill tree for a player required to get to a skill
 *------------------------------------------*/
ACMD(skilltree)
{
	struct map_session_data *pl_sd = NULL;
	uint16 skill_id;
	int meets, j, c=0;
	char target[NAME_LENGTH];
	struct skill_tree_entry *entry;

	if(!*message || sscanf(message, "%5hu %23[^\r\n]", &skill_id, target) != 2) {
		clif->message(fd, msg_fd(fd, MSGTBL_SKILLTREE_USAGE)); // Usage: @skilltree <skill ID> <target>
		return false;
	}

	if ( (pl_sd = map->nick2sd(target, true)) == NULL ) {
		clif->message(fd, msg_fd(fd, MSGTBL_CHARACTER_NOT_FOUND)); // Character not found.
		return false;
	}

	c = pc->calc_skilltree_normalize_job(pl_sd);
	c = pc->mapid2jobid(c, pl_sd->status.sex);

	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_PLAYER_SKILL_TREE_INFO), pc->job_name(c), pc->checkskill(pl_sd, NV_BASIC)); // Player is using %s skill tree (%d basic points).
	clif->message(fd, atcmd_output);

	ARR_FIND(0, MAX_SKILL_TREE, j, pc->skill_tree[c][j].id == 0 || pc->skill_tree[c][j].id == skill_id);
	if (j == MAX_SKILL_TREE || pc->skill_tree[c][j].id == 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_CANNOT_USE_SKILL)); // The player cannot use that skill.
		return false;
	}

	entry = &pc->skill_tree[c][j];

	meets = 1;
	for (j = 0; j < VECTOR_LENGTH(entry->need); j++) {
		struct skill_tree_requirement *req = &VECTOR_INDEX(entry->need, j);
		if (pc->checkskill(sd, req->id) < req->lv) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_PLAYER_REQUIRES_LEVEL), req->lv, skill->get_desc(req->id)); // Player requires level %d of skill %s.
			clif->message(fd, atcmd_output);
			meets = 0;
		}
	}
	if (meets == 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_PLAYER_MEETS_REQUIREMENTS)); // The player meets all the requirements for that skill.
	}

	return true;
}

// Hand a ring with partners name on it to this char
static void atcommand_getring(struct map_session_data *sd)
{
	int flag, item_id;
	struct item item_tmp;
	nullpo_retv(sd);
	item_id = (sd->status.sex) ? WEDDING_RING_M : WEDDING_RING_F;

	memset(&item_tmp, 0, sizeof(item_tmp));
	item_tmp.nameid = item_id;
	item_tmp.identify = 1;
	item_tmp.card[0] = CARD0_FORGE;
	item_tmp.card[2] = GetWord(sd->status.partner_id, 0);
	item_tmp.card[3] = GetWord(sd->status.partner_id, 1);

	if((flag = pc->additem(sd,&item_tmp,1,LOG_TYPE_COMMAND))) {
		clif->additem(sd,0,0,flag);
		map->addflooritem(&sd->bl, &item_tmp, 1, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0, false);
	}
}

/*==========================================
 * @marry by [MouseJstr], fixed by Lupus
 * Marry two players
 *------------------------------------------*/
ACMD(marry)
{
	struct map_session_data *pl_sd = NULL;
	char player_name[NAME_LENGTH] = "";

	if (!*message || sscanf(message, "%23s", player_name) != 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_MARRY_USAGE)); // Usage: @marry <char name>
		return false;
	}

	if ((pl_sd = map->nick2sd(player_name, true)) == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_CHARACTER_NOT_FOUND));
		return false;
	}

	if (pc->marriage(sd, pl_sd) == 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_ALREADY_MARRIED)); // They are married... wish them well.
		clif->wedding_effect(&pl_sd->bl); //wedding effect and music [Lupus]
		atcommand->getring(sd); // Auto-give named rings (Aru)
		atcommand->getring(pl_sd);
		return true;
	}

	clif->message(fd, msg_fd(fd, MSGTBL_CANNOT_WED)); // The two cannot wed because one is either a baby or already married.
	return false;
}

/*==========================================
 * @divorce by [MouseJstr], fixed by [Lupus]
 * divorce two players
 *------------------------------------------*/
ACMD(divorce)
{
	if (pc->divorce(sd) != 0) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_NOT_MARRIED), sd->status.name); // '%s' is not married.
		clif->message(fd, atcmd_output);
		return false;
	}

	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_DIVORCE_SUCCESS), sd->status.name); // '%s' and his/her partner are now divorced.
	clif->message(fd, atcmd_output);
	return true;
}

/*==========================================
 * @changelook by [Celest]
 *------------------------------------------*/
ACMD(changelook)
{
	int i, j = 0, k = 0;
	int pos[8] = { LOOK_HEAD_TOP,LOOK_HEAD_MID,LOOK_HEAD_BOTTOM,LOOK_WEAPON,LOOK_SHIELD,LOOK_SHOES,LOOK_ROBE,LOOK_BODY2 };

	if((i = sscanf(message, "%12d %12d", &j, &k)) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_CHANGELOOK_USAGE)); // Usage: @changelook {<position>} <view id>
		clif->message(fd, msg_fd(fd, MSGTBL_CHANGELOOK_POSITION_INFO)); // Position: 1-Top 2-Middle 3-Bottom 4-Weapon 5-Shield 6-Shoes 7-Robe
		return false;
	} else if ( i == 2 ) {
		if (j < 1 || j > 7)
			j = 1;
		j = pos[j - 1];
	} else if( i == 1 ) { // position not defined, use HEAD_TOP as default
		k = j; // swap
		j = LOOK_HEAD_TOP;
	}

	clif->changelook(&sd->bl,j,k);

	return true;
}

/*==========================================
 * @autotrade by durf [Lupus] [Paradox924X]
 * Turns on/off Autotrade for a specific player
 *------------------------------------------*/
ACMD(autotrade)
{
	if( map->list[sd->bl.m].flag.autotrade != battle_config.autotrade_mapflag ) {
		clif->message(fd, msg_fd(fd, MSGTBL_AUTOTRADE_NOT_ALLOWED)); // Autotrade is not allowed in this map.
		return false;
	}

	if( pc_isdead(sd) ) {
		clif->message(fd, msg_fd(fd, MSGTBL_CANNOT_AUTOTRADE_WHEN_DEAD)); // You cannot autotrade when dead.
		return false;
	}

	if( !sd->state.vending && !sd->state.buyingstore ) { //check if player is vending or buying
		clif->message(fd, msg_fd(fd, MSGTBL_AUTOTRADE_MISSING_SHOP)); // "You should have a shop open in order to use @autotrade."
		return false;
	}

	sd->state.autotrade = 1;
	if( battle_config.at_timeout ) {
		int timeout = atoi(message);
		status->change_start(NULL,&sd->bl, SC_AUTOTRADE, 10000, 0, 0, 0, 0,
		                     ((timeout > 0) ? min(timeout, battle_config.at_timeout) : battle_config.at_timeout) * 60000, SCFLAG_NONE, 0);
	}

	channel->quit(sd);
	goldpc->stop(sd);

	clif->authfail_fd(sd->fd, 15);

	/* currently standalone is not supporting buyingstores, so we rely on the previous method */
	if( sd->state.buyingstore )
		return true;

#ifdef AUTOTRADE_PERSISTENCY
	sd->state.autotrade = 2;/** state will enter pre-save, we use it to rule out some criterias **/
	pc->autotrade_prepare(sd);

	return false;/* we fail to not cause it to proceed on is_atcommand */
#else
	return true;
#endif
}

/*==========================================
 * @changegm by durf (changed by Lupus)
 * Changes Master of your Guild to a specified guild member
 *------------------------------------------*/
ACMD(changegm)
{
	struct guild *g;
	struct map_session_data *pl_sd;

	if (sd->status.guild_id == 0 || (g = sd->guild) == NULL || strcmp(g->master,sd->status.name)) {
		clif->message(fd, msg_fd(fd, MSGTBL_CHANGEGM_GM_REQUIRED)); // You need to be a Guild Master to use this command.
		return false;
	}

	if (map->list[sd->bl.m].flag.guildlock || map->list[sd->bl.m].flag.gvg_castle) {
		clif->message(fd, msg_fd(fd, MSGTBL_CHANGEGM_MAP_RESTRICTED)); // You cannot change guild leaders in this map.
		return false;
	}

	if (!message[0]) {
		clif->message(fd, msg_fd(fd, MSGTBL_CHANGEGM_USAGE)); // Usage: @changegm <guild_member_name>
		return false;
	}

	if ((pl_sd=map->nick2sd(message, true)) == NULL || pl_sd->status.guild_id != sd->status.guild_id) {
		clif->message(fd, msg_fd(fd, MSGTBL_CHANGEGM_TARGET_CONDITIONS)); // Target character must be online and be a guild member.
		return false;
	}

	guild->gm_change(sd->status.guild_id, pl_sd->status.char_id);
	return true;
}

/*==========================================
 * @changeleader by Skotlex
 * Changes the leader of a party.
 *------------------------------------------*/
ACMD(changeleader)
{

	if (!message[0]) {
		clif->message(fd, msg_fd(fd, MSGTBL_CHANGELEADER_USAGE)); // Usage: @changeleader <party_member_name>
		return false;
	}

	if (party->changeleader(sd, map->nick2sd(message, true)))
		return true;
	return false;
}

/*==========================================
 * @partyoption by Skotlex
 * Used to change the item share setting of a party.
 *------------------------------------------*/
ACMD(partyoption)
{
	struct party_data *p;
	int mi, option;
	char w1[16], w2[16];

	if (sd->status.party_id == 0 || (p = party->search(sd->status.party_id)) == NULL)
	{
		clif->message(fd, msg_fd(fd, MSGTBL_MUST_BE_PARTY_LEADER));
		return false;
	}

	ARR_FIND(0, MAX_PARTY, mi, p->data[mi].sd == sd);
	if (mi == MAX_PARTY)
		return false; //Shouldn't happen

	if (!p->party.member[mi].leader)
	{
		clif->message(fd, msg_fd(fd, MSGTBL_MUST_BE_PARTY_LEADER));
		return false;
	}

	if (!*message || sscanf(message, "%15s %15s", w1, w2) < 2)
	{
		clif->message(fd, msg_fd(fd, MSGTBL_PARTYOPTION_USAGE)); // Usage: @partyoption <pickup share: yes/no> <item distribution: yes/no>
		return false;
	}

	option = (config_switch(w1)?1:0)|(config_switch(w2)?2:0); // TODO: Add documentation for these values

	//Change item share type.
	if (option != p->party.item)
		party->changeoption(sd, p->party.exp, option);
	else
		clif->message(fd, msg_fd(fd, MSGTBL_NO_CHANGE_IN_PARTY_SETTINGS));

	return true;
}

/*==========================================
 * @autoloot by Upa-Kun
 * Turns on/off AutoLoot for a specific player
 *------------------------------------------*/
ACMD(autoloot)
{
	int rate;

	// autoloot command without value
	if (!*message)
	{
		if (sd->state.autoloot)
			rate = 0;
		else
			rate = 10000;
	} else {
		double drate;
		drate = atof(message);
		rate = (int)(drate*100);
	}
	if (rate < 0) rate = 0;
	if (rate > 10000) rate = 10000;

	sd->state.autoloot = rate;
	if (sd->state.autoloot) {
		snprintf(atcmd_output, sizeof atcmd_output, msg_fd(fd, MSGTBL_AUTOLOOT_DROP_RATE),((double)sd->state.autoloot)/100.); // Autolooting items with drop rates of %0.02f%% and below.
		clif->message(fd, atcmd_output);
	}else
		clif->message(fd, msg_fd(fd, MSGTBL_AUTOLOOT_OFF)); // Autoloot is now off.

	return true;
}

/*==========================================
 * @alootid
 *------------------------------------------*/
ACMD(autolootitem)
{
	struct item_data *item_data = NULL;
	int i;
	int action = 3; // 1=add, 2=remove, 3=help+list (default), 4=reset

	if (*message) {
		if (message[0] == '+') {
			message++;
			action = 1;
		}
		else if (message[0] == '-') {
			message++;
			action = 2;
		}
		else if (!strcmp(message,"reset"))
			action = 4;

		if (action < 3) // add or remove
		{
			if ((item_data = itemdb->exists(atoi(message))) == NULL)
				item_data = itemdb->search_name(message);
			if (!item_data) {
				// No items founds in the DB with Id or Name
				clif->message(fd, msg_fd(fd, MSGTBL_AUTOLOOTITEM_NOT_FOUND)); // Item not found.
				return false;
			}
		}
	}

	switch (action) {
		case 1:
			ARR_FIND(0, AUTOLOOTITEM_SIZE, i, sd->state.autolootid[i] == item_data->nameid);
			if (i != AUTOLOOTITEM_SIZE) {
				clif->message(fd, msg_fd(fd, MSGTBL_AUTOLOOTITEM_ALREADY)); // You're already autolooting this item.
				return false;
			}
			ARR_FIND(0, AUTOLOOTITEM_SIZE, i, sd->state.autolootid[i] == 0);
			if (i == AUTOLOOTITEM_SIZE) {
				clif->message(fd, msg_fd(fd, MSGTBL_AUTOLOOTITEM_LIST_FULL)); // Your autolootitem list is full. Remove some items first with @autolootid -<item name or ID>.
				return false;
			}
			sd->state.autolootid[i] = item_data->nameid; // Autoloot Activated
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_AUTOLOOTITEM_ADDED), item_data->name, item_data->jname, item_data->nameid); // Autolooting item: '%s'/'%s' {%d}
			clif->message(fd, atcmd_output);
			sd->state.autolooting = 1;
			break;
		case 2:
			ARR_FIND(0, AUTOLOOTITEM_SIZE, i, sd->state.autolootid[i] == item_data->nameid);
			if (i == AUTOLOOTITEM_SIZE) {
				clif->message(fd, msg_fd(fd, MSGTBL_AUTOLOOTITEM_NOT_CURRENTLY)); // You're currently not autolooting this item.
				return false;
			}
			sd->state.autolootid[i] = 0;
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_AUTOLOOTITEM_REMOVED), item_data->name, item_data->jname, item_data->nameid); // Removed item: '%s'/'%s' {%d} from your autolootitem list.
			clif->message(fd, atcmd_output);
			ARR_FIND(0, AUTOLOOTITEM_SIZE, i, sd->state.autolootid[i] != 0);
			if (i == AUTOLOOTITEM_SIZE) {
				sd->state.autolooting = 0;
			}
			break;
		case 3:
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_AUTOLOOTITEM_LIST_LIMIT), AUTOLOOTITEM_SIZE); // You can have %d items on your autolootitem list.
			clif->message(fd, atcmd_output);
			clif->message(fd, msg_fd(fd, MSGTBL_AUTOLOOTITEM_ADD_REMOVE_INFO)); // To add an item to the list, use "@alootid +<item name or ID>". To remove an item, use "@alootid -<item name or ID>".
			clif->message(fd, msg_fd(fd, MSGTBL_AUTOLOOTITEM_RESET_INFO)); // "@alootid reset" will clear your autolootitem list.
			ARR_FIND(0, AUTOLOOTITEM_SIZE, i, sd->state.autolootid[i] != 0);
			if (i == AUTOLOOTITEM_SIZE) {
				clif->message(fd, msg_fd(fd, MSGTBL_AUTOLOOTITEM_EMPTY)); // Your autolootitem list is empty.
			} else {
				clif->message(fd, msg_fd(fd, MSGTBL_AUTOLOOTITEM_LIST_HEADER)); // Items on your autolootitem list:
				for(i = 0; i < AUTOLOOTITEM_SIZE; i++)
				{
					if (sd->state.autolootid[i] == 0)
						continue;
					if (!(item_data = itemdb->exists(sd->state.autolootid[i]))) {
						ShowDebug("Non-existant item %d on autolootitem list (account_id: %d, char_id: %d)", sd->state.autolootid[i], sd->status.account_id, sd->status.char_id);
						continue;
					}
					snprintf(atcmd_output, sizeof(atcmd_output), "'%s'/'%s' {%d}", item_data->name, item_data->jname, item_data->nameid);
					clif->message(fd, atcmd_output);
				}
			}
			break;
		case 4:
			memset(sd->state.autolootid, 0, sizeof(sd->state.autolootid));
			clif->message(fd, msg_fd(fd, MSGTBL_AUTOLOOTITEM_RESET_SUCCESS)); // Your autolootitem list has been reset.
			sd->state.autolooting = 0;
			break;
	}
	return true;
}

/*==========================================
 * @autoloottype
 * Credits:
 *    chriser,Aleos
 *------------------------------------------*/
ACMD(autoloottype)
{
	uint8 action = 3; // 1=add, 2=remove, 3=help+list (default), 4=reset
	enum item_types type = -1;
	int ITEM_NONE = 0;

	if (*message) {
		if (message[0] == '+') {
			message++;
			action = 1;
		} else if (message[0] == '-') {
			message++;
			action = 2;
		} else if (strcmp(message,"reset") == 0) {
			action = 4;
		}

		if (action < 3) {
			// add or remove
			if (strncmp(message, "healing", 3) == 0)
				type = IT_HEALING;
			else if (strncmp(message, "usable", 3) == 0)
				type = IT_USABLE;
			else if (strncmp(message, "etc", 3) == 0)
				type = IT_ETC;
			else if (strncmp(message, "weapon", 3) == 0)
				type = IT_WEAPON;
			else if (strncmp(message, "armor", 3) == 0)
				type = IT_ARMOR;
			else if (strncmp(message, "card", 3) == 0)
				type = IT_CARD;
			else if (strncmp(message, "petegg", 4) == 0)
				type = IT_PETEGG;
			else if (strncmp(message, "petarmor", 4) == 0)
				type = IT_PETARMOR;
			else if (strncmp(message, "ammo", 3) == 0)
				type = IT_AMMO;
			else {
				clif->message(fd, msg_fd(fd, MSGTBL_AUTOLOOT_TYPE_NOT_FOUND)); // Item type not found.
				return false;
			}
		}
	}

	switch (action) {
		case 1:
			if (sd->state.autoloottype&(1<<type)) {
				clif->message(fd, msg_fd(fd, MSGTBL_AUTOLOOT_ALREADY_ENABLED)); // You're already autolooting this item type.
				return false;
			}
			sd->state.autoloottype |= (1<<type); // Stores the type
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_AUTOLOOT_TYPE_ENABLED), itemdb->typename(type)); // Autolooting item type: '%s'
			clif->message(fd, atcmd_output);
			break;
		case 2:
			if (!(sd->state.autoloottype&(1<<type))) {
				clif->message(fd, msg_fd(fd, MSGTBL_AUTOLOOT_NOT_ENABLED)); // You're currently not autolooting this item type.
				return false;
			}
			sd->state.autoloottype &= ~(1<<type);
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_AUTOLOOT_TYPE_REMOVED), itemdb->typename(type)); // Removed item type: '%s' from your autoloottype list.
			clif->message(fd, atcmd_output);
			break;
		case 3:
			clif->message(fd, msg_fd(fd, MSGTBL_INVALID_LOCATION_OR_NAME)); // Invalid location number, or name.

			{
				// attempt to find the text help string
				const char *text = atcommand_help_string(info);
				if (text) clif->messageln(fd, text); // send the text to the client
			}

			if (sd->state.autoloottype == ITEM_NONE) {
				clif->message(fd, msg_fd(fd, MSGTBL_AUTOLOOT_TYPE_LIST_EMPTY)); // Your autoloottype list is empty.
			} else {
				int i;
				clif->message(fd, msg_fd(fd, MSGTBL_AUTOLOOT_TYPE_LIST)); // Item types on your autoloottype list:
				for(i=0; i < IT_MAX; i++) {
					if (sd->state.autoloottype&(1<<i)) {
						snprintf(atcmd_output, sizeof(atcmd_output), " '%s'", itemdb->typename(i));
						clif->message(fd, atcmd_output);
					}
				}
			}
			break;
		case 4:
			sd->state.autoloottype = ITEM_NONE;
			clif->message(fd, msg_fd(fd, MSGTBL_AUTOLOOT_TYPE_RESET)); // Your autoloottype list has been reset.
			break;
	}
	return true;
}

/*==========================================
 * It is made to snow.
 *------------------------------------------*/
ACMD(snow)
{
	if (map->list[sd->bl.m].flag.snow) {
		map->list[sd->bl.m].flag.snow=0;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_fd(fd, MSGTBL_SNOW_STOPPED)); // Snow has stopped falling.
	} else {
		map->list[sd->bl.m].flag.snow=1;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_fd(fd, MSGTBL_SNOW_STARTED)); // It has started to snow.
	}

	return true;
}

/*==========================================
 * Cherry tree snowstorm is made to fall. (Sakura)
 *------------------------------------------*/
ACMD(sakura)
{
	if (map->list[sd->bl.m].flag.sakura) {
		map->list[sd->bl.m].flag.sakura=0;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_fd(fd, MSGTBL_SAKURA_STOPPED)); // Cherry tree leaves no longer fall.
	} else {
		map->list[sd->bl.m].flag.sakura=1;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_fd(fd, MSGTBL_SAKURA_STARTED)); // Cherry tree leaves have begun to fall.
	}
	return true;
}

/*==========================================
 * Clouds appear.
 *------------------------------------------*/
ACMD(clouds)
{
	if (map->list[sd->bl.m].flag.clouds) {
		map->list[sd->bl.m].flag.clouds=0;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_fd(fd, MSGTBL_CLOUDS_DISAPPEARED)); // The clouds has disappear.
	} else {
		map->list[sd->bl.m].flag.clouds=1;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_fd(fd, MSGTBL_CLOUDS_APPEARED)); // Clouds appear.
	}

	return true;
}

/*==========================================
 * Different type of clouds using effect 516
 *------------------------------------------*/
ACMD(clouds2)
{
	if (map->list[sd->bl.m].flag.clouds2) {
		map->list[sd->bl.m].flag.clouds2=0;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_fd(fd, MSGTBL_CLOUDS2_DISAPPEARED)); // The alternative clouds disappear.
	} else {
		map->list[sd->bl.m].flag.clouds2=1;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_fd(fd, MSGTBL_CLOUDS2_APPEARED)); // Alternative clouds appear.
	}

	return true;
}

/*==========================================
 * Fog hangs over.
 *------------------------------------------*/
ACMD(fog)
{
	if (map->list[sd->bl.m].flag.fog) {
		map->list[sd->bl.m].flag.fog=0;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_fd(fd, MSGTBL_FOG_GONE)); // The fog has gone.
	} else {
		map->list[sd->bl.m].flag.fog=1;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_fd(fd, MSGTBL_FOG_APPEARED)); // Fog hangs over.
	}
	return true;
}

/*==========================================
 * Fallen leaves fall.
 *------------------------------------------*/
ACMD(leaves)
{
	if (map->list[sd->bl.m].flag.leaves) {
		map->list[sd->bl.m].flag.leaves=0;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_fd(fd, MSGTBL_LEAVES_STOPPED)); // Leaves no longer fall.
	} else {
		map->list[sd->bl.m].flag.leaves=1;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_fd(fd, MSGTBL_LEAVES_STARTED)); // Fallen leaves fall.
	}

	return true;
}

/*==========================================
 * Fireworks appear.
 *------------------------------------------*/
ACMD(fireworks)
{
	if (map->list[sd->bl.m].flag.fireworks) {
		map->list[sd->bl.m].flag.fireworks=0;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_fd(fd, MSGTBL_FIREWORKS_ENDED)); // Fireworks have ended.
	} else {
		map->list[sd->bl.m].flag.fireworks=1;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_fd(fd, MSGTBL_FIREWORKS_LAUNCHED)); // Fireworks have launched.
	}

	return true;
}

/*==========================================
 * Clearing Weather Effects by Dexity
 *------------------------------------------*/
ACMD(clearweather)
{
	map->list[sd->bl.m].flag.snow=0;
	map->list[sd->bl.m].flag.sakura=0;
	map->list[sd->bl.m].flag.clouds=0;
	map->list[sd->bl.m].flag.clouds2=0;
	map->list[sd->bl.m].flag.fog=0;
	map->list[sd->bl.m].flag.fireworks=0;
	map->list[sd->bl.m].flag.leaves=0;
	clif->weather(sd->bl.m);
	clif->message(fd, msg_fd(fd, MSGTBL_CLEARWEATHER)); // "Weather effects will disappear after teleporting or refreshing."

	return true;
}

/*===============================================================
 * Sound Command - plays a sound for everyone around! [Codemaster]
 *---------------------------------------------------------------*/
ACMD(sound)
{
	char sound_file[100];

	memset(sound_file, '\0', sizeof(sound_file));

	if(!*message || sscanf(message, "%99[^\n]", sound_file) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_SOUND_USAGE)); // Please enter a sound filename (usage: @sound <filename>).
		return false;
	}

	if(strstr(sound_file, ".wav") == NULL)
		strcat(sound_file, ".wav");

	clif->soundeffectall(&sd->bl, sound_file, PLAY_SOUND_ONCE, 0, AREA);

	return true;
}

/*==========================================
 * MOB Search
 *------------------------------------------*/
ACMD(mobsearch)
{
	char mob_name[100];
	int mob_id;
	int number = 0;
	struct s_mapiterator *it;
	const struct mob_data *md = NULL;

	if (!*message || sscanf(message, "%99[^\n]", mob_name) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_MOBSEARCH_USAGE)); // Please enter a monster name (usage: @mobsearch <monster name>).
		return false;
	}

	if ((mob_id = atoi(mob_name)) == 0)
		mob_id = mob->db_searchname(mob_name);
	if(mob_id > 0 && mob->db_checkid(mob_id) == 0){
		snprintf(atcmd_output, sizeof atcmd_output, msg_fd(fd, MSGTBL_MOBSEARCH_INVALID_ID),mob_name); // Invalid mob ID %s!
		clif->message(fd, atcmd_output);
		return false;
	}
	if (mob_id == atoi(mob_name)) {
		strcpy(mob_name,mob->db(mob_id)->jname); // DEFAULT_MOB_JNAME
		//strcpy(mob_name,mob->db(mob_id)->name); // DEFAULT_MOB_NAME
	}

	snprintf(atcmd_output, sizeof atcmd_output, msg_fd(fd, MSGTBL_MOBSEARCH_RESULT), mob_name, mapindex_id2name(sd->mapindex)); // Mob Search... %s %s
	clif->message(fd, atcmd_output);

	it = mapit_geteachmob();
	for (md = BL_UCCAST(BL_MOB, mapit->first(it)); mapit->exists(it); md = BL_UCCAST(BL_MOB, mapit->next(it))) {
		if( md->bl.m != sd->bl.m )
			continue;
		if( mob_id != -1 && md->class_ != mob_id )
			continue;

		++number;
		if( md->spawn_timer == INVALID_TIMER )
			snprintf(atcmd_output, sizeof(atcmd_output), "%2d[%3d:%3d] %s", number, md->bl.x, md->bl.y, md->name);
		else
			snprintf(atcmd_output, sizeof(atcmd_output), "%2d[%s] %s", number, "dead", md->name);
		clif->message(fd, atcmd_output);
	}
	mapit->free(it);

	return true;
}

/*==========================================
 * @cleanmap - cleans items on the ground
 * @cleanarea - cleans items on the ground within an specified area
 *------------------------------------------*/
static int atcommand_cleanfloor_sub(struct block_list *bl, va_list ap)
{
	nullpo_ret(bl);
	map->clearflooritem(bl);

	return 0;
}

ACMD(cleanmap)
{
	map->foreachinmap(atcommand->cleanfloor_sub, sd->bl.m, BL_ITEM);
	clif->message(fd, msg_fd(fd, MSGTBL_CLEANMAP_SUCCESS)); // All dropped items have been cleaned up.
	return true;
}

ACMD(cleanarea)
{
	int x0 = 0, y0 = 0, x1 = 0, y1 = 0, n = 0;

	if (!*message || (n=sscanf(message, "%d %d %d %d", &x0, &y0, &x1, &y1)) < 1) {
		map->foreachinrange(atcommand->cleanfloor_sub, &sd->bl, AREA_SIZE * 2, BL_ITEM);
	} else if (n == 4) {
		map->foreachinarea(atcommand->cleanfloor_sub, sd->bl.m, x0, y0, x1, y1, BL_ITEM);
	} else {
		map->foreachinrange(atcommand->cleanfloor_sub, &sd->bl, x0, BL_ITEM);
	}

	clif->message(fd, msg_fd(fd, MSGTBL_CLEANMAP_SUCCESS)); // All dropped items have been cleaned up.
	return true;
}

/*==========================================
 * make a NPC/PET talk
 * @npctalkc [SnakeDrak]
 *------------------------------------------*/
ACMD(npctalk)
{
	char name[NAME_LENGTH], mes[100], temp[200];
	struct npc_data *nd;
	bool ifcolor=(*(info->command + 7) != 'c' && *(info->command + 7) != 'C')?0:1;
	unsigned int color = 0;

	if (!pc->can_talk(sd))
		return false;

	if(!ifcolor) {
		if (!*message || sscanf(message, "%23[^,], %99[^\n]", name, mes) < 2) {
			clif->message(fd, msg_fd(fd, MSGTBL_NPCTALK_USAGE)); // Please enter the correct parameters (usage: @npctalk <npc name>, <message>).
			return false;
		}
	}
	else {
		if (!*message || sscanf(message, "%12u %23[^,], %99[^\n]", &color, name, mes) < 3) {
			clif->message(fd, msg_fd(fd, MSGTBL_NPCTALKC_USAGE)); // Please enter the correct parameters (usage: @npctalkc <color> <npc name>, <message>).
			return false;
		}
	}

	if (!(nd = npc->name2id(name))) {
		clif->message(fd, msg_fd(fd, MSGTBL_NPC_DOESNT_EXIST)); // This NPC doesn't exist
		return false;
	}

	strtok(name, "#"); // discard extra name identifier if present
	snprintf(temp, sizeof(temp), "%s : %s", name, mes);

	if(ifcolor) clif->messagecolor(&nd->bl,color,temp);
	else clif->disp_overhead(&nd->bl, temp, AREA_CHAT_WOC, NULL);

	return true;
}

ACMD(pettalk)
{
	char mes[100], temp[200];
	struct pet_data *pd;

	if (battle_config.min_chat_delay) {
		if( DIFF_TICK(sd->cantalk_tick, timer->gettick()) > 0 )
			return true;
		sd->cantalk_tick = timer->gettick() + battle_config.min_chat_delay;
	}

	if (!sd->status.pet_id || !(pd=sd->pd))
	{
		clif->message(fd, msg_fd(fd, MSGTBL_NO_PET));
		return false;
	}

	if (!pc->can_talk(sd))
		return false;

	if (!*message || sscanf(message, "%99[^\n]", mes) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_PETTALK_USAGE)); // Please enter a message (usage: @pettalk <message>).
		return false;
	}

	if (message[0] == '/')
	{ // pet emotion processing
		const char* emo[] = {
			"/!", "/?", "/ho", "/lv", "/swt", "/ic", "/an", "/ag", "/$", "/...",
			"/scissors", "/rock", "/paper", "/korea", "/lv2", "/thx", "/wah", "/sry", "/heh", "/swt2",
			"/hmm", "/no1", "/??", "/omg", "/O", "/X", "/hlp", "/go", "/sob", "/gg",
			"/kis", "/kis2", "/pif", "/ok", "-?-", "/indonesia", "/bzz", "/rice", "/awsm", "/meh",
			"/shy", "/pat", "/mp", "/slur", "/com", "/yawn", "/grat", "/hp", "/philippines", "/malaysia",
			"/singapore", "/brazil", "/fsh", "/spin", "/sigh", "/dum", "/crwd", "/desp", "/dice", "-dice2",
			"-dice3", "-dice4", "-dice5", "-dice6", "/india", "/love", "/russia", "-?-", "/mobile", "/mail",
			"/chinese", "/antenna1", "/antenna2", "/antenna3", "/hum", "/abs", "/oops", "/spit", "/ene", "/panic",
			"/whisp"
		};
		int i;
		ARR_FIND( 0, ARRAYLENGTH(emo), i, stricmp(message, emo[i]) == 0 );
		if( i == E_DICE1 ) i = rnd()%6 + E_DICE1; // randomize /dice
		if( i < ARRAYLENGTH(emo) )
		{
			if (sd->emotionlasttime + 1 >= time(NULL)) { // not more than 1 per second
				sd->emotionlasttime = time(NULL);
				return true;
			}
			sd->emotionlasttime = time(NULL);

			clif->emotion(&pd->bl, i);
			return true;
		}
	}

	snprintf(temp, sizeof temp ,"%s : %s", pd->pet.name, mes);
	clif->disp_overhead(&pd->bl, temp, AREA_CHAT_WOC, NULL);

	return true;
}

/// @users - displays the number of players present on each map (and percentage)
/// #users displays on the target user instead of self
ACMD(users)
{
	char buf[CHAT_SIZE_MAX];
	int users[MAX_MAPINDEX];
	int users_all;
	struct s_mapiterator *iter;
	const struct map_session_data *pl_sd = NULL;

	memset(users, 0, sizeof(users));
	users_all = 0;

	// count users on each map
	iter = mapit_getallusers();
	for (pl_sd = BL_UCCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); pl_sd = BL_UCCAST(BL_PC, mapit->next(iter))) {
		if (pl_sd->mapindex >= MAX_MAPINDEX)
			continue;// invalid mapindex

		if (users[pl_sd->mapindex] < INT_MAX)
			++users[pl_sd->mapindex];
		if (users_all < INT_MAX)
			++users_all;
	}
	mapit->free(iter);

	if (users_all) {
		int i;
		// display results for each map
		for (i = 0; i < MAX_MAPINDEX; ++i) {
			if (users[i] == 0)
				continue; // empty

			snprintf(buf, sizeof(buf), "%s: %d (%.2f%%)", mapindex_id2name(i), users[i], (float)(100.0f*users[i]/users_all));
			clif->message(sd->fd, buf);
		}
	}

	// display overall count
	snprintf(buf, sizeof(buf), "all: %d", users_all);
	clif->message(sd->fd, buf);

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(reset)
{
	pc->resetstate(sd);
	pc->resetskill(sd, PCRESETSKILL_RESYNC);
	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_PLAYER_SKILL_AND_STATS_RESET), sd->status.name); // '%s' skill and stats points reseted!
	clif->message(fd, atcmd_output);
	return true;
}

/*==========================================
 *
 *------------------------------------------*/

/**
 * Spawns mobs which treats the invoking as its master.
 *
 * @code{.herc}
 *	@summon <monster name/ID> {<duration>}
 * @endcode
 *
 **/
ACMD(summon)
{
	char name[NAME_LENGTH + 1] = {'\0'};
	int duration = 0;

	if (*message == '\0' || sscanf(message, "%24s %12d", name, &duration) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_SUMMON_USAGE)); /// Please enter a monster name (usage: @summon <monster name> {duration}).
		return false;
	}

	int mob_id = atoi(name);

	if (mob_id == 0)
		mob_id = mob->db_searchname(name);

	if (mob_id == 0 || mob->db_checkid(mob_id) == 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_MONSTER_ID_NAME)); /// Invalid monster ID or name.
		return false;
	}

	struct mob_data *md = mob->once_spawn_sub(&sd->bl, sd->bl.m, -1, -1, DEFAULT_MOB_JNAME, mob_id, "",
						  SZ_SMALL, AI_NONE, 0);

	if (md == NULL)
		return false;

	md->master_id = sd->bl.id;
	md->special_state.ai = AI_ATTACK;

	const int64 tick = timer->gettick();

	md->deletetimer = timer->add(tick + (int64)cap_value(duration, 1, 60) * 60000, mob->timer_delete, md->bl.id, 0);
	clif->specialeffect(&md->bl, 344, AREA);
	mob->spawn(md);
	sc_start4(NULL, &md->bl, SC_MODECHANGE, 100, 1, 0, MD_AGGRESSIVE, 0, 60000, 0);
	clif->skill_poseffect(&sd->bl, AM_CALLHOMUN, 1, md->bl.x, md->bl.y, tick);
	clif->message(fd, msg_fd(fd, MSGTBL_MONSTERS_SUMMONED)); /// All monster summoned!
	return true;
}

/*==========================================
 * @adjgroup
 * Temporarily move player to another group
 * Useful during beta testing to allow players to use GM commands for short periods of time
 *------------------------------------------*/
ACMD(adjgroup)
{
	int new_group = 0;

	if (!*message || sscanf(message, "%12d", &new_group) != 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_ADJGROUP_USAGE)); // Usage: @adjgroup <group_id>
		return false;
	}

	if (pc->set_group(sd, new_group) != 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_ADJGROUP_NOT_EXIST)); // Specified group does not exist.
		return false;
	}

	clif->message(fd, msg_fd(fd, MSGTBL_ADJGROUP_CHANGE_SUCCESS)); // Group changed successfully.
	clif->message(sd->fd, msg_fd(fd, MSGTBL_ADJGROUP_YOUR_CHANGE)); // Your group has changed.
	return true;
}

/*==========================================
 * @trade by [MouseJstr]
 * Open a trade window with a remote player
 *------------------------------------------*/
ACMD(trade)
{
	struct map_session_data *pl_sd = NULL;

	if (!*message) {
		clif->message(fd, msg_fd(fd, MSGTBL_TRADE_USAGE)); // Please enter a player name (usage: @trade <char name>).
		return false;
	}

	if ((pl_sd = map->nick2sd(message, true)) == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_CHARACTER_NOT_FOUND)); // Character not found.
		return false;
	}

	trade->request(sd, pl_sd);
	return true;
}

/*==========================================
 * @setbattleflag by [MouseJstr]
 * set a battle_config flag without having to reboot
 *------------------------------------------*/
ACMD(setbattleflag)
{
	char flag[128], value[128];

	if (!*message || sscanf(message, "%127s %127s", flag, value) != 2) {
		clif->message(fd, msg_fd(fd, MSGTBL_SETBATTLEFLAG_USAGE)); // Usage: @setbattleflag <flag> <value>
		return false;
	}

	if (battle->config_set_value(flag, value) == 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_SETBATTLEFLAG_UNKNOWN_FLAG)); // Unknown battle_config flag.
		return false;
	}

	clif->message(fd, msg_fd(fd, MSGTBL_SETBATTLEFLAG_SUCCESS)); // Set battle_config as requested.

	return true;
}

/*==========================================
 * @unmute [Valaris]
 *------------------------------------------*/
ACMD(unmute)
{
	struct map_session_data *pl_sd = NULL;

	if (!*message) {
		clif->message(fd, msg_fd(fd, MSGTBL_UNMUTE_USAGE)); // Please enter a player name (usage: @unmute <char name>).
		return false;
	}

	if ((pl_sd = map->nick2sd(message, true)) == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_CHARACTER_NOT_FOUND)); // Character not found.
		return false;
	}

	if (!pl_sd->sc.data[SC_NOCHAT]) {
		clif->message(sd->fd,msg_fd(fd, MSGTBL_UNMUTE_NOT_MUTED)); // Player is not muted.
		return false;
	}

	pl_sd->status.manner = 0;
	status_change_end(&pl_sd->bl, SC_NOCHAT, INVALID_TIMER);
	clif->message(sd->fd,msg_fd(fd, MSGTBL_UNMUTE_SUCCESS)); // Player unmuted.

	return true;
}

/*==========================================
 * @uptime by MC Cameri
 *------------------------------------------*/
ACMD(uptime)
{
	unsigned long seconds = 0, day = 24*60*60, hour = 60*60,
	minute = 60, days = 0, hours = 0, minutes = 0;

	seconds = timer->get_uptime();
	days = seconds/day;
	seconds -= (seconds/day>0)?(seconds/day)*day:0;
	hours = seconds/hour;
	seconds -= (seconds/hour>0)?(seconds/hour)*hour:0;
	minutes = seconds/minute;
	seconds -= (seconds/minute>0)?(seconds/minute)*minute:0;

	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_UPTIME), days, hours, minutes, seconds);
	clif->message(fd, atcmd_output);

	return true;
}

/*==========================================
 * @changesex <sex>
 * => Changes one's sex. Argument sex can be 0 or 1, m or f, male or female.
 *------------------------------------------*/
ACMD(changesex)
{
	int i;

	pc->resetskill(sd, PCRESETSKILL_CHSEX);
	// to avoid any problem with equipment and invalid sex, equipment is unequipped.
	for (i=0; i<EQI_MAX; i++)
		if (sd->equip_index[i] >= 0) pc->unequipitem(sd, sd->equip_index[i], PCUNEQUIPITEM_RECALC|PCUNEQUIPITEM_FORCE);
	chrif->changesex(sd, true);
	return true;
}

ACMD(changecharsex)
{
	int i;

	pc->resetskill(sd, PCRESETSKILL_CHSEX);
	// to avoid any problem with equipment and invalid sex, equipment is unequipped.
	for (i=0; i<EQI_MAX; i++)
		if (sd->equip_index[i] >= 0) pc->unequipitem(sd, sd->equip_index[i], PCUNEQUIPITEM_RECALC|PCUNEQUIPITEM_FORCE);
	chrif->changesex(sd, false);
	return true;
}

ACMD(unequipall)
{
	if (pc_isvending(sd))
		return false;

	if (pc_isdead(sd)) {
		clif->clearunit_area(&sd->bl,CLR_DEAD);
		return false;
	}

	if (sd->npc_id) {
		if ((sd->npc_item_flag & ITEMENABLEDNPC_EQUIP) == 0 && sd->state.using_megaphone == 0)
			return false;
	} else if (sd->state.storage_flag != STORAGE_FLAG_CLOSED || sd->sc.opt1) {
		; //You can equip/unequip stuff while storage is open/under status changes
	} else if (pc_cant_act2(sd) || sd->state.prerefining)
		return false;

	pc->update_idle_time(sd, BCIDLE_USEITEM);

	int eqiLast = EQI_MAX;
	if (strcmp(message, "basic") == 0) {
		eqiLast = EQI_COSTUME_TOP;
	}

	for (int i = 0; i < eqiLast; i++) {
		if (sd->equip_index[i] >= 0)
			pc->unequipitem(sd, sd->equip_index[i], PCUNEQUIPITEM_RECALC | PCUNEQUIPITEM_FORCE);
	}
	return true;
}

/*================================================
 * @mute - Mutes a player for a set amount of time
 *------------------------------------------------*/
ACMD(mute)
{
	struct map_session_data *pl_sd = NULL;
	int manner;

	if (!*message || sscanf(message, "%12d %23[^\n]", &manner, atcmd_player_name) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_MUTE_USAGE)); // Usage: @mute <time> <char name>
		return false;
	}

	if ((pl_sd = map->nick2sd(atcmd_player_name, true)) == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_CHARACTER_NOT_FOUND)); // Character not found.
		return false;
	}

	if (pc_get_group_level(sd) < pc_get_group_level(pl_sd))
	{
		clif->message(fd, msg_fd(fd, MSGTBL_GM_LEVEL_UNAUTHORIZED)); // Your GM level don't authorize you to do this action on this player.
		return false;
	}

	clif->manner_message(sd, 0);
	clif->manner_message(pl_sd, 5);

	if (pl_sd->status.manner < manner) {
		pl_sd->status.manner -= manner;
		sc_start(NULL, &pl_sd->bl, SC_NOCHAT, 100, 0, 0, 0);
	} else {
		pl_sd->status.manner = 0;
		status_change_end(&pl_sd->bl, SC_NOCHAT, INVALID_TIMER);
	}

	clif->GM_silence(sd, pl_sd, (manner > 0 ? 1 : 0));

	return true;
}

/*==========================================
 * @refresh (like @jumpto <<yourself>>)
 *------------------------------------------*/
ACMD(refresh)
{
	if (sd->npc_id > 0)
		return false;

	clif->refresh(sd);
	return true;
}

ACMD(refreshall)
{
	struct map_session_data* iter_sd;
	struct s_mapiterator* iter;

	iter = mapit_getallusers();
	for (iter_sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); iter_sd = BL_UCAST(BL_PC, mapit->next(iter)))
		if (iter_sd->npc_id <= 0)
			clif->refresh(iter_sd);
	mapit->free(iter);
	return true;
}

/*==========================================
 * @identify / @identifyall
 * => GM's magnifier.
 *------------------------------------------*/
ACMD(identify)
{
	int num = 0;
	bool identifyall = (strcmpi(info->command, "identifyall") == 0);

	if (!identifyall) {
		for (int i = 0; i < sd->status.inventorySize; i++) {
			if (sd->status.inventory[i].nameid > 0 && sd->status.inventory[i].identify != 1) {
				num++;
			}
		}
	} else {
		for (int i = 0; i < sd->status.inventorySize; i++) {
			if (sd->status.inventory[i].nameid > 0 && sd->status.inventory[i].identify != 1) {
				skill->identify(sd, i);
				num++;
			}
		}
	}

	if (num == 0)
		clif->message(fd,msg_fd(fd, MSGTBL_IDENTIFY_NO_ITEMS)); // There are no items to appraise.
	else if (!identifyall)
		clif->item_identify_list(sd);

	return true;
}

ACMD(misceffect)
{
	int effect = 0;

	if (!*message)
		return false;
	if (sscanf(message, "%12d", &effect) < 1)
		return false;
	clif->misceffect(&sd->bl,effect);

	return true;
}

/*==========================================
 * MAIL SYSTEM
 *------------------------------------------*/
ACMD(mail)
{
	mail->openmail(sd);
	return true;
}

/*==========================================
 * Show Monster DB Info   v 1.0
 * originally by [Lupus]
 *------------------------------------------*/
ACMD(mobinfo)
{
	unsigned char msize[3][7] = {"Small", "Medium", "Large"};
	unsigned char mrace[12][11] = {"Formless", "Undead", "Beast", "Plant", "Insect", "Fish", "Demon", "Demi-Human", "Angel", "Dragon", "Boss", "Non-Boss"};
	unsigned char melement[10][8] = {"Neutral", "Water", "Earth", "Fire", "Wind", "Poison", "Holy", "Dark", "Ghost", "Undead"};
	char atcmd_output2[CHAT_SIZE_MAX];
	struct item_data *item_data;
	struct mob_db *monster, *mob_array[MAX_SEARCH];
	int count;
	int i, k;

	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(atcmd_output2, '\0', sizeof(atcmd_output2));

	if (!*message) {
		clif->message(fd, msg_fd(fd, MSGTBL_MOBINFO_USAGE)); // Please enter a monster name/ID (usage: @mobinfo <monster_name_or_monster_ID>).
		return false;
	}

	// If monster identifier/name argument is a name
	if ((i = mob->db_checkid(atoi(message)))) {
		mob_array[0] = mob->db(i);
		count = 1;
	} else
		count = mob->db_searchname_array(mob_array, MAX_SEARCH, message, 0);

	if (!count) {
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_MONSTER_ID_NAME)); // Invalid monster ID or name.
		return false;
	}

	if (count > MAX_SEARCH) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_SEARCH_RESULT_OFFSET), MAX_SEARCH, count);
		clif->message(fd, atcmd_output);
		count = MAX_SEARCH;
	}

	for (k = 0; k < count; k++) {
		unsigned int job_exp, base_exp;
		int j;

		monster = mob_array[k];

		job_exp  = monster->job_exp;
		base_exp = monster->base_exp;

#ifdef RENEWAL_EXP
		if( battle_config.atcommand_mobinfo_type ) {
			base_exp = base_exp * pc->level_penalty_mod(monster->lv - sd->status.base_level, monster->status.race, monster->status.mode, 1) / 100;
			job_exp = job_exp * pc->level_penalty_mod(monster->lv - sd->status.base_level, monster->status.race, monster->status.mode, 1) / 100;
		}
#endif

		// stats
		if (monster->mexp)
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_MOBINFO_MVP), monster->name, monster->jname, monster->sprite, monster->vd.class); // MVP Monster: '%s'/'%s'/'%s' (%d)
		else
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_MOBINFO_NORMAL), monster->name, monster->jname, monster->sprite, monster->vd.class); // Monster: '%s'/'%s'/'%s' (%d)
		clif->message(fd, atcmd_output);

		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_MOBINFO_STATS), monster->lv, monster->status.max_hp, base_exp, job_exp, MOB_HIT(monster), MOB_FLEE(monster)); //  Lv:%d  HP:%d  Base EXP:%u  Job EXP:%u  HIT:%d  FLEE:%d
		clif->message(fd, atcmd_output);

		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_MOBINFO_ATTRIBUTES), //  DEF:%d  MDEF:%d  STR:%d  AGI:%d  VIT:%d  INT:%d  DEX:%d  LUK:%d
				monster->status.def, monster->status.mdef, monster->status.str, monster->status.agi,
				monster->status.vit, monster->status.int_, monster->status.dex, monster->status.luk);
		clif->message(fd, atcmd_output);

#ifdef RENEWAL
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_MOBINFO_ADDITIONAL_INFO), //  ATK : %d~%d MATK : %d~%d Range : %d~%d~%d  Size : %s  Race : %s  Element : %s(Lv : %d)
				MOB_ATK1(monster), MOB_ATK2(monster), MOB_MATK1(monster), MOB_MATK2(monster), monster->status.rhw.range,
				monster->range2 , monster->range3, msize[monster->status.size],
				mrace[monster->status.race], melement[monster->status.def_ele], monster->status.ele_lv);
#else
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_MOBINFO_ATTACK), //  ATK:%d~%d  Range:%d~%d~%d  Size:%s  Race: %s  Element: %s (Lv:%d)
				monster->status.rhw.atk, monster->status.rhw.atk2, monster->status.rhw.range,
				monster->range2 , monster->range3, msize[monster->status.size],
				mrace[monster->status.race], melement[monster->status.def_ele], monster->status.ele_lv);
#endif
		clif->message(fd, atcmd_output);

		// drops
		clif->message(fd, msg_fd(fd, MSGTBL_MOBINFO_DROPS_HEADER)); //  Drops:
		strcpy(atcmd_output, " ");
		j = 0;
		for (i = 0; i < MAX_MOB_DROP; i++) {
			int droprate;

			if (monster->dropitem[i].nameid <= 0 || monster->dropitem[i].p < 1 || (item_data = itemdb->exists(monster->dropitem[i].nameid)) == NULL)
				continue;

			droprate = monster->dropitem[i].p;

#ifdef RENEWAL_DROP
			if( battle_config.atcommand_mobinfo_type ) {
				droprate = droprate * pc->level_penalty_mod(monster->lv - sd->status.base_level, monster->status.race, monster->status.mode, 2) / 100;

				if (droprate <= 0 && !battle_config.drop_rate0item)
					droprate = 1;
			}
#endif

			if (item_data->slot)
				snprintf(atcmd_output2, sizeof(atcmd_output2), " - %s[%d]  %02.02f%%", item_data->jname, item_data->slot, (float)droprate / 100);
			else
				snprintf(atcmd_output2, sizeof(atcmd_output2), " - %s  %02.02f%%", item_data->jname, (float)droprate / 100);

			strcat(atcmd_output, atcmd_output2);

			if (++j % 3 == 0) {
				clif->message(fd, atcmd_output);
				strcpy(atcmd_output, " ");
			}
		}

		if (j == 0)
			clif->message(fd, msg_fd(fd, MSGTBL_MOBINFO_NO_DROPS)); // This monster has no drops.
		else if (j % 3 != 0)
			clif->message(fd, atcmd_output);
		// mvp
		if (monster->mexp) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_MOBINFO_MVP_BONUS_EXP), monster->mexp); //  MVP Bonus EXP:%u
			clif->message(fd, atcmd_output);

			safestrncpy(atcmd_output, msg_fd(fd, MSGTBL_MOBINFO_MVP_ITEMS_HEADER), sizeof(atcmd_output)); //  MVP Items:
			j = 0;
			for (i = 0; i < MAX_MVP_DROP; i++) {
				if (monster->mvpitem[i].nameid <= 0 || (item_data = itemdb->exists(monster->mvpitem[i].nameid)) == NULL)
					continue;
				if (monster->mvpitem[i].p > 0) {
					j++;
					if(item_data->slot)
						snprintf(atcmd_output2, sizeof(atcmd_output2), " %s%s[%d]  %02.02f%%", j != 1 ? "- " : "", item_data->jname, item_data->slot, (float)monster->mvpitem[i].p / 100);
					else
						snprintf(atcmd_output2, sizeof(atcmd_output2), " %s%s  %02.02f%%", j != 1 ? "- " : "", item_data->jname, (float)monster->mvpitem[i].p / 100);
					strcat(atcmd_output, atcmd_output2);
				}
			}
			if (j == 0)
				clif->message(fd, msg_fd(fd, MSGTBL_MOBINFO_NO_MVP_PRIZES)); // This monster has no MVP prizes.
			else
				clif->message(fd, atcmd_output);
		}
	}
	return true;
}

/*=========================================
 * @showmobs by KarLaeda
 * => For 15 sec displays the mobs on minimap
 *------------------------------------------*/
ACMD(showmobs)
{
	char mob_name[100];
	int mob_id;
	int number = 0;
	struct s_mapiterator *it;
	const struct mob_data *md = NULL;

	if (sscanf(message, "%99[^\n]", mob_name) < 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_SHOWMOBS_USAGE)); // Please enter a mob name/id (usage: @showmobs <mob name/id>)
		return false;
	}

	if( (mob_id = atoi(mob_name)) == 0 )
		mob_id = mob->db_searchname(mob_name);

	if( mob_id == 0 ) {
		snprintf(atcmd_output, sizeof atcmd_output, msg_fd(fd, MSGTBL_SHOMOBS_INVALID_NAME), mob_name); // Invalid mob name %s!
		clif->message(fd, atcmd_output);
		return false;
	}

	if(mob_id > 0 && mob->db_checkid(mob_id) == 0){
		snprintf(atcmd_output, sizeof atcmd_output, msg_fd(fd, MSGTBL_SHOWMOBS_INVALID_ID),mob_name); // Invalid mob id %s!
		clif->message(fd, atcmd_output);
		return false;
	}

	if (mob->db(mob_id)->status.mode&MD_BOSS && !pc_has_permission(sd, PC_PERM_SHOW_BOSS)) {
		// If player group does not have access to boss mobs.
		clif->message(fd, msg_fd(fd, MSGTBL_SHOWMOBS_BOSS_MOBS)); // Can't show boss mobs!
		return false;
	}

	if (mob_id == atoi(mob_name)) {
		strcpy(mob_name,mob->db(mob_id)->jname); // DEFAULT_MOB_JNAME
		//strcpy(mob_name,mob->db(mob_id)->name); // DEFAULT_MOB_NAME
	}

	snprintf(atcmd_output, sizeof atcmd_output, msg_fd(fd, MSGTBL_SHOWMOBS_SEARCH_RESULT), // Mob Search... %s %s
			 mob_name, mapindex_id2name(sd->mapindex));
	clif->message(fd, atcmd_output);

	it = mapit_geteachmob();
	for (md = BL_UCCAST(BL_MOB, mapit->first(it)); mapit->exists(it); md = BL_UCCAST(BL_MOB, mapit->next(it))) {
		if( md->bl.m != sd->bl.m )
			continue;
		if( mob_id != -1 && md->class_ != mob_id )
			continue;
		if (md->special_state.ai != AI_NONE || md->master_id)
			continue; // hide slaves and player summoned mobs
		if( md->spawn_timer != INVALID_TIMER )
			continue; // hide mobs waiting for respawn

		++number;
		clif->viewpoint(sd, 1, 0, md->bl.x, md->bl.y, number, 0xFFFFFF);
	}
	mapit->free(it);

	return true;
}

/*==========================================
 * homunculus level up [orn]
 *------------------------------------------*/
ACMD(homlevel)
{
	struct homun_data *hd;
	int level = 0;
	enum homun_type htype;

	if (!*message || ( level = atoi(message) ) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_HOMUN_ENTER_LV_ADJUST)); // Please enter a level adjustment (usage: @homlevel <number of levels>).
		return false;
	}

	if (!homun_alive(sd->hd)) {
		clif->message(fd, msg_fd(fd, MSGTBL_HOMUN_NOT_EXIST)); // You do not have a homunculus.
		return false;
	}

	hd = sd->hd;

	if ((htype = homun->class2type(hd->homunculus.class_)) == HT_INVALID) {
		ShowError("atcommand_homlevel: invalid homun class %d (player %s)\n", hd->homunculus.class_,sd->status.name);
		return false;
	}


	if (hd->homunculus.level >= homun->get_max_level(hd)) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_HOMUNCULU_MAX_LV), hd->homunculus.level); // Homun reached its maximum level of '%d'
		clif->message(fd, atcmd_output);
		return true;
	}

	do {
		hd->homunculus.exp += hd->exp_next;
	} while( hd->homunculus.level < level && homun->levelup(hd) );

	status_calc_homunculus(hd,SCO_NONE);
	status_percent_heal(&hd->bl, 100, 100);
	clif->specialeffect(&hd->bl,568,AREA);
	return true;
}

/*==========================================
 * homunculus evolution H [orn]
 *------------------------------------------*/
ACMD(homevolution)
{
	if ( !homun_alive(sd->hd) ) {
		clif->message(fd, msg_fd(fd, MSGTBL_HOMUN_NOT_EXIST)); // You do not have a homunculus.
		return false;
	}

	if ( !homun->evolve(sd->hd) ) {
		clif->message(fd, msg_fd(fd, MSGTBL_HOMUN_NO_EVOLVE)); // Your homunculus doesn't evolve.
		return false;
	}
	clif->homskillinfoblock(sd);
	return true;
}

ACMD(hommutate)
{
	int homun_id;
	enum homun_type m_class, m_id;

	if (!homun_alive(sd->hd)) {
		clif->message(fd, msg_fd(fd, MSGTBL_HOMUN_NOT_EXIST)); // You do not have a homunculus.
		return false;
	}

	if (*message == '\0') {
		homun_id = HOMID_EIRA + (rnd() % 4);
	} else {
		homun_id = atoi(message);
	}

	m_class = homun->class2type(sd->hd->homunculus.class_);
	m_id    = homun->class2type(homun_id);

	if (m_class == HT_EVO && m_id == HT_S && sd->hd->homunculus.level >= 99) {
		homun->mutate(sd->hd, homun_id);
	} else {
		clif->emotion(&sd->hd->bl, E_SWT);
	}
	return true;
}

/*==========================================
 * call choosen homunculus [orn]
 *------------------------------------------*/
ACMD(makehomun)
{
	int homunid;

	if (!*message) {
		clif->message(fd, msg_fd(fd, MSGTBL_MAKEHOMUN_USAGE)); // Please enter a homunculus ID (usage: @makehomun <homunculus id>).
		return false;
	}

	homunid = atoi(message);

	if (homunid == -1 && sd->status.hom_id && !(homun_alive(sd->hd))) {
		if (!sd->hd)
			homun->call(sd);
		else if( sd->hd->homunculus.vaporize )
			homun->ressurect(sd, 100, sd->bl.x, sd->bl.y);
		else
			homun->call(sd);
		return true;
	}

	if (sd->status.hom_id) {
		clif->message(fd, msg_fd(fd, MSGTBL_ALREADY_HAVE_HOMUN));
		return false;
	}

	if (homunid < HM_CLASS_BASE || homunid > HM_CLASS_BASE + MAX_HOMUNCULUS_CLASS - 1)
	{
		clif->message(fd, msg_fd(fd, MSGTBL_MAKEHOMUN_INVALID_ID)); // Invalid Homunculus ID.
		return false;
	}

	homun->creation_request(sd,homunid);
	return true;
}

/*==========================================
 * modify homunculus intimacy [orn]
 *------------------------------------------*/
ACMD(homfriendly)
{
	int friendly = 0;

	if (!homun_alive(sd->hd)) {
		clif->message(fd, msg_fd(fd, MSGTBL_HOMUN_NOT_EXIST)); // You do not have a homunculus.
		return false;
	}

	if (!*message) {
		clif->message(fd, msg_fd(fd, MSGTBL_HOMFRIENDLY_USAGE)); // Please enter a friendly value (usage: @homfriendly <friendly value [0-1000]>).
		return false;
	}

	friendly = atoi(message);
	friendly = cap_value(friendly, 0, 1000);

	sd->hd->homunculus.intimacy = friendly * 100 ;
	clif->send_homdata(sd,SP_INTIMATE,friendly);
	return true;
}

/*==========================================
 * modify homunculus hunger [orn]
 *------------------------------------------*/
ACMD(homhungry)
{
	int hungry = 0;

	if (!homun_alive(sd->hd)) {
		clif->message(fd, msg_fd(fd, MSGTBL_HOMUN_NOT_EXIST)); // You do not have a homunculus.
		return false;
	}

	if (!*message) {
		clif->message(fd, msg_fd(fd, MSGTBL_HOMHUNGRY_USAGE)); // Please enter a hunger value (usage: @homhungry <hunger value [0-100]>).
		return false;
	}

	hungry = atoi(message);
	hungry = cap_value(hungry, 0, 100);

	sd->hd->homunculus.hunger = hungry;
	clif->send_homdata(sd,SP_HUNGRY,hungry);
	return true;
}

/*==========================================
 * make the homunculus speak [orn]
 *------------------------------------------*/
ACMD(homtalk)
{
	char mes[100], temp[200];

	if (battle_config.min_chat_delay) {
		if (DIFF_TICK(sd->cantalk_tick, timer->gettick()) > 0)
			return true;
		sd->cantalk_tick = timer->gettick() + battle_config.min_chat_delay;
	}

	if (!pc->can_talk(sd))
		return false;

	if (!homun_alive(sd->hd)) {
		clif->message(fd, msg_fd(fd, MSGTBL_HOMUN_NOT_EXIST)); // You do not have a homunculus.
		return false;
	}

	if (!*message || sscanf(message, "%99[^\n]", mes) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_HOMTALK_USAGE)); // Please enter a message (usage: @homtalk <message>).
		return false;
	}

	snprintf(temp, sizeof temp ,"%s : %s", sd->hd->homunculus.name, mes);
	clif->disp_overhead(&sd->hd->bl, temp, AREA_CHAT_WOC, NULL);

	return true;
}

/*==========================================
 * Show homunculus stats
 *------------------------------------------*/
ACMD(hominfo)
{
	struct homun_data *hd;
	struct status_data *st;

	if (!homun_alive(sd->hd)) {
		clif->message(fd, msg_fd(fd, MSGTBL_HOMUN_NOT_EXIST)); // You do not have a homunculus.
		return false;
	}

	hd = sd->hd;
	st = status->get_status_data(&hd->bl);
	clif->message(fd, msg_fd(fd, MSGTBL_HOMINFO_STATS_HEADER)); // Homunculus stats:

	snprintf(atcmd_output, sizeof(atcmd_output) ,msg_fd(fd, MSGTBL_HOMINFO_HP_SP), // HP: %d/%d - SP: %d/%d
			 st->hp, st->max_hp, st->sp, st->max_sp);
	clif->message(fd, atcmd_output);

	snprintf(atcmd_output, sizeof(atcmd_output) ,msg_fd(fd, MSGTBL_HOMINFO_ATK_MATK), // ATK: %d - MATK: %d~%d
			 st->rhw.atk2 +st->batk, st->matk_min, st->matk_max);
	clif->message(fd, atcmd_output);

	snprintf(atcmd_output, sizeof(atcmd_output) ,msg_fd(fd, MSGTBL_HOMINFO_HUNGRY_INTIMACY), // Hungry: %d - Intimacy: %u
			 hd->homunculus.hunger, hd->homunculus.intimacy/100);
	clif->message(fd, atcmd_output);

	snprintf(atcmd_output, sizeof(atcmd_output) ,
			 msg_fd(fd, MSGTBL_HOMINFO_STATS_DETAIL), // Stats: Str %d / Agi %d / Vit %d / Int %d / Dex %d / Luk %d
			 st->str, st->agi, st->vit,
			 st->int_, st->dex, st->luk);
	clif->message(fd, atcmd_output);

	return true;
}

ACMD(homstats)
{
	struct homun_data *hd;
	struct s_homunculus_db *db;
	struct s_homunculus *hom;
	int lv, min, max, evo;

	if (!homun_alive(sd->hd)) {
		clif->message(fd, msg_fd(fd, MSGTBL_HOMUN_NOT_EXIST)); // You do not have a homunculus.
		return false;
	}

	hd = sd->hd;

	hom = &hd->homunculus;
	db = hd->homunculusDB;
	lv = hom->level;

	snprintf(atcmd_output, sizeof(atcmd_output) ,
			 msg_fd(fd, MSGTBL_HOMSTATS_GROWTH), lv, db->name); // Homunculus growth stats (Lv %d %s):
	clif->message(fd, atcmd_output);
	lv--; //Since the first increase is at level 2.

	evo = (hom->class_ == db->evo_class);
	min = db->base.HP +lv*db->gmin.HP +(evo?db->emin.HP:0);
	max = db->base.HP +lv*db->gmax.HP +(evo?db->emax.HP:0);;
	snprintf(atcmd_output, sizeof(atcmd_output) ,msg_fd(fd, MSGTBL_HOMSTATS_MAX_HP), hom->max_hp, min, max); // Max HP: %d (%d~%d)
	clif->message(fd, atcmd_output);

	min = db->base.SP +lv*db->gmin.SP +(evo?db->emin.SP:0);
	max = db->base.SP +lv*db->gmax.SP +(evo?db->emax.SP:0);;
	snprintf(atcmd_output, sizeof(atcmd_output) ,msg_fd(fd, MSGTBL_HOMSTATS_MAX_SP), hom->max_sp, min, max); // Max SP: %d (%d~%d)
	clif->message(fd, atcmd_output);

	min = db->base.str +lv*(db->gmin.str/10) +(evo?db->emin.str:0);
	max = db->base.str +lv*(db->gmax.str/10) +(evo?db->emax.str:0);;
	snprintf(atcmd_output, sizeof(atcmd_output) ,msg_fd(fd, MSGTBL_HOMSTATS_STR), hom->str/10, min, max); // Str: %d (%d~%d)
	clif->message(fd, atcmd_output);

	min = db->base.agi +lv*(db->gmin.agi/10) +(evo?db->emin.agi:0);
	max = db->base.agi +lv*(db->gmax.agi/10) +(evo?db->emax.agi:0);;
	snprintf(atcmd_output, sizeof(atcmd_output) ,msg_fd(fd, MSGTBL_HOMSTATS_AGI), hom->agi/10, min, max); // Agi: %d (%d~%d)
	clif->message(fd, atcmd_output);

	min = db->base.vit +lv*(db->gmin.vit/10) +(evo?db->emin.vit:0);
	max = db->base.vit +lv*(db->gmax.vit/10) +(evo?db->emax.vit:0);;
	snprintf(atcmd_output, sizeof(atcmd_output) ,msg_fd(fd, MSGTBL_HOMSTATS_VIT), hom->vit/10, min, max); // Vit: %d (%d~%d)
	clif->message(fd, atcmd_output);

	min = db->base.int_ +lv*(db->gmin.int_/10) +(evo?db->emin.int_:0);
	max = db->base.int_ +lv*(db->gmax.int_/10) +(evo?db->emax.int_:0);;
	snprintf(atcmd_output, sizeof(atcmd_output) ,msg_fd(fd, MSGTBL_HOMSTATS_INT), hom->int_/10, min, max); // Int: %d (%d~%d)
	clif->message(fd, atcmd_output);

	min = db->base.dex +lv*(db->gmin.dex/10) +(evo?db->emin.dex:0);
	max = db->base.dex +lv*(db->gmax.dex/10) +(evo?db->emax.dex:0);;
	snprintf(atcmd_output, sizeof(atcmd_output) ,msg_fd(fd, MSGTBL_HOMSTATS_DEX), hom->dex/10, min, max); // Dex: %d (%d~%d)
	clif->message(fd, atcmd_output);

	min = db->base.luk +lv*(db->gmin.luk/10) +(evo?db->emin.luk:0);
	max = db->base.luk +lv*(db->gmax.luk/10) +(evo?db->emax.luk:0);;
	snprintf(atcmd_output, sizeof(atcmd_output) ,msg_fd(fd, MSGTBL_HOMSTATS_LUK), hom->luk/10, min, max); // Luk: %d (%d~%d)
	clif->message(fd, atcmd_output);

	return true;
}

ACMD(homshuffle)
{
	if (!sd->hd)
		return false; // nothing to do

	if (!homun->shuffle(sd->hd))
		return false;

	clif->message(sd->fd, msg_fd(fd, MSGTBL_HOMSHUFFLE_ALTERED)); // Homunculus stats altered.
	atcommand_homstats(fd, sd, command, message, info); //Print out the new stats
	return true;
}

/*==========================================
 * Show Items DB Info   v 1.0
 * originally by [Lupus]
 *------------------------------------------*/
ACMD(iteminfo)
{
	struct item_data *item_array[MAX_SEARCH];
	int i, count = 1;

	if (!*message) {
		clif->message(fd, msg_fd(fd, MSGTBL_ITEMINFO_USAGE)); // Please enter an item name/ID (usage: @ii/@iteminfo <item name/ID>).
		return false;
	}
	if ((item_array[0] = itemdb->exists(atoi(message))) == NULL)
		count = itemdb->search_name_array(item_array, MAX_SEARCH, message, IT_SEARCH_NAME_PARTIAL);

	if (!count) {
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_ITEMID_NAME)); // Invalid item ID or name.
		return false;
	}

	if (count > MAX_SEARCH) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_SEARCH_RESULT_OFFSET), MAX_SEARCH, count); // Displaying first %d out of %d matches
		clif->message(fd, atcmd_output);
		count = MAX_SEARCH;
	}
	StringBuf buf;
	StrBuf->Init(&buf);
	for (i = 0; i < count; i++) {
		struct item_data *item_data = item_array[i];
		if (item_data != NULL) {

			struct item link_item = { 0 };
			link_item.nameid = item_data->nameid;
			clif->format_itemlink(&buf, &link_item);

			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_ITEMINFO_DETAILS), // Item: '%s'/'%s' (%d) Type: %s | Extra Effect: %s
				item_data->name, StrBuf->Value(&buf), item_data->nameid,
				itemdb->typename(item_data->type),
				(item_data->script == NULL) ? msg_fd(fd, MSGTBL_ITEMINFO_NONE) : msg_fd(fd, MSGTBL_ITEMINFO_WITH_SCRIPT) // None / With script
			);
			StrBuf->Clear(&buf);
			clif->message(fd, atcmd_output);

			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_ITEMINFO_NPC_DETAILS), item_data->value_buy, item_data->value_sell, item_data->weight / 10.); // NPC Buy:%dz, Sell:%dz | Weight: %.1f
			clif->message(fd, atcmd_output);

			if (item_data->maxchance == -1)
				safestrncpy(atcmd_output, msg_fd(fd, MSGTBL_ITEMINFO_SHOPS_ONLY), sizeof(atcmd_output)); //  - Available in the shops only.
			else if (!battle_config.atcommand_mobinfo_type) {
				if (item_data->maxchance)
					snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_ITEMINFO_MAX_DROP_CHANCE), (float)item_data->maxchance / 100); //  - Maximal monsters drop chance: %02.02f%%
				else
					safestrncpy(atcmd_output, msg_fd(fd, MSGTBL_ITEMINFO_NO_MONSTER_DROP), sizeof(atcmd_output)); //  - Monsters don't drop this item.
			}
			clif->message(fd, atcmd_output);
		}
	}
	StrBuf->Destroy(&buf);
	return true;
}

/*==========================================
 * Show who drops the item.
 *------------------------------------------*/
ACMD(whodrops)
{
	struct item_data *item_array[MAX_SEARCH];
	int i, j, count = 1;

	if (!*message) {
		clif->message(fd, msg_fd(fd, MSGTBL_WHODROPS_USAGE)); // Please enter item name/ID (usage: @whodrops <item name/ID>).
		return false;
	}
	if ((item_array[0] = itemdb->exists(atoi(message))) == NULL)
		count = itemdb->search_name_array(item_array, MAX_SEARCH, message, IT_SEARCH_NAME_PARTIAL);

	if (!count) {
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_ITEMID_NAME)); // Invalid item ID or name.
		return false;
	}

	if (count > MAX_SEARCH) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_SEARCH_RESULT_OFFSET), MAX_SEARCH, count); // Displaying first %d out of %d matches
		clif->message(fd, atcmd_output);
		count = MAX_SEARCH;
	}
	for (i = 0; i < count; i++) {
		struct item_data *item_data = item_array[i];
		if (item_data != NULL) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_WHODROPS_ITEM), item_data->jname, item_data->slot); // Item: '%s'[%d]
			clif->message(fd, atcmd_output);

			if (item_data->mob[0].chance == 0) {
				safestrncpy(atcmd_output, msg_fd(fd, MSGTBL_WHODROPS_NO_DROP), sizeof(atcmd_output)); //  - Item is not dropped by mobs.
				clif->message(fd, atcmd_output);
			} else {
				snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_WHODROPS_COMMON_MOBS), MAX_SEARCH); //  - Common mobs with highest drop chance (only max %d are listed):
				clif->message(fd, atcmd_output);

				for (j = 0; j < MAX_SEARCH && item_data->mob[j].chance > 0; j++) {
					snprintf(atcmd_output, sizeof(atcmd_output), "- %s (%02.02f%%)", mob->db(item_data->mob[j].id)->jname, item_data->mob[j].chance / 100.);
					clif->message(fd, atcmd_output);
				}
			}
		}
	}
	return true;
}

ACMD(whereis)
{
	struct mob_db *mob_array[MAX_SEARCH];
	int count;
	int i, j, k;

	if (!*message) {
		clif->message(fd, msg_fd(fd, MSGTBL_WHEREIS_USAGE)); // Please enter a monster name/ID (usage: @whereis <monster_name_or_monster_ID>).
		return false;
	}

	// If monster identifier/name argument is a name
	if ((i = mob->db_checkid(atoi(message))))
	{
		mob_array[0] = mob->db(i);
		count = 1;
	} else
		count = mob->db_searchname_array(mob_array, MAX_SEARCH, message, 0);

	if (!count) {
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_MONSTER_ID_NAME)); // Invalid monster ID or name.
		return false;
	}

	if (count > MAX_SEARCH) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_SEARCH_RESULT_OFFSET), MAX_SEARCH, count);
		clif->message(fd, atcmd_output);
		count = MAX_SEARCH;
	}
	for (k = 0; k < count; k++) {
		struct mob_db *monster = mob_array[k];
		snprintf(atcmd_output, sizeof atcmd_output, msg_fd(fd, MSGTBL_WHEREIS_SPAWNS_IN), monster->jname); // %s spawns in:
		clif->message(fd, atcmd_output);

		for (i = 0; i < ARRAYLENGTH(monster->spawn) && monster->spawn[i].qty; i++) {
			j = map->mapindex2mapid(monster->spawn[i].mapindex);
			if (j < 0) continue;
			snprintf(atcmd_output, sizeof atcmd_output, "%s (%d)", map->list[j].name, monster->spawn[i].qty);
			clif->message(fd, atcmd_output);
		}
		if (i == 0)
			clif->message(fd, msg_fd(fd, MSGTBL_WHEREIS_NO_SPAWN)); // This monster does not spawn normally.
	}

	return true;
}

ACMD(version)
{
	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_HERCULES_BIT_INFO), sysinfo->is64bit() ? 64 : 32, sysinfo->platform()); // Hercules %d-bit for %s
	clif->message(fd, atcmd_output);
	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_VERSION_REVISION_INFO), sysinfo->vcstype(), sysinfo->vcsrevision_src(), sysinfo->vcsrevision_scripts()); // %s revision '%s' (src) / '%s' (scripts)
	clif->message(fd, atcmd_output);

	return true;
}

/*==========================================
 * @mutearea by MouseJstr
 *------------------------------------------*/
static int atcommand_mutearea_sub(struct block_list *bl, va_list ap)
{
	// As it is being used [ACMD(mutearea)] there's no need to be a bool, but if there's need to reuse it, it's better to be this way

	int time, id;
	struct map_session_data *pl_sd = BL_CAST(BL_PC, bl);
	if (pl_sd == NULL)
		return 0;

	id = va_arg(ap, int);
	time = va_arg(ap, int);

	if (id != bl->id && !pc_get_group_level(pl_sd)) {
		pl_sd->status.manner -= time;
		if (pl_sd->status.manner < 0)
			sc_start(NULL, &pl_sd->bl, SC_NOCHAT, 100, 0, 0, 0);
		else
			status_change_end(&pl_sd->bl, SC_NOCHAT, INVALID_TIMER);
	}
	return 1;
}

ACMD(mutearea)
{
	int time;

	if (!*message) {
		clif->message(fd, msg_fd(fd, MSGTBL_MUTEAREA_USAGE)); // Please enter a time in minutes (usage: @mutearea/@stfu <time in minutes>).
		return false;
	}

	time = atoi(message);

	map->foreachinarea(atcommand->mutearea_sub,sd->bl.m,
	                   sd->bl.x-AREA_SIZE, sd->bl.y-AREA_SIZE,
	                   sd->bl.x+AREA_SIZE, sd->bl.y+AREA_SIZE, BL_PC, sd->bl.id, time);

	return true;
}

ACMD(rates)
{
	char buf[CHAT_SIZE_MAX];

	memset(buf, '\0', sizeof(buf));

	snprintf(buf, CHAT_SIZE_MAX, msg_fd(fd, MSGTBL_RATES_EXP), // Experience rates: Base %.2fx / Job %.2fx
			 battle_config.base_exp_rate/100., battle_config.job_exp_rate/100.);
	clif->message(fd, buf);
	snprintf(buf, CHAT_SIZE_MAX, msg_fd(fd, MSGTBL_RATES_NORMAL_DROP), // Normal Drop Rates: Common %.2fx / Healing %.2fx / Usable %.2fx / Equipment %.2fx / Card %.2fx
			 battle_config.item_rate_common/100., battle_config.item_rate_heal/100., battle_config.item_rate_use/100., battle_config.item_rate_equip/100., battle_config.item_rate_card/100.);
	clif->message(fd, buf);
	snprintf(buf, CHAT_SIZE_MAX, msg_fd(fd, MSGTBL_RATES_BOSS_DROP), // Boss Drop Rates: Common %.2fx / Healing %.2fx / Usable %.2fx / Equipment %.2fx / Card %.2fx
			 battle_config.item_rate_common_boss/100., battle_config.item_rate_heal_boss/100., battle_config.item_rate_use_boss/100., battle_config.item_rate_equip_boss/100., battle_config.item_rate_card_boss/100.);
	clif->message(fd, buf);
	snprintf(buf, CHAT_SIZE_MAX, msg_fd(fd, MSGTBL_RATES_OTHER_DROP), // Other Drop Rates: MvP %.2fx / Card-Based %.2fx / Treasure %.2fx
			 battle_config.item_rate_mvp/100., battle_config.item_rate_adddrop/100., battle_config.item_rate_treasure/100.);
	clif->message(fd, buf);

	return true;
}

/*==========================================
 * @me by lordalfa
 * => Displays the OUTPUT string on top of the Visible players Heads.
 *------------------------------------------*/
ACMD(me)
{
	char tempmes[CHAT_SIZE_MAX];

	memset(tempmes, '\0', sizeof(tempmes));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!pc->can_talk(sd))
		return false;

	if (!*message || sscanf(message, "%199[^\n]", tempmes) < 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_ME_USAGE)); // Please enter a message (usage: @me <message>).
		return false;
	}

	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_ATCMD_ME_OUTPUT_FORMAT), sd->status.name, tempmes); // *%s %s*
	clif->disp_overhead(&sd->bl, atcmd_output, AREA_CHAT_WOC, NULL);

	return true;
}

/*==========================================
 * @size
 * => Resize your character sprite. [Valaris]
 *------------------------------------------*/
ACMD(size)
{
	int size = 0;

	size = cap_value(atoi(message),SZ_SMALL,SZ_BIG);

	if(sd->state.size) {
		sd->state.size = SZ_SMALL;
		pc->setpos(sd, sd->mapindex, sd->bl.x, sd->bl.y, CLR_TELEPORT);
	}

	sd->state.size = size;
	if( size == SZ_MEDIUM )
		clif->specialeffect(&sd->bl,420,AREA);
	else if( size == SZ_BIG )
		clif->specialeffect(&sd->bl,422,AREA);

	clif->message(fd, msg_fd(fd, MSGTBL_SIZE_CHANGE_APPLIED)); // Size change applied.
	return true;
}

ACMD(sizeall)
{
	int size;
	struct map_session_data *pl_sd;
	struct s_mapiterator* iter;

	size = atoi(message);
	size = cap_value(size,0,2);

	iter = mapit_getallusers();
	for (pl_sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); pl_sd = BL_UCAST(BL_PC, mapit->next(iter))) {
		if (pl_sd->state.size != size) {
			if (pl_sd->state.size) {
				pl_sd->state.size = SZ_SMALL;
				pc->setpos(pl_sd, pl_sd->mapindex, pl_sd->bl.x, pl_sd->bl.y, CLR_TELEPORT);
			}

			pl_sd->state.size = size;
			if (size == SZ_MEDIUM)
				clif->specialeffect(&pl_sd->bl,420,AREA);
			else if (size == SZ_BIG)
				clif->specialeffect(&pl_sd->bl,422,AREA);
		}
	}
	mapit->free(iter);

	clif->message(fd, msg_fd(fd, MSGTBL_SIZE_CHANGE_APPLIED)); // Size change applied.
	return true;
}

ACMD(sizeguild)
{
	int size = 0, i;
	char guild_name[NAME_LENGTH];
	struct map_session_data *pl_sd;
	struct guild *g;

	memset(guild_name, '\0', sizeof(guild_name));

	if (!*message || sscanf(message, "%d %23[^\n]", &size, guild_name) < 2) {
		clif->message(fd, msg_fd(fd, MSGTBL_SIZEGUILD_USAGE)); // Please enter guild name/ID (usage: @sizeguild <size> <guild name/ID>).
		return false;
	}

	if ((g = guild->searchname(guild_name)) == NULL && (g = guild->search(atoi(guild_name))) == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_RECALL_INVALID_GUILD_NAME)); // Incorrect name/ID, or no one from the guild is online.
		return false;
	}

	size = cap_value(size,SZ_SMALL,SZ_BIG);

	for (i = 0; i < g->max_member; i++) {
		if ((pl_sd = g->member[i].sd) && pl_sd->state.size != size) {
			if( pl_sd->state.size ) {
				pl_sd->state.size = SZ_SMALL;
				pc->setpos(pl_sd, pl_sd->mapindex, pl_sd->bl.x, pl_sd->bl.y, CLR_TELEPORT);
			}

			pl_sd->state.size = size;
			if( size == SZ_MEDIUM )
				clif->specialeffect(&pl_sd->bl,420,AREA);
			else if( size == SZ_BIG )
				clif->specialeffect(&pl_sd->bl,422,AREA);
		}
	}

	clif->message(fd, msg_fd(fd, MSGTBL_SIZE_CHANGE_APPLIED)); // Size change applied.
	return true;
}

/*==========================================
 * @monsterignore
 * => Makes monsters ignore you. [Valaris]
 *------------------------------------------*/
ACMD(monsterignore)
{
	if (!sd->block_action.immune) {
		sd->block_action.immune = 1;
		clif->message(sd->fd, msg_fd(fd, MSGTBL_MONSTERIGNORE_IMMUNE)); // You are now immune to attacks.
	} else {
		sd->block_action.immune = 0;
		clif->message(sd->fd, msg_fd(fd, MSGTBL_MONSTERIGNORE_NORMAL_STATE)); // Returned to normal state.
	}

	return true;
}

/**
 * Temporarily changes the character's name to the specified string.
 *
 * @code{.herc}
 *	@fakename {<options>} {<fake_name>}
 * @endcode
 *
 **/
ACMD(fakename)
{
	if (*message == '\0') {
		if (sd->fakename[0] != '\0') {
			sd->fakename[0] = '\0';
			sd->fakename_options = FAKENAME_OPTION_NONE;
			clif->blname_ack(0, &sd->bl);

			if (sd->disguise != 0) // Another packet should be sent so the client updates the name for sd.
				clif->blname_ack(sd->fd, &sd->bl);

			clif->message(sd->fd, msg_fd(fd, MSGTBL_FAKENAME_RETURNED)); // Returned to real name.
			return true;
		}

		clif->message(sd->fd, msg_fd(fd, MSGTBL_FAKENAME_NO_NAME)); // You must enter a name.
		return false;
	}

	int options = FAKENAME_OPTION_NONE;
	char buf[NAME_LENGTH] = {'\0'};
	const char *fake_name = NULL;

	if (sscanf(message, "%d %23[^\n]", &options, buf) == 2) {
		fake_name = buf;
	} else {
		options = FAKENAME_OPTION_NONE;
		fake_name = message;
	}

	if (strlen(fake_name) < 2) {
		clif->message(sd->fd, msg_fd(fd, MSGTBL_FAKENAME_SHORT_NAME)); // Fake name must be at least two characters.
		return false;
	}

	if (options < FAKENAME_OPTION_NONE)
		options = FAKENAME_OPTION_NONE;

	safestrncpy(sd->fakename, fake_name, sizeof(sd->fakename));
	sd->fakename_options = options;
	clif->blname_ack(0, &sd->bl);

	if (sd->disguise != 0) // Another packet should be sent so the client updates the name for sd.
		clif->blname_ack(sd->fd, &sd->bl);

	clif->message(sd->fd, msg_fd(fd, MSGTBL_FAKENAME_ENABLED)); // Fake name enabled.

	return true;
}

/*==========================================
 * Ragnarok Resources
 *------------------------------------------*/
ACMD(mapflag)
{
#define CHECKFLAG(cmd) do { \
	if (map->list[sd->bl.m].flag.cmd != 0) { \
		clif->message(sd->fd, #cmd); \
	} \
} while(0)
#define SETFLAG(cmd) do { \
	if (strcmp(flag_name, #cmd) == 0) { \
		map->list[sd->bl.m].flag.cmd = flag; \
		snprintf(atcmd_output, sizeof(atcmd_output), "[ @mapflag ] %s flag has been set to %s value = %hd", #cmd, (flag ? "On" : "Off"), flag); \
		clif->message(sd->fd, atcmd_output); \
		return true; \
	} \
} while(0)

	char flag_name[100];
	short flag = 0;
	int i = 0;

	memset(flag_name, '\0', sizeof(flag_name));

	if (!*message || (sscanf(message, "%99s %5hd", flag_name, &flag) < 1)) {
		clif->message(sd->fd, msg_fd(fd, MSGTBL_MAPFLAG_ENABLED)); // Enabled Mapflags in this map:
		clif->message(sd->fd, "----------------------------------");

		CHECKFLAG(town);
		CHECKFLAG(autotrade);
		CHECKFLAG(allowks);
		CHECKFLAG(nomemo);
		CHECKFLAG(noteleport);
		CHECKFLAG(noreturn);
		CHECKFLAG(monster_noteleport);
		CHECKFLAG(nosave);
		CHECKFLAG(nobranch);
		CHECKFLAG(noexppenalty);
		CHECKFLAG(pvp);
		CHECKFLAG(pvp_noparty);
		CHECKFLAG(pvp_noguild);
		CHECKFLAG(pvp_nightmaredrop);
		CHECKFLAG(pvp_nocalcrank);
		CHECKFLAG(gvg_castle);
		CHECKFLAG(gvg);
		CHECKFLAG(gvg_dungeon);
		CHECKFLAG(gvg_noparty);
		CHECKFLAG(battleground);
		CHECKFLAG(cvc);
		CHECKFLAG(nozenypenalty);
		CHECKFLAG(notrade);
		CHECKFLAG(noskill);
		CHECKFLAG(nowarp);
		CHECKFLAG(nowarpto);
		CHECKFLAG(noicewall);
		CHECKFLAG(snow);
		CHECKFLAG(clouds);
		CHECKFLAG(clouds2);
		CHECKFLAG(fog);
		CHECKFLAG(fireworks);
		CHECKFLAG(sakura);
		CHECKFLAG(leaves);
		CHECKFLAG(nobaseexp);
		CHECKFLAG(nojobexp);
		CHECKFLAG(nomobloot);
		CHECKFLAG(nomvploot);
		CHECKFLAG(nightenabled);
		CHECKFLAG(nodrop);
		CHECKFLAG(novending);
		CHECKFLAG(loadevent);
		CHECKFLAG(nochat);
		CHECKFLAG(partylock);
		CHECKFLAG(guildlock);
		CHECKFLAG(src4instance);
		CHECKFLAG(reset);
		CHECKFLAG(chsysnolocalaj);
		CHECKFLAG(noknockback);
		CHECKFLAG(notomb);
		CHECKFLAG(nocashshop);
		CHECKFLAG(noautoloot);
		CHECKFLAG(pairship_startable);
		CHECKFLAG(pairship_endable);
		CHECKFLAG(nostorage);
		CHECKFLAG(nogstorage);
		CHECKFLAG(noviewid);
		CHECKFLAG(specialpopup);
		CHECKFLAG(nosendmail);

		clif->message(sd->fd, " ");
		clif->message(sd->fd, msg_fd(fd, MSGTBL_MAPFLAG_USAGE)); // Usage: "@mapflag monster_noteleport 1" (0=Off | 1=On)
		clif->message(sd->fd, msg_fd(fd, MSGTBL_MAPFLAG_TYPE_AVAILABLE)); // Type "@mapflag available" to list the available mapflags.
		return true;
	}

	for (i = 0; flag_name[i] != '\0'; i++)
		flag_name[i] = TOLOWER(flag_name[i]); //lowercase

	if (strcmp(flag_name, "gvg") == 0) {
		if (flag && !map->list[sd->bl.m].flag.gvg)
			map->zone_change2(sd->bl.m, strdb_get(map->zone_db, MAP_ZONE_GVG_NAME));
		else if (!flag && map->list[sd->bl.m].flag.gvg)
			map->zone_change2(sd->bl.m, map->list[sd->bl.m].prev_zone);
	} else if (strcmp(flag_name, "pvp") == 0) {
		if (flag && !map->list[sd->bl.m].flag.pvp)
			map->zone_change2(sd->bl.m, strdb_get(map->zone_db, MAP_ZONE_PVP_NAME));
		else if (!flag && map->list[sd->bl.m].flag.pvp)
			map->zone_change2(sd->bl.m, map->list[sd->bl.m].prev_zone);
	} else if (strcmp(flag_name, "battleground") == 0) {
		if (flag && !map->list[sd->bl.m].flag.battleground)
			map->zone_change2(sd->bl.m, strdb_get(map->zone_db, MAP_ZONE_BG_NAME));
		else if (!flag && map->list[sd->bl.m].flag.battleground)
			map->zone_change2(sd->bl.m, map->list[sd->bl.m].prev_zone);
	} else if (strcmp(flag_name, "cvc") == 0) {
		if (flag && !map->list[sd->bl.m].flag.cvc)
			map->zone_change2(sd->bl.m, strdb_get(map->zone_db, MAP_ZONE_CVC_NAME));
		else if (!flag && map->list[sd->bl.m].flag.cvc)
			map->zone_change2(sd->bl.m, map->list[sd->bl.m].prev_zone);
	}

	SETFLAG(town);
	SETFLAG(autotrade);
	SETFLAG(allowks);
	SETFLAG(nomemo);
	SETFLAG(noteleport);
	SETFLAG(noreturn);
	SETFLAG(monster_noteleport);
	SETFLAG(nosave);
	SETFLAG(nobranch);
	SETFLAG(noexppenalty);
	SETFLAG(pvp);
	SETFLAG(pvp_noparty);
	SETFLAG(pvp_noguild);
	SETFLAG(pvp_nightmaredrop);
	SETFLAG(pvp_nocalcrank);
	SETFLAG(gvg_castle);
	SETFLAG(gvg);
	SETFLAG(gvg_dungeon);
	SETFLAG(gvg_noparty);
	SETFLAG(battleground);
	SETFLAG(cvc);
	SETFLAG(nozenypenalty);
	SETFLAG(notrade);
	SETFLAG(noskill);
	SETFLAG(nowarp);
	SETFLAG(nowarpto);
	SETFLAG(noicewall);
	SETFLAG(snow);
	SETFLAG(clouds);
	SETFLAG(clouds2);
	SETFLAG(fog);
	SETFLAG(fireworks);
	SETFLAG(sakura);
	SETFLAG(leaves);
	SETFLAG(nobaseexp);
	SETFLAG(nojobexp);
	SETFLAG(nomobloot);
	SETFLAG(nomvploot);
	SETFLAG(nightenabled);
	SETFLAG(nodrop);
	SETFLAG(novending);
	SETFLAG(loadevent);
	SETFLAG(nochat);
	SETFLAG(partylock);
	SETFLAG(guildlock);
	SETFLAG(src4instance);
	SETFLAG(reset);
	SETFLAG(chsysnolocalaj);
	SETFLAG(noknockback);
	SETFLAG(notomb);
	SETFLAG(nocashshop);
	SETFLAG(noautoloot);
	SETFLAG(pairship_startable);
	SETFLAG(pairship_endable);
	SETFLAG(nostorage);
	SETFLAG(nogstorage);
	SETFLAG(noviewid);
	SETFLAG(specialpopup);
	SETFLAG(nosendmail);

	clif->message(sd->fd, msg_fd(fd, MSGTBL_MAPFLAG_INVALID_FLAG)); // Invalid flag name or flag.
	clif->message(sd->fd, msg_fd(fd, MSGTBL_MAPFLAG_USAGE)); // Usage: "@mapflag monster_noteleport 1" (0=Off | 1=On)
	clif->message(sd->fd, msg_fd(fd, MSGTBL_MAPFLAG_AVAILABLE_FLAGS)); // Available Flags:
	clif->message(sd->fd, "----------------------------------");
	clif->message(sd->fd, "town, autotrade, allowks, nomemo, noteleport, noreturn, monster_noteleport, nosave,");
	clif->message(sd->fd, "nobranch, noexppenalty, pvp, pvp_noparty, pvp_noguild, pvp_nightmaredrop,");
	clif->message(sd->fd, "pvp_nocalcrank, gvg_castle, gvg, gvg_dungeon, gvg_noparty, battleground, cvc,");
	clif->message(sd->fd, "nozenypenalty, notrade, noskill, nowarp, nowarpto, noicewall, snow, clouds, clouds2,");
	clif->message(sd->fd, "fog, fireworks, sakura, leaves, nobaseexp, nojobexp, nomobloot, nomvploot,");
	clif->message(sd->fd, "nightenabled, nodrop, novending, loadevent, nochat, partylock, guildlock,");
	clif->message(sd->fd, "src4instance, reset, chsysnolocalaj, noknockback, notomb, nocashshop, noautoloot,");
	clif->message(sd->fd, "pairship_startable, pairship_endable, nostorage, nogstorage, noviewid, specialpopup,");
	clif->message(sd->fd, "nosendmail");

#undef CHECKFLAG
#undef SETFLAG

	return true;
}

/*===================================
 * Remove some messages
 *-----------------------------------*/
ACMD(showexp)
{
	if (sd->state.showexp) {
		sd->state.showexp = 0;
		clif->message(fd, msg_fd(fd, MSGTBL_SHOWEXP_NOT_SHOWN)); // Gained exp will not be shown.
		return true;
	}

	sd->state.showexp = 1;
	clif->message(fd, msg_fd(fd, MSGTBL_SHOWEXP_NOW_SHOWN)); // Gained exp is now shown.
	return true;
}

ACMD(showzeny)
{
	if (sd->state.showzeny) {
		sd->state.showzeny = 0;
		clif->message(fd, msg_fd(fd, MSGTBL_SHOWZENY_NOT_SHOWN)); // Gained zeny will not be shown.
		return true;
	}

	sd->state.showzeny = 1;
	clif->message(fd, msg_fd(fd, MSGTBL_SHOWZENY_NOW_SHOWN)); // Gained zeny is now shown.
	return true;
}

ACMD(showdelay)
{
	if (sd->state.showdelay) {
		sd->state.showdelay = 0;
		clif->message(fd, msg_fd(fd, MSGTBL_SHOWDELAY_NOT_SHOWN)); // Skill delay failures will not be shown.
		return true;
	}

	sd->state.showdelay = 1;
	clif->message(fd, msg_fd(fd, MSGTBL_SHOWDELAY_NOW_SHOWN)); // Skill delay failures are now shown.
	return true;
}

/*==========================================
 * Duel organizing functions [LuzZza]
 *
 * @duel [limit|nick] - create a duel
 * @invite <nick> - invite player
 * @accept - accept invitation
 * @reject - reject invitation
 * @leave - leave duel
 *------------------------------------------*/
ACMD(invite)
{
	unsigned int did = sd->duel_group;
	struct map_session_data *target_sd = map->nick2sd(message, true);

	if (did == 0)
	{
		// "Duel: @invite without @duel."
		clif->message(fd, msg_fd(fd, MSGTBL_DUEL_CANT_INVITE));
		return false;
	}

	if (duel->list[did].max_players_limit > 0 &&
	    duel->list[did].members_count >= duel->list[did].max_players_limit) {
		// "Duel: Limit of players is reached."
		clif->message(fd, msg_fd(fd, MSGTBL_DUEL_LIMIT_REACHED));
		return false;
	}

	if (target_sd == NULL) {
		// "Duel: Player not found."
		clif->message(fd, msg_fd(fd, MSGTBL_DUEL_PLAYER_NOT_FOUND));
		return false;
	}

	if (target_sd->duel_group > 0 || target_sd->duel_invite > 0) {
		// "Duel: Player already in duel."
		clif->message(fd, msg_fd(fd, MSGTBL_DUEL_PLAYER_IN_DUEL));
		return false;
	}

	if (battle_config.duel_only_on_same_map && target_sd->bl.m != sd->bl.m)
	{
		// "Duel: You can't invite %s because he/she isn't in the same map."
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_DUEL_PLAYER_NOT_IN_MAP), message);
		clif->message(fd, atcmd_output);
		return false;
	}

	duel->invite(did, sd, target_sd);
	// "Duel: Invitation has been sent."
	clif->message(fd, msg_fd(fd, MSGTBL_DUEL_INVITATION_SENT));
	return true;
}

ACMD(duel)
{
	unsigned int maxpl = 0;

	if (sd->duel_group > 0) {
		duel->showinfo(sd->duel_group, sd);
		return true;
	}

	if (sd->duel_invite > 0) {
		// "Duel: @duel without @reject."
		clif->message(fd, msg_fd(fd, MSGTBL_DUEL_NEEDS_REJECT_FIRST));
		return false;
	}

	int64 diff = duel->difftime(sd);
	if (diff > 0) {
		char output[CHAT_SIZE_MAX];
		// "Duel: You can take part in duel again after %d secconds."
		sprintf(output, msg_fd(fd, MSGTBL_DUEL_COOLDOWN), (int)diff);
		clif->message(fd, output);
		return false;
	}

	if (message[0]) {
		if (sscanf(message, "%12u", &maxpl) >= 1) {
			if (maxpl < 2 || maxpl > 65535) {
				clif->message(fd, msg_fd(fd, MSGTBL_DUEL_INVALID_VALUE)); // "Duel: Invalid value."
				return false;
			}
			duel->create(sd, maxpl);
		} else {
			struct map_session_data *target_sd = map->nick2sd(message, true);
			if (target_sd != NULL) {
				unsigned int newduel;
				if ((newduel = duel->create(sd, 2)) != -1) {
					if (target_sd->duel_group > 0 || target_sd->duel_invite > 0) {
						clif->message(fd, msg_fd(fd, MSGTBL_DUEL_PLAYER_IN_DUEL)); // "Duel: Player already in duel."
						return false;
					}
					duel->invite(newduel, sd, target_sd);
					clif->message(fd, msg_fd(fd, MSGTBL_DUEL_INVITATION_SENT)); // "Duel: Invitation has been sent."
				}
			} else {
				// "Duel: Player not found."
				clif->message(fd, msg_fd(fd, MSGTBL_DUEL_PLAYER_NOT_FOUND));
				return false;
			}
		}
	} else
		duel->create(sd, 0);

	return true;
}

ACMD(leave)
{
	if (sd->duel_group <= 0) {
		// "Duel: @leave without @duel."
		clif->message(fd, msg_fd(fd, MSGTBL_DUEL_CANT_LEAVE));
		return false;
	}
	duel->leave(sd->duel_group, sd);
	clif->message(fd, msg_fd(fd, MSGTBL_DUEL_LEFT)); // "Duel: You left the duel."
	return true;
}

ACMD(accept)
{
	int64 diff = duel->difftime(sd);
	if (diff > 0) {
		char output[CHAT_SIZE_MAX];
		// "Duel: You can take part in duel again after %d seconds."
		sprintf(output, msg_fd(fd, MSGTBL_DUEL_COOLDOWN), (int)diff);
		clif->message(fd, output);
		return false;
	}

	if (sd->duel_invite <= 0) {
		// "Duel: @accept without invitation."
		clif->message(fd, msg_fd(fd, MSGTBL_DUEL_CANT_ACCEPT));
		return false;
	}

	if (duel->list[sd->duel_invite].max_players_limit > 0
	   && duel->list[sd->duel_invite].members_count >= duel->list[sd->duel_invite].max_players_limit) {
		// "Duel: Limit of players is reached."
		clif->message(fd, msg_fd(fd, MSGTBL_DUEL_LIMIT_REACHED));
		return false;
	}

	duel->accept(sd->duel_invite, sd);
	// "Duel: Invitation has been accepted."
	clif->message(fd, msg_fd(fd, MSGTBL_DUEL_INVITATION_ACCEPTED));
	return true;
}

ACMD(reject)
{
	if (sd->duel_invite <= 0) {
		// "Duel: @reject without invitation."
		clif->message(fd, msg_fd(fd, MSGTBL_DUEL_CANT_REJECT));
		return false;
	}

	duel->reject(sd->duel_invite, sd);
	// "Duel: Invitation has been rejected."
	clif->message(fd, msg_fd(fd, MSGTBL_DUEL_INVITATION_REJECTED));
	return true;
}

/*===================================
 * Cash Points
 *-----------------------------------*/
ACMD(cash)
{
	char output[128];
	int value;

	if (!*message || (value = atoi(message)) == 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_CASH_ENTER_AMOUNT)); // Please enter an amount.
		return false;
	}

	if (!strcmpi(info->command,"cash")) {
		if (value > 0) {
			if ((pc->getcash(sd, value, 0)) >= 0) {
				// If this option is set, the message is already sent by pc function
				if (!battle_config.cashshop_show_points) {
					sprintf(output, msg_fd(fd, MSGTBL_GAINED_CASHPOINTS), value, sd->cashPoints);
					clif_disp_onlyself(sd, output);
					clif->message(fd, output);
				}
			} else
				clif->message(fd, msg_fd(fd, MSGTBL_IMPOSSIBLE_TO_INCREASE_VALUE)); // Unable to decrease the number/value.
		} else {
			if ((pc->paycash(sd, -value, 0)) >= 0) {
			    sprintf(output, msg_fd(fd, MSGTBL_REMOVED_CASHPOINTS), -value, sd->cashPoints);
				clif_disp_onlyself(sd, output);
				clif->message(fd, output);
			} else
				clif->message(fd, msg_fd(fd, MSGTBL_UNABLE_TO_DECREASE_VALUE)); // Unable to decrease the number/value.
		}
	} else { // @points
		if (value > 0) {
			if ((pc->getcash(sd, 0, value)) >= 0) {
				// If this option is set, the message is already sent by pc function
				if (!battle_config.cashshop_show_points) {
					sprintf(output, msg_fd(fd, MSGTBL_GAINED_KAFRAPOINTS), value, sd->kafraPoints);
					clif_disp_onlyself(sd, output);
					clif->message(fd, output);
				}
			} else
				clif->message(fd, msg_fd(fd, MSGTBL_IMPOSSIBLE_TO_INCREASE_VALUE)); // Unable to decrease the number/value.
		} else {
			if ((pc->paycash(sd, -value, -value)) >= 0) {
			    sprintf(output, msg_fd(fd, MSGTBL_REMOVED_KAFRAPOINTS), -value, sd->kafraPoints);
				clif_disp_onlyself(sd, output);
				clif->message(fd, output);
			} else
				clif->message(fd, msg_fd(fd, MSGTBL_UNABLE_TO_DECREASE_VALUE)); // Unable to decrease the number/value.
		}
	}

	return true;
}

// @clone/@slaveclone/@evilclone <playername> [Valaris]
ACMD(clone)
{
	int x=0,y=0,flag=0,master=0,i=0;
	struct map_session_data *pl_sd=NULL;

	if (!*message) {
		clif->message(sd->fd,msg_fd(fd, MSGTBL_CLONE_ENTER_NAME_OR_ID)); // You must enter a player name or ID.
		return false;
	}

	if ((pl_sd=map->nick2sd(message, true)) == NULL && (pl_sd=map->charid2sd(atoi(message))) == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_CHARACTER_NOT_FOUND)); // Character not found.
		return false;
	}

	if (pc_get_group_level(pl_sd) > pc_get_group_level(sd)) {
		clif->message(fd, msg_fd(fd, MSGTBL_CLONE_HIGHER_GM)); // Cannot clone a player of higher GM level than yourself.
		return false;
	}

	if (strcmpi(info->command, "clone") == 0)
		flag = 1;
	else if (strcmpi(info->command, "slaveclone") == 0) {
		flag = 2;
		if (pc_isdead(sd)){
			//"Unable to spawn slave clone."
		    clif->message(fd, msg_fd(fd, MSGTBL_EVILCLONE_FAIL + flag * 2));
		    return false;
		}
		master = sd->bl.id;
		if (battle_config.atc_slave_clone_limit
			&& mob->countslave(&sd->bl) >= battle_config.atc_slave_clone_limit) {
			clif->message(fd, msg_fd(fd, MSGTBL_CLONE_LIMIT_REACHED)); // You've reached your slave clones limit.
			return false;
		}
	}

	do {
		x = sd->bl.x + (rnd() % 10 - 5);
		y = sd->bl.y + (rnd() % 10 - 5);
	} while (map->getcell(sd->bl.m, &sd->bl, x, y, CELL_CHKNOPASS) && i++ < 10);

	if (i >= 10) {
		x = sd->bl.x;
		y = sd->bl.y;
	}

	if ((x = mob->clone_spawn(pl_sd, sd->bl.m, x, y, "", master, MD_NONE, flag?1:0, 0)) > 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_EVILCLONE_SPAWNED + flag * 2)); // Evil Clone spawned. Clone spawned. Slave clone spawned.
		return true;
	}
	clif->message(fd, msg_fd(fd, MSGTBL_EVILCLONE_FAIL + flag * 2)); // Unable to spawn evil clone. Unable to spawn clone. Unable to spawn slave clone.
	return false;
}

/*=====================================
 * Autorejecting Invites/Deals [LuzZza]
 * Usage: @noask
 *-------------------------------------*/
ACMD(noask)
{
	if (sd->state.noask) {
		clif->message(fd, msg_fd(fd, MSGTBL_NOASK_OFF)); // Autorejecting is deactivated.
		sd->state.noask = 0;
	} else {
		clif->message(fd, msg_fd(fd, MSGTBL_NOASK_ON)); // Autorejecting is activated.
		sd->state.noask = 1;
	}

	return true;
}

/*=====================================
 * Send a @request message to all GMs of lowest_gm_level.
 * Usage: @request <petition>
 *-------------------------------------*/
ACMD(request)
{
	if (!*message) {
		clif->message(sd->fd,msg_fd(fd, MSGTBL_REQUEST_USAGE)); // Usage: @request <petition/message to online GMs>.
		return false;
	}

	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_REQUEST_MESSAGE), message); // (@request): %s
	pc->wis_message_to_gm(sd->status.name, PC_PERM_RECEIVE_REQUESTS, atcmd_output);
	clif_disp_onlyself(sd, atcmd_output);
	clif->message(sd->fd,msg_fd(fd, MSGTBL_REQUEST_SENT)); // @request sent.
	return true;
}

/*==========================================
 * Feel (SG save map) Reset [HiddenDragon]
 *------------------------------------------*/
ACMD(feelreset)
{
	pc->resetfeel(sd);
	clif->message(fd, msg_fd(fd, MSGTBL_FEELRESET_RESET)); // "Reset 'Feeling' maps."

	return true;
}

// Reset hatred targets [Wolfie]
ACMD(hatereset)
{
	pc->resethate(sd);
	clif->message(fd, msg_fd(fd, MSGTBL_RESET_HATRED_TARGETS)); // "Reset 'Hatred' targets."

	return true;
}

/*==========================================
 * AUCTION SYSTEM
 *------------------------------------------*/
ACMD(auction)
{
	if (!battle_config.feature_auction) {
		clif->messagecolor_self(sd->fd, COLOR_RED, msg_fd(fd, MSGTBL_AUCTION_DISABLED)); // "Auction is disabled."
		return false;
	}

	clif->auction_openwindow(sd);

	return true;
}

/*==========================================
 * Kill Steal Protection
 *------------------------------------------*/
ACMD(ksprotection)
{
	if( sd->state.noks ) {
		sd->state.noks = KSPROTECT_NONE;
		clif->message(fd, msg_fd(fd, MSGTBL_NOKS_INACTIVE)); // [ K.S Protection Inactive ]
	} else if (!*message || strcmpi(message, "party") == 0) {
		// Default is Party
		sd->state.noks = KSPROTECT_PARTY;
		clif->message(fd, msg_fd(fd, MSGTBL_NOKS_ACTIVE_PARTY)); // [ K.S Protection Active - Option: Party ]
	} else if( strcmpi(message, "self") == 0 ) {
		sd->state.noks = KSPROTECT_SELF;
		clif->message(fd, msg_fd(fd, MSGTBL_NOKS_ACTIVE_SELF)); // [ K.S Protection Active - Option: Self ]
	} else if( strcmpi(message, "guild") == 0 ) {
		sd->state.noks = KSPROTECT_GUILD;
		clif->message(fd, msg_fd(fd, MSGTBL_NOKS_ACTIVE_GUILD)); // [ K.S Protection Active - Option: Guild ]
	} else {
		clif->message(fd, msg_fd(fd, MSGTBL_NOKS_USAGE)); // Usage: @noks <self|party|guild>
	}
	return true;
}
/*==========================================
 * Map Kill Steal Protection Setting
 *------------------------------------------*/
ACMD(allowks)
{
	if( map->list[sd->bl.m].flag.allowks ) {
		map->list[sd->bl.m].flag.allowks = 0;
		clif->message(fd, msg_fd(fd, MSGTBL_ALLOWKS_ACTIVE)); // [ Map K.S Protection Active ]
	} else {
		map->list[sd->bl.m].flag.allowks = 1;
		clif->message(fd, msg_fd(fd, MSGTBL_ALLOWKS_INACTIVE)); // [ Map K.S Protection Inactive ]
	}
	return true;
}

ACMD(resetstat)
{
	pc->resetstate(sd);
	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_PLAYER_STATS_POINTS_RESET), sd->status.name);
	clif->message(fd, atcmd_output);
	return true;
}

ACMD(resetskill)
{
	pc->resetskill(sd, PCRESETSKILL_RESYNC);
	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_PLAYER_SKILL_POINTS_RESET), sd->status.name);
	clif->message(fd, atcmd_output);
	return true;
}

/*==========================================
 * #storagelist: Displays the items list of a player's storage.
 * #cartlist: Displays contents of target's cart.
 * #itemlist: Displays contents of target's inventory.
 *------------------------------------------*/
ACMD(itemlist)
{
	int i, j, count, counter;
	const char* location;
	const struct item* items;
	int size;
	StringBuf buf;

	if( strcmpi(info->command, "storagelist") == 0 ) {
		location = "storage";
		items = VECTOR_DATA(sd->storage.item);
		size = VECTOR_LENGTH(sd->storage.item);
	} else if( strcmpi(info->command, "cartlist") == 0 ) {
		location = "cart";
		items = sd->status.cart;
		size = MAX_CART;
	} else if( strcmpi(info->command, "itemlist") == 0 ) {
		location = "inventory";
		items = sd->status.inventory;
		size = sd->status.inventorySize;
	} else
		return false;

	StrBuf->Init(&buf);

	count = 0; // total slots occupied
	counter = 0; // total items found
	for( i = 0; i < size; ++i )
	{
		const struct item* it = &items[i];
		struct item_data* itd;

		if (it->nameid == 0 || (itd = itemdb->exists(it->nameid)) == NULL)
			continue;

		counter += it->amount;
		count++;

		if( count == 1 )
		{
			StrBuf->Printf(&buf, msg_fd(fd, MSGTBL_ITEMLIST_TITLE), location, sd->status.name); // ------ %s items list of '%s' ------
			clif->message(fd, StrBuf->Value(&buf));
			StrBuf->Clear(&buf);
		}

		if( it->refine )
			StrBuf->Printf(&buf, "%d: %d %s %+d (%s, id: %d)", i, it->amount, itd->jname, it->refine, itd->name, it->nameid);
		else
			StrBuf->Printf(&buf, "%d: %d %s (%s, id: %d)", i, it->amount, itd->jname, itd->name, it->nameid);

		if( it->equip ) {
			char equipstr[CHAT_SIZE_MAX];
			strcpy(equipstr, msg_fd(fd, MSGTBL_ITEMLIST_EQUIPPED)); //  | equipped:
			if( it->equip & EQP_GARMENT )
				strcat(equipstr, msg_fd(fd, MSGTBL_ITEMLIST_GARMENT)); // garment,
			if( it->equip & EQP_ACC_L )
				strcat(equipstr, msg_fd(fd, MSGTBL_ITEMLIST_LEFT_ACCESSORY)); // left accessory,
			if( it->equip & EQP_ARMOR )
				strcat(equipstr, msg_fd(fd, MSGTBL_ITEMLIST_BODY_ARMOR)); // body/armor,
			if( (it->equip & EQP_ARMS) == EQP_HAND_R )
				strcat(equipstr, msg_fd(fd, MSGTBL_ITEMLIST_RIGHT_HAND)); // right hand,
			if( (it->equip & EQP_ARMS) == EQP_HAND_L )
				strcat(equipstr, msg_fd(fd, MSGTBL_ITEMLIST_LEFT_HAND)); // left hand,
			if( (it->equip & EQP_ARMS) == EQP_ARMS )
				strcat(equipstr, msg_fd(fd, MSGTBL_ITEMLIST_BOTH_HANDS)); // both hands,
			if( it->equip & EQP_SHOES )
				strcat(equipstr, msg_fd(fd, MSGTBL_ITEMLIST_FEET)); // feet,
			if( it->equip & EQP_ACC_R )
				strcat(equipstr, msg_fd(fd, MSGTBL_ITEMLIST_RIGHT_ACCESSORY)); // right accessory,
			if( (it->equip & EQP_HELM) == EQP_HEAD_LOW )
				strcat(equipstr, msg_fd(fd, MSGTBL_ITEMLIST_LOWER_HEAD)); // lower head,
			if( (it->equip & EQP_HELM) == EQP_HEAD_TOP )
				strcat(equipstr, msg_fd(fd, MSGTBL_ITEMLIST_TOP_HEAD)); // top head,
			if( (it->equip & EQP_HELM) == (EQP_HEAD_LOW|EQP_HEAD_TOP) )
				strcat(equipstr, msg_fd(fd, MSGTBL_ITEMLIST_LOWER_TOP_HEAD)); // lower/top head,
			if( (it->equip & EQP_HELM) == EQP_HEAD_MID )
				strcat(equipstr, msg_fd(fd, MSGTBL_ITEMLIST_MID_HEAD)); // mid head,
			if( (it->equip & EQP_HELM) == (EQP_HEAD_LOW|EQP_HEAD_MID) )
				strcat(equipstr, msg_fd(fd, MSGTBL_ITEMLIST_LOWER_MID_HEAD)); // lower/mid head,
			if( (it->equip & EQP_HELM) == EQP_HELM )
				strcat(equipstr, msg_fd(fd, MSGTBL_ITEMLIST_LOWER_MID_TOP_HEAD)); // lower/mid/top head,
			// remove final ', '
			equipstr[strlen(equipstr) - 2] = '\0';
			StrBuf->AppendStr(&buf, equipstr);
		}

		clif->message(fd, StrBuf->Value(&buf));
		StrBuf->Clear(&buf);

		if( it->card[0] == CARD0_PET ) {
			// pet egg
			if ((it->card[3] & 1) != 0)
				StrBuf->Printf(&buf, msg_fd(fd, MSGTBL_ITEMLIST_PET_NAMED), (unsigned int)MakeDWord(it->card[1], it->card[2])); //  -> (pet egg, pet id: %u, named)
			else
				StrBuf->Printf(&buf, msg_fd(fd, MSGTBL_ITEMLIST_PET_UNNAMED), (unsigned int)MakeDWord(it->card[1], it->card[2])); //  -> (pet egg, pet id: %u, unnamed)
		} else if(it->card[0] == CARD0_FORGE) {
			// forged item
			StrBuf->Printf(&buf, msg_fd(fd, MSGTBL_ITEMLIST_CRAFTED_ITEM), (unsigned int)MakeDWord(it->card[2], it->card[3]), it->card[1]>>8, it->card[1]&0x0f); //  -> (crafted item, creator id: %u, star crumbs %d, element %d)
		} else if(it->card[0] == CARD0_CREATE) {
			// created item
			StrBuf->Printf(&buf, msg_fd(fd, MSGTBL_ITEMLIST_PRODUCED_ITEM), (unsigned int)MakeDWord(it->card[2], it->card[3])); //  -> (produced item, creator id: %u)
		} else {
			// normal item
			int counter2 = 0;

			for( j = 0; j < itd->slot; ++j ) {
				struct item_data* card;

				if( it->card[j] == 0 || (card = itemdb->exists(it->card[j])) == NULL )
					continue;

				counter2++;

				if( counter2 == 1 )
					StrBuf->AppendStr(&buf, msg_fd(fd, MSGTBL_ITEMLIST_CARDS)); //  -> (card(s):

				if( counter2 != 1 )
					StrBuf->AppendStr(&buf, ", ");

				StrBuf->Printf(&buf, "#%d %s (id: %d)", counter2, card->jname, card->nameid);
			}

			if( counter2 > 0 )
				StrBuf->AppendStr(&buf, ")");
		}

		if( StrBuf->Length(&buf) > 0 )
			clif->message(fd, StrBuf->Value(&buf));

		StrBuf->Clear(&buf);
	}

	if( count == 0 )
		StrBuf->Printf(&buf, msg_fd(fd, MSGTBL_ITEMLIST_NO_ITEM_FOUND), location); // No item found in this player's %s.
	else
		StrBuf->Printf(&buf, msg_fd(fd, MSGTBL_ITEMLIST_ITEMS_FOUND), counter, count, location); // %d item(s) found in %d %s slots.

	clif->message(fd, StrBuf->Value(&buf));

	StrBuf->Destroy(&buf);
	return true;
}

ACMD(stats)
{
	char job_jobname[100];
	char output[CHAT_SIZE_MAX];
	int i;
	struct {
		const char* format;
		int value;
	} output_table[] = {
		{ "Base Level - %d", 0 },
		{ NULL, 0 },
		{ "Hp - %d", 0 },
		{ "MaxHp - %d", 0 },
		{ "Sp - %d", 0 },
		{ "MaxSp - %d", 0 },
		{ "Str - %3d", 0 },
		{ "Agi - %3d", 0 },
		{ "Vit - %3d", 0 },
		{ "Int - %3d", 0 },
		{ "Dex - %3d", 0 },
		{ "Luk - %3d", 0 },
		{ "Zeny - %d", 0 },
		{ "Free SK Points - %d", 0 },
		{ "JobChangeLvl (2nd) - %d", 0 },
		{ "JobChangeLvl (3rd) - %d", 0 },
		{ NULL, 0 }
	};

	memset(job_jobname, '\0', sizeof(job_jobname));
	memset(output, '\0', sizeof(output));

	//direct array initialization with variables is not standard C compliant.
	output_table[0].value = sd->status.base_level;
	output_table[1].format = job_jobname;
	output_table[1].value = sd->status.job_level;
	output_table[2].value = sd->status.hp;
	output_table[3].value = sd->status.max_hp;
	output_table[4].value = sd->status.sp;
	output_table[5].value = sd->status.max_sp;
	output_table[6].value = sd->status.str;
	output_table[7].value = sd->status.agi;
	output_table[8].value = sd->status.vit;
	output_table[9].value = sd->status.int_;
	output_table[10].value = sd->status.dex;
	output_table[11].value = sd->status.luk;
	output_table[12].value = sd->status.zeny;
	output_table[13].value = sd->status.skill_point;
	output_table[14].value = sd->change_level_2nd;
	output_table[15].value = sd->change_level_3rd;

	sprintf(job_jobname, "Job - %s %s", pc->job_name(sd->status.class), "(level %d)");
	sprintf(output, msg_fd(fd, MSGTBL_TARGET_STATS), sd->status.name); // '%s' stats:

	clif->message(fd, output);

	for (i = 0; output_table[i].format != NULL; i++) {
		sprintf(output, output_table[i].format, output_table[i].value);
		clif->message(fd, output);
	}
	return true;
}

ACMD(delitem)
{
	char item_name[100];
	int nameid, amount = 0, total, idx;
	struct item_data* id;

	if (!*message || (sscanf(message, "\"%99[^\"]\" %12d", item_name, &amount) < 2 && sscanf(message, "%99s %12d", item_name, &amount) < 2) || amount < 1)
	{
		clif->message(fd, msg_fd(fd, MSGTBL_DELITEM_USAGE)); // Please enter an item name/ID, a quantity, and a player name (usage: #delitem <player> <item_name_or_ID> <quantity>).
		return false;
	}

	if ((id = itemdb->search_name(item_name)) != NULL || (id = itemdb->exists(atoi(item_name))) != NULL)
	{
		nameid = id->nameid;
	}
	else
	{
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_ITEMID_NAME)); // Invalid item ID or name.
		return false;
	}

	total = amount;

	// delete items
	while (amount && (idx = pc->search_inventory(sd, nameid)) != INDEX_NOT_FOUND) {
		int delamount = ( amount < sd->status.inventory[idx].amount ) ? amount : sd->status.inventory[idx].amount;

		if( sd->inventory_data[idx]->type == IT_PETEGG && sd->status.inventory[idx].card[0] == CARD0_PET )
		{// delete pet
			intif->delete_petdata(MakeDWord(sd->status.inventory[idx].card[1], sd->status.inventory[idx].card[2]));
		}
		pc->delitem(sd, idx, delamount, 0, DELITEM_NORMAL, LOG_TYPE_COMMAND);

		amount-= delamount;
	}

	// notify target
	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_N_ITEMS_REMOVED_BY_GM), total-amount); // %d item(s) removed by a GM.
	clif->message(sd->fd, atcmd_output);

	// notify source
	if( amount == total )
	{
		clif->message(fd, msg_fd(fd, MSGTBL_DOES_NOT_HAVE_ITEM)); // Character does not have the item.
	}
	else if( amount )
	{
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_N_ITEMS_REMOVED_AMOUNT), total-amount, total-amount, total); // %d item(s) removed. Player had only %d on %d items.
		clif->message(fd, atcmd_output);
	}
	else
	{
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_N_ITEMS_REMOVED_FROM_PLAYER), total); // %d item(s) removed from the player.
		clif->message(fd, atcmd_output);
	}
	return true;
}

/*==========================================
 * Custom Fonts
 *------------------------------------------*/
ACMD(font)
{
	int font_id;

	font_id = atoi(message);
	if( font_id == 0 )
	{
		if( sd->status.font )
		{
			sd->status.font = 0;
			clif->message(fd, msg_fd(fd, MSGTBL_FONT_RETURN_NORMAL)); // Returning to normal font.
			clif->font(sd);
		}
		else
		{
			clif->message(fd, msg_fd(fd, MSGTBL_FONT_CHANGE_USAGE)); // Use @font <1-9> to change your message font.
			clif->message(fd, msg_fd(fd, MSGTBL_FONT_RETURN_NO_PARAM)); // Use 0 or no parameter to return to normal font.
		}
	}
	else if( font_id < 0 || font_id > 9 )
		clif->message(fd, msg_fd(fd, MSGTBL_FONT_INVALID)); // Invalid font. Use a value from 0 to 9.
	else if( font_id != sd->status.font )
	{
		sd->status.font = font_id;
		clif->font(sd);
		clif->message(fd, msg_fd(fd, MSGTBL_FONT_CHANGED)); // Font changed.
	}
	else
		clif->message(fd, msg_fd(fd, MSGTBL_FONT_ALREADY_USED)); // Already using this font.

	return true;
}

/*==========================================
 * type: 1 = commands (@), 2 = charcommands (#)
 *------------------------------------------*/
static void atcommand_commands_sub(struct map_session_data *sd, const int fd, AtCommandType type)
{
	char line_buff[CHATBOX_SIZE];
	char* cur = line_buff;
	AtCommandInfo* cmd;
	struct DBIterator *iter = db_iterator(atcommand->db);
	int count = 0;

	memset(line_buff,' ',CHATBOX_SIZE);
	line_buff[CHATBOX_SIZE-1] = 0;

	clif->message(fd, msg_fd(fd, MSGTBL_AVAILABLE_COMMANDS)); // "Available commands:"

	for (cmd = dbi_first(iter); dbi_exists(iter); cmd = dbi_next(iter)) {
		size_t slen;

		switch( type ) {
			case COMMAND_CHARCOMMAND:
				if( cmd->char_groups[pcg->get_idx(sd->group)] == 0 )
					continue;
				break;
			case COMMAND_ATCOMMAND:
				if( cmd->at_groups[pcg->get_idx(sd->group)] == 0 )
					continue;
				break;
			default:
				continue;
		}

		slen = strlen(cmd->command);

		// flush the text buffer if this command won't fit into it
		if ( slen + cur - line_buff >= CHATBOX_SIZE )
		{
			clif->message(fd,line_buff);
			cur = line_buff;
			memset(line_buff,' ',CHATBOX_SIZE);
			line_buff[CHATBOX_SIZE-1] = 0;
		}

		memcpy(cur,cmd->command,slen);
		cur += slen+(10-slen%10);

		count++;
	}
	dbi_destroy(iter);
	clif->message(fd,line_buff);

	if (atcommand->binding_count > 0) {
		int i, count_bind = 0;
		int gm_lvl = pc_get_group_level(sd);

		for (i = 0; i < atcommand->binding_count; i++) {
			if (gm_lvl >= ((type == COMMAND_ATCOMMAND) ? atcommand->binding[i]->group_lv : atcommand->binding[i]->group_lv_char)
				|| (type == COMMAND_ATCOMMAND && atcommand->binding[i]->at_groups[pcg->get_idx(sd->group)] > 0)
				|| (type == COMMAND_CHARCOMMAND && atcommand->binding[i]->char_groups[pcg->get_idx(sd->group)] > 0)) {
				size_t slen = strlen(atcommand->binding[i]->command);
				if (count_bind == 0) {
					cur = line_buff;
					memset(line_buff, ' ', CHATBOX_SIZE);
					line_buff[CHATBOX_SIZE - 1] = 0;
					clif->message(fd, "------------------");
					clif->message(fd, msg_sd(sd, MSGTBL_CUSTOM_COMMANDS)); // "Custom commands:"
				}
				if (slen + cur - line_buff >= CHATBOX_SIZE) {
					clif->message(fd, line_buff);
					cur = line_buff;
					memset(line_buff, ' ', CHATBOX_SIZE);
					line_buff[CHATBOX_SIZE - 1] = 0;
				}
				memcpy(cur, atcommand->binding[i]->command, slen);
				cur += slen + (10 - slen % 10);
				count_bind++;
			}
		}
		if (count_bind > 0)
			clif->message(fd, line_buff); // Last Line
		count += count_bind;
	}

	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_N_COMMANDS_FOUND), count); // "%d commands found."
	clif->message(fd, atcmd_output);

	return;
}

/*==========================================
 * @commands Lists available @ commands to you
 *------------------------------------------*/
ACMD(commands)
{
	atcommand->commands_sub(sd, fd, COMMAND_ATCOMMAND);
	return true;
}

/*==========================================
 * @charcommands Lists available # commands to you
 *------------------------------------------*/
ACMD(charcommands)
{
	atcommand->commands_sub(sd, fd, COMMAND_CHARCOMMAND);
	return true;
}

/* For new mounts */
ACMD(cashmount)
{
	if (pc_hasmount(sd)) {
		clif->message(fd, msg_fd(fd, MSGTBL_ALREADY_MOUNTED)); // You are already mounting something else
		return false;
	}

	clif->message(sd->fd, msg_fd(fd, MSGTBL_NEW_MOUNT_NOTICE)); // NOTICE: If you crash with mount, your Lua files are outdated.

	if (!sd->sc.data[SC_ALL_RIDING]) {
		clif->message(sd->fd, msg_fd(fd, MSGTBL_NEW_MOUNT_MOUNTED)); // You are mounted now.
		sc_start(NULL, &sd->bl, SC_ALL_RIDING, 100, battle_config.boarding_halter_speed, INFINITE_DURATION, 0);
	} else {
		clif->message(sd->fd, msg_fd(fd, MSGTBL_NEW_MOUNT_RELEASED)); // You have released your mount.
		status_change_end(&sd->bl, SC_ALL_RIDING, INVALID_TIMER);
	}

	return true;
}

ACMD(accinfo)
{
	char query[NAME_LENGTH];

	if (!*message || strlen(message) > NAME_LENGTH ) {
		clif->message(fd, msg_fd(fd, MSGTBL_ACCINFO_USAGE)); // Usage: @accinfo/@accountinfo <account_id/char name>
		clif->message(fd, msg_fd(fd, MSGTBL_ACCINFO_SEARCH_TIP)); // You may search partial name by making use of '%' in the search, ex. "@accinfo %Mario%" lists all characters whose name contains "Mario".
		return false;
	}

	//remove const type
	safestrncpy(query, message, NAME_LENGTH);
	intif->request_accinfo( sd->fd, sd->bl.id, pc_get_group_level(sd), query );

	return true;
}

/* [Ind] */
ACMD(set)
{
	char reg[SCRIPT_VARNAME_LENGTH + 1];
	char val[SCRIPT_STRING_VAR_LENGTH + 1];
	struct script_data* data;
	int toset = 0;
	bool is_str = false;
	size_t len;

	char format[20];
	snprintf(format, sizeof(format), "%%%ds %%%d[^\\n]", SCRIPT_VARNAME_LENGTH, SCRIPT_STRING_VAR_LENGTH);

	if (*message == '\0' || (toset = sscanf(message, format, reg, val)) < 1) {
		clif->message(fd, msg_fd(fd, MSGTBL_SET_USAGE)); // Usage: @set <variable name> <value>
		clif->message(fd, msg_fd(fd, MSGTBL_SET_EXAMPLE_1)); // Usage: ex. "@set PoringCharVar 50"
		clif->message(fd, msg_fd(fd, MSGTBL_SET_EXAMPLE_2)); // Usage: ex. "@set PoringCharVarSTR$ Super Duper String"
		clif->message(fd, msg_fd(fd, MSGTBL_SET_EXAMPLE_3)); // Usage: ex. "@set PoringCharVarSTR$" outputs its value, Super Duper String.
		return false;
	}

	/* disabled variable types (they require a proper script state to function, so allowing them would crash the server) */
	if( reg[0] == '.' ) {
		clif->message(fd, msg_fd(fd, MSGTBL_SET_NPC_VARIABLE)); // NPC variables may not be used with @set.
		return false;
	} else if( reg[0] == '\'' ) {
		clif->message(fd, msg_fd(fd, MSGTBL_SET_INSTANCE_VARIABLE)); // Instance variables may not be used with @set.
		return false;
	}

	is_str = ( reg[strlen(reg) - 1] == '$' ) ? true : false;

	if( ( len = strlen(val) ) > 1 ) {
		if( val[0] == '"' && val[len-1] == '"') {
			val[len-1] = '\0'; //Strip quotes.
			memmove(val, val+1, len-1);
		}
	}

	if( toset >= 2 ) {/* we only set the var if there is an val, otherwise we only output the value */
		if( is_str )
			script->set_var(sd, reg, (void*) val);
		else
			script->set_var(sd, reg, (void*)h64BPTRSIZE((atoi(val))));
	}

	CREATE(data, struct script_data,1);

	if (is_str) {
		// string variable
		const char *str = NULL;
		switch (reg[0]) {
			case '@':
				str = pc->readregstr(sd, script->add_variable(reg));
				break;
			case '$':
				str = mapreg->readregstr(script->add_variable(reg));
				break;
			case '#':
				if (reg[1] == '#')
					str = pc_readaccountreg2str(sd, script->add_variable(reg));// global
				else
					str = pc_readaccountregstr(sd, script->add_variable(reg));// local
				break;
			default:
				str = pc_readglobalreg_str(sd, script->add_variable(reg));
				break;
		}
		if (str == NULL || str[0] == '\0') {
			// empty string
			data->type = C_CONSTSTR;
			data->u.str = "";
		} else {
			// duplicate string
			data->type = C_STR;
			data->u.mutstr = aStrdup(str);
		}
	} else {// integer variable
		data->type = C_INT;
		switch( reg[0] ) {
			case '@':
				data->u.num = pc->readreg(sd, script->add_variable(reg));
				break;
			case '$':
				data->u.num = mapreg->readreg(script->add_variable(reg));
				break;
			case '#':
				if( reg[1] == '#' )
					data->u.num = pc_readaccountreg2(sd, script->add_variable(reg));// global
				else
					data->u.num = pc_readaccountreg(sd, script->add_variable(reg));// local
				break;
			default:
				data->u.num = pc_readglobalreg(sd, script->add_variable(reg));
				break;
		}
	}

	PRAGMA_GCC46(GCC diagnostic push)
	PRAGMA_GCC46(GCC diagnostic ignored "-Wswitch-enum")
	switch (data->type) {
		case C_INT:
			snprintf(atcmd_output, sizeof(atcmd_output),msg_fd(fd, MSGTBL_SET_VALUE_INT),reg,data->u.num); // %s value is now :%d
			break;
		case C_STR:
			snprintf(atcmd_output, sizeof(atcmd_output),msg_fd(fd, MSGTBL_SET_VALUE_STRING),reg,data->u.mutstr); // %s value is now :%s
			break;
		case C_CONSTSTR:
			snprintf(atcmd_output, sizeof(atcmd_output),msg_fd(fd, MSGTBL_SET_EMPTY),reg); // %s is empty
			break;
		default:
			snprintf(atcmd_output, sizeof(atcmd_output),msg_fd(fd, MSGTBL_SET_UNSUPPORTED_TYPE),reg,data->type); // %s data type is not supported :%u
			break;
	}
	PRAGMA_GCC46(GCC diagnostic pop)
	clif->message(fd, atcmd_output);

	aFree(data);
	return true;
}

/**
 * Sends @quest command help text to fd
 * @param fd
 */
static void atcommand_quest_help(int fd)
{
	clif->message(fd, msg_fd(fd, MSGTBL_QUEST_USAGE));
	clif->message(fd, msg_fd(fd, MSGTBL_QUEST_ADD_ACTION));
	clif->message(fd, msg_fd(fd, MSGTBL_QUEST_DELETE_ACTION));
	clif->message(fd, msg_fd(fd, MSGTBL_QUEST_COMPLETE_ACTION));
}

ACMD(quest)
{
	char subcmd[20];
	int quest_id;

	// @quest add|set <quest id>
	// @quest del|delete <quest id>
	// @quest complete <quest id>
	if (!*message || sscanf(message, "%19s %d", subcmd, &quest_id) < 2) {
		atcommand->quest_help(fd);
		return true;
	}

	struct quest_db *qi = quest->db(quest_id);
	if (qi == &quest->dummy) {
		clif->message(fd, msg_fd(fd, MSGTBL_QUEST_NOT_EXIST));
		return true;
	}

	if (strcmpi(subcmd, "add") == 0 || strcmpi(subcmd, "set") == 0) {
		if (quest->check(sd, quest_id, HAVEQUEST) >= 0) {
			clif->message(fd, msg_fd(fd, MSGTBL_QUEST_ALREADY_HAVE));
			return true;
		}

		if (quest->add(sd, quest_id, 0) != 0) {
			clif->message(fd, msg_fd(fd, MSGTBL_QUEST_ADD_FAILED));
			return true;
		}

		clif->message(fd, msg_fd(fd, MSGTBL_QUEST_ADDED));
		return true;
	}

	if (strcmpi(subcmd, "del") == 0 || strcmpi(subcmd, "delete") == 0) {
		if (quest->check(sd, quest_id, HAVEQUEST) == -1) {
			clif->message(fd, msg_fd(fd, MSGTBL_QUEST_NOT_IN_LOG));
			return true;
		}

		if (quest->delete(sd, quest_id) != 0) {
			clif->message(fd, msg_fd(fd, MSGTBL_QUEST_ERASE_FAILED));
			return true;
		}

		clif->message(fd, msg_fd(fd, MSGTBL_QUEST_REMOVED));
		return true;
	}

	if (strcmpi(subcmd, "complete") == 0) {
		if (quest->check(sd, quest_id, HAVEQUEST) == -1) {
			clif->message(fd, msg_fd(fd, MSGTBL_QUEST_NOT_IN_LOG));
			return true;
		}

		if (quest->update_status(sd, quest_id, Q_COMPLETE) != 0) {
			clif->message(fd, msg_fd(fd, MSGTBL_QUEST_COMPLETE_FAILED));
			return true;
		}

		clif->message(fd, msg_fd(fd, MSGTBL_QUEST_COMPLETED));
		return true;
	}

	atcommand->quest_help(fd);
	return true;
}

ACMD(reloadquestdb)
{
	quest->reload();
	clif->message(fd, msg_fd(fd, MSGTBL_RELOADQUESTDB_SUCCESS)); // Quest database has been reloaded.
	return true;
}

ACMD(addperm)
{
	int perm_size = pcg->permission_count;
	bool add = (strcmpi(info->command, "addperm") == 0) ? true : false;
	int i;

	if (!*message) {
		snprintf(atcmd_output, sizeof(atcmd_output),  msg_fd(fd, MSGTBL_ADDPERM_USAGE),command); // Usage: %s <permission_name>
		clif->message(fd, atcmd_output);
		clif->message(fd, msg_fd(fd, MSGTBL_ADDPERM_PERMISSION_LIST)); // -- Permission List
		for( i = 0; i < perm_size; i++ ) {
			snprintf(atcmd_output, sizeof(atcmd_output),"- %s",pcg->permissions[i].name);
			clif->message(fd, atcmd_output);
		}
		return false;
	}

	ARR_FIND(0, perm_size, i, strcmpi(pcg->permissions[i].name, message) == 0);
	if( i == perm_size ) {
		snprintf(atcmd_output, sizeof(atcmd_output),msg_fd(fd, MSGTBL_ADDPERM_UNKNOWN_PERMISSION),message); // '%s' is not a known permission.
		clif->message(fd, atcmd_output);
		clif->message(fd, msg_fd(fd, MSGTBL_ADDPERM_PERMISSION_LIST)); // -- Permission List
		for( i = 0; i < perm_size; i++ ) {
			snprintf(atcmd_output, sizeof(atcmd_output),"- %s",pcg->permissions[i].name);
			clif->message(fd, atcmd_output);
		}
		return false;
	}

	if( add && (sd->extra_temp_permissions&pcg->permissions[i].permission) ) {
		snprintf(atcmd_output, sizeof(atcmd_output),  msg_fd(fd, MSGTBL_ADDPERM_USER_ALREADY_PERMISSION),sd->status.name,pcg->permissions[i].name); // User '%s' already possesses the '%s' permission.
		clif->message(fd, atcmd_output);
		return false;
	} else if ( !add && !(sd->extra_temp_permissions&pcg->permissions[i].permission) ) {
		snprintf(atcmd_output, sizeof(atcmd_output),  msg_fd(fd, MSGTBL_ADDPERM_USER_NO_PERMISSION),sd->status.name,pcg->permissions[i].name); // User '%s' doesn't possess the '%s' permission.
		clif->message(fd, atcmd_output);
		snprintf(atcmd_output, sizeof(atcmd_output),msg_fd(fd, MSGTBL_ADDPERM_USER_PERMISSIONS),sd->status.name); // -- User '%s' Permissions
		clif->message(fd, atcmd_output);
		for( i = 0; i < perm_size; i++ ) {
			if( sd->extra_temp_permissions&pcg->permissions[i].permission ) {
				snprintf(atcmd_output, sizeof(atcmd_output),"- %s",pcg->permissions[i].name);
				clif->message(fd, atcmd_output);
			}
		}
		return false;
	}

	if( add )
		sd->extra_temp_permissions |= pcg->permissions[i].permission;
	else
		sd->extra_temp_permissions &=~ pcg->permissions[i].permission;

	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_ADDPERM_SUCCESS),sd->status.name); // User '%s' permissions updated successfully. The changes are temporary.
	clif->message(fd, atcmd_output);

	return true;
}

/**
 * Unloads a script file.
 * Note: Be aware that some changes made by NPC are not reverted on unload. See doc/atcommands.txt for details.
 *
 * @code{.herc}
 *	@unloadnpcfile <path> {<flag>}
 * @endcode
 *
 **/
ACMD(unloadnpcfile)
{
	char format[20];

	snprintf(format, sizeof(format), "%%%ds %%1d", MAX_DIR_PATH);

	char file_path[MAX_DIR_PATH + 1] = {'\0'};
	int flag = 1;

	if (*message == '\0' || (sscanf(message, format, file_path, &flag) < 1)) {
		clif->message(fd, msg_fd(fd, MSGTBL_UNLOADNPCFILE_USAGE)); /// Usage: @unloadnpcfile <path> {<flag>}
		return false;
	}

	if (!exists(file_path)) {
		clif->message(fd, msg_fd(fd, MSGTBL_UNLOADNPCFILE_NOT_FOUND)); /// File not found.
		return false;
	}

	if (!is_file(file_path)) {
		clif->message(fd, msg_fd(fd, MSGTBL_NOT_A_FILE)); /// Not a file.
		return false;
	}

	FILE *fp = fopen(file_path, "r");

	if (fp == NULL) {
		clif->message(fd, msg_fd(fd, MSGTBL_CANT_OPEN_FILE)); /// Can't open file.
		return false;
	}

	fclose(fp);

	if (!npc->unloadfile(file_path, (flag != 0))) {
		clif->message(fd, msg_fd(fd, MSGTBL_RELOADNPC_NOT_UNLOADED)); /// Script could not be unloaded.
		return false;
	}

	clif->message(fd, msg_fd(fd, MSGTBL_UNLOADNPCFILE_SUCCESS)); /// File unloaded. Be aware that...
	return true;
}

ACMD(cart)
{
#define MC_CART_MDFY(x,idx) do { \
	sd->status.skill[idx].id = (x)?MC_PUSHCART:0; \
	sd->status.skill[idx].lv = (x)?1:0; \
	sd->status.skill[idx].flag = (x)?1:0; \
} while(0)

	int val = atoi(message);
	bool need_skill = pc->checkskill(sd, MC_PUSHCART) ? false : true;
	int index = skill->get_index(MC_PUSHCART);

	if (!*message || val < 0 || val > MAX_CARTS) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CART_UNKNOWN_CART),command,MAX_CARTS); // Unknown Cart (usage: %s <0-%d>).
		clif->message(fd, atcmd_output);
		return false;
	}

	if( val == 0 && !pc_iscarton(sd) ) {
		clif->message(fd, msg_fd(fd, MSGTBL_CART_NO_CART_TO_REMOVE)); // You do not possess a cart to be removed
		return false;
	}

	if( need_skill ) {
		MC_CART_MDFY(1,index);
	}

	if( pc->setcart(sd, val) ) {
		if( need_skill ) {
			MC_CART_MDFY(0,index);
		}
		return false;/* @cart failed */
	}

	if( need_skill ) {
		MC_CART_MDFY(0,index);
	}

	clif->message(fd, msg_fd(fd, MSGTBL_CART_ADDED)); // Cart Added

	return true;
#undef MC_CART_MDFY
}

/* [Ind/Hercules] */
ACMD(join)
{
	struct channel_data *chan = NULL;
	char name[HCS_NAME_LENGTH], pass[HCS_NAME_LENGTH];
	enum channel_operation_status ret = HCS_STATUS_OK;

	if (!*message || sscanf(message, "%19s %19s", name, pass) < 1) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_JOIN_UNKNOWN_CHANNEL),command); // Unknown Channel (usage: %s <#channel_name>)
		clif->message(fd, atcmd_output);
		return false;
	}

	chan = channel->search(name, sd);

	if (!chan) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_JOIN_UNKNOWN_CHANNEL_NAME),name,command); // Unknown Channel '%s' (usage: %s <#channel_name>)
		clif->message(fd, atcmd_output);
		return false;
	}

	ret = channel->join(chan, sd, pass, false);

	if (ret == HCS_STATUS_ALREADY) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_HERC_CHAT_ALREADY_IN_CHANNEL),name); // You're already in the '%s' channel
		clif->message(fd, atcmd_output);
		return false;
	}

	if (ret == HCS_STATUS_NOPERM) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_JOIN_CHANNEL_PW_PROTECTED),name,command); // '%s' Channel is password protected (usage: %s <#channel_name> <password>)
		clif->message(fd, atcmd_output);
		return false;
	}

	if (ret == HCS_STATUS_BANNED) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_CANNOT_JOIN_BANNED),name); // You cannot join the '%s' channel because you've been banned from it
		clif->message(fd, atcmd_output);
		return false;
	}

	return true;
}

/* [Ind/Hercules] */
static void atcommand_channel_help(int fd, const char *command, bool can_create)
{
	nullpo_retv(command);
	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_FAILED),command); // %s failed.
	clif->message(fd, atcmd_output);
	clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_AVAILABLE_OPT));// --- Available options:
	if( can_create ) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_CREATE_USAGE),command);// -- %s create <channel name> <channel password>
		clif->message(fd, atcmd_output);
		clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_CREATE_DESC));// - creates a new channel
	}
	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_LIST_USAGE),command);// -- %s list
	clif->message(fd, atcmd_output);
	clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_LIST_DESC));// - lists public channels
	if( can_create ) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_LIST_COLORS_USAGE),command);// -- %s list colors
		clif->message(fd, atcmd_output);
		clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_LIST_COLORS_DESC));// - lists colors available to select for custom channels
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_SETCOLOR_USAGE),command);// -- %s setcolor <channel name> <color name>
		clif->message(fd, atcmd_output);
		clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_SETCOLOR_DESC));// - changes <channel name> color to <color name>
	}
	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_LEAVE_USAGE),command);// -- %s leave <channel name>
	clif->message(fd, atcmd_output);
	clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_LEAVE_DESC));// - leaves <channel name>
	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_BINDTO_USAGE),command);// -- %s bindto <channel name>
	clif->message(fd, atcmd_output);
	clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_BINDTO_DESC));// - binds global chat to <channel name>, making anything you type in global be sent to the channel
	snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_UNBIND_USAGE),command);// -- %s unbind
	clif->message(fd, atcmd_output);
	clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_UNBIND_DESC));// - unbinds your global chat from its attached channel (if bound)
	if( can_create ) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_BAN_USAGE),command);// -- %s ban <channel name> <character name>
		clif->message(fd, atcmd_output);
		clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_BAN_DESC));// - bans <character name> from <channel name> channel
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_BANLIST_USAGE),command);// -- %s banlist <channel name>
		clif->message(fd, atcmd_output);
		clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_BANLIST_DESC));// - lists all banned characters from <channel name> channel
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_UNBAN_USAGE),command);// -- %s unban <channel name> <character name>
		clif->message(fd, atcmd_output);
		clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_UNBAN_DESC));// - unbans <character name> from <channel name> channel
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_UNBANALL_USAGE),command);// -- %s unbanall <channel name>
		clif->message(fd, atcmd_output);
		clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_UNBANALL));// - unbans everyone from <channel name>
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_SETOPT_USAGE),command);// -- %s setopt <channel name> <option name> <option value>
		clif->message(fd, atcmd_output);
		clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_SETOPT_DESC));// - adds or removes <option name> with <option value> to <channel name> channel
	}
}

/* [Ind/Hercules] */
ACMD(channel)
{
	struct channel_data *chan;
	char subcmd[HCS_NAME_LENGTH], sub1[HCS_NAME_LENGTH], sub2[HCS_NAME_LENGTH], sub3[HCS_NAME_LENGTH];
	sub1[0] = sub2[0] = sub3[0] = '\0';

	if (!*message || sscanf(message, "%19s %19s %19s %19s", subcmd, sub1, sub2, sub3) < 1) {
		atcommand->channel_help(fd,command, (channel->config->allow_user_channel_creation || pc_has_permission(sd, PC_PERM_HCHSYS_ADMIN)));
		return true;
	}

	if (strcmpi(subcmd,"create") == 0 && (channel->config->allow_user_channel_creation || pc_has_permission(sd, PC_PERM_HCHSYS_ADMIN))) {
		// sub1 = channel name; sub2 = password; sub3 = unused
		size_t len = strlen(sub1);
		const char *pass = *sub2 ? sub2 : "";
		if (sub1[0] != '#') {
			clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_NAME_START));// Channel name must start with a '#'
			return false;
		} else if (len < 3 || len > HCS_NAME_LENGTH) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_NAME_LENGTH_INVALID), HCS_NAME_LENGTH);// Channel length must be between 3 and %d
			clif->message(fd, atcmd_output);
			return false;
		} else if (sub3[0] != '\0') {
			clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_PW_INVALID)); // Channel password may not contain spaces
			return false;
		}
		if (strcmpi(sub1 + 1, channel->config->local_name) == 0 || strcmpi(sub1 + 1, channel->config->ally_name) == 0 || strdb_exists(channel->db, sub1 + 1)) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_NOT_AVAILABLE), sub1);// Channel '%s' is not available
			clif->message(fd, atcmd_output);
			return false;
		}

		chan = channel->create(HCS_TYPE_PRIVATE, sub1 + 1, 0);
		channel->set_password(chan, pass);
		chan->owner = sd->status.char_id;

		channel->join(chan, sd, pass, false);
	} else if (strcmpi(subcmd,"list") == 0) {
		// sub1 = list type; sub2 = unused; sub3 = unused
		if (sub1[0] != '\0' && strcmpi(sub1,"colors") == 0) {
			for (int k = 0; k < channel->config->colors_count; k++) {
				snprintf(atcmd_output, sizeof(atcmd_output), "[ %s list colors ] : %s", command, channel->config->colors_name[k]);

				clif->messagecolor_self(fd, channel->config->colors[k], atcmd_output);
			}
		} else {
			struct DBIterator *iter = db_iterator(channel->db);
			bool show_all = pc_has_permission(sd, PC_PERM_HCHSYS_ADMIN) ? true : false;
			clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_PUBLIC_LIST)); // -- Public Channels
			if (channel->config->local) {
				snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_LIST_ENTRY), channel->config->local_name, map->list[sd->bl.m].channel ? db_size(map->list[sd->bl.m].channel->users) : 0);// - #%s ( %d users )
				clif->message(fd, atcmd_output);
			}
			if (channel->config->ally && sd->status.guild_id) {
				struct guild *g = sd->guild;
				if( !g ) { dbi_destroy(iter); return false; }
				snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_LIST_ENTRY), channel->config->ally_name, db_size(g->channel->users));// - #%s ( %d users )
				clif->message(fd, atcmd_output);
			}
			for (chan = dbi_first(iter); dbi_exists(iter); chan = dbi_next(iter)) {
				if (show_all || chan->type == HCS_TYPE_PUBLIC || chan->type == HCS_TYPE_IRC) {
					snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_LIST_ENTRY), chan->name, db_size(chan->users));// - #%s ( %d users )
					clif->message(fd, atcmd_output);
				}
			}
			dbi_destroy(iter);
		}
	} else if (strcmpi(subcmd,"setcolor") == 0) {
		// sub1 = channel name; sub2 = color; sub3 = unused
		int k;
		if (sub1[0] != '#') {
			clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_NAME_START));// Channel name must start with a '#'
			return false;
		}

		if (!(chan = channel->search(sub1, sd))) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_NOT_AVAILABLE), sub1);// Channel '%s' is not available
			clif->message(fd, atcmd_output);
			return false;
		}

		if (chan->owner != sd->status.char_id && !pc_has_permission(sd, PC_PERM_HCHSYS_ADMIN)) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_NOT_OWNER), sub1);// You're not the owner of channel '%s'
			clif->message(fd, atcmd_output);
			return false;
		}

		ARR_FIND(0, channel->config->colors_count, k, strcmpi(sub2, channel->config->colors_name[k]) == 0);
		if (k == channel->config->colors_count) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_UNKNOWN_COLOR), sub2);// Unknown color '%s'
			clif->message(fd, atcmd_output);
			return false;
		}
		chan->color = k;
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_COLOR_UPDATED), sub1, channel->config->colors_name[k]);// '%s' channel color updated to '%s'
		clif->message(fd, atcmd_output);
	} else if (strcmpi(subcmd,"leave") == 0) {
		// sub1 = channel name; sub2 = unused; sub3 = unused
		int k;
		if (sub1[0] != '#') {
			clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_NAME_START));// Channel name must start with a '#'
			return false;
		}
		ARR_FIND(0, VECTOR_LENGTH(sd->channels), k, strcmpi(sub1 + 1, VECTOR_INDEX(sd->channels, k)->name) == 0);
		if (k == VECTOR_LENGTH(sd->channels)) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_NOT_PART_OF),sub1);// You're not part of the '%s' channel
			clif->message(fd, atcmd_output);
			return false;
		}
		if (VECTOR_INDEX(sd->channels, k)->type == HCS_TYPE_ALLY) {
			for (k = VECTOR_LENGTH(sd->channels) - 1; k >= 0; k--) {
				// Loop downward to avoid issues when channel->leave() compacts the array
				if (VECTOR_INDEX(sd->channels, k)->type == HCS_TYPE_ALLY) {
					channel->leave(VECTOR_INDEX(sd->channels, k), sd);
				}
			}
		} else {
			channel->leave(VECTOR_INDEX(sd->channels, k), sd);
		}
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_LEFT),sub1); // You've left the '%s' channel
		clif->message(fd, atcmd_output);
	} else if (strcmpi(subcmd,"bindto") == 0) {
		// sub1 = channel name; sub2 = unused; sub3 = unused
		int k;
		if (sub1[0] != '#') {
			clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_NAME_START));// Channel name must start with a '#'
			return false;
		}

		ARR_FIND(0, VECTOR_LENGTH(sd->channels), k, strcmpi(sub1 + 1, VECTOR_INDEX(sd->channels, k)->name) == 0);
		if (k == VECTOR_LENGTH(sd->channels)) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_NOT_PART_OF),sub1);// You're not part of the '%s' channel
			clif->message(fd, atcmd_output);
			return false;
		}

		sd->gcbind = VECTOR_INDEX(sd->channels, k);
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_BIND_SUCCESS),sub1); // Your global chat is now bound to the '%s' channel
		clif->message(fd, atcmd_output);
	} else if (strcmpi(subcmd,"unbind") == 0) {
		// sub1 = unused; sub2 = unused; sub3 = unused
		if (sd->gcbind == NULL) {
			clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_NOT_BOUND));// Your global chat is not bound to any channel
			return false;
		}

		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_UNBIND_SUCCESS),sd->gcbind->name); // Your global chat is no longer bound to the '#%s' channel
		clif->message(fd, atcmd_output);

		sd->gcbind = NULL;
	} else if (strcmpi(subcmd,"ban") == 0) {
		// sub1 = channel name; sub2 = unused; sub3 = unused
		struct map_session_data *pl_sd = NULL;
		char sub4[NAME_LENGTH]; ///< player name
		enum channel_operation_status ret = HCS_STATUS_OK;

		if (sub1[0] != '#') {
			clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_NAME_START));// Channel name must start with a '#'
			return false;
		}

		if (!(chan = channel->search(sub1, sd))) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_NOT_AVAILABLE), sub1);// Channel '%s' is not available
			clif->message(fd, atcmd_output);
			return false;
		}

		if (!*message || sscanf(message, "%19s %19s %23[^\n]", subcmd, sub1, sub4) < 3) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_PLAYER_NOT_FOUND), sub4);// Player '%s' was not found
			clif->message(fd, atcmd_output);
			return false;
		}

		if (sub4[0] == '\0' || (pl_sd = map->nick2sd(sub4, true)) == NULL) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_PLAYER_NOT_FOUND), sub4);// Player '%s' was not found
			clif->message(fd, atcmd_output);
			return false;
		 }

		ret = channel->ban(chan, sd, pl_sd);

		if (ret == HCS_STATUS_NOPERM) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_NOT_OWNER), sub1);// You're not the owner of channel '%s'
			clif->message(fd, atcmd_output);
			return false;
		}

		if (ret == HCS_STATUS_ALREADY) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_PLAYER_BANNED), pl_sd->status.name);// Player '%s' is already banned from this channel
			clif->message(fd, atcmd_output);
			return false;
		}

		if (ret != HCS_STATUS_OK/*ret == HCS_STATUS_FAIL*/) {
			clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_BAN_FAILED)); // Ban failed, not possible to ban this user.
			return false;
		}

		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_BAN_SUCCESS),pl_sd->status.name,sub1); // Player '%s' has now been banned from '%s' channel
		clif->message(fd, atcmd_output);
	} else if (strcmpi(subcmd,"unban") == 0) {
		// sub1 = channel name; sub2 = unused; sub3 = unused
		struct map_session_data *pl_sd = NULL;
		char sub4[NAME_LENGTH]; ///< player name
		enum channel_operation_status ret = HCS_STATUS_OK;

		if (sub1[0] != '#') {
			clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_NAME_START));// Channel name must start with a '#'
			return false;
		}
		if (!(chan = channel->search(sub1, sd))) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_NOT_AVAILABLE), sub1);// Channel '%s' is not available
			clif->message(fd, atcmd_output);
			return false;
		}
		if (!*message || sscanf(message, "%19s %19s %23[^\n]", subcmd, sub1, sub4) < 3) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_PLAYER_NOT_FOUND), sub4);// Player '%s' was not found
			clif->message(fd, atcmd_output);
			return false;
		}
		if (sub4[0] == '\0' || (pl_sd = map->nick2sd(sub4, true)) == NULL) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_PLAYER_NOT_FOUND), sub4);// Player '%s' was not found
			clif->message(fd, atcmd_output);
			return false;
		}

		ret = channel->unban(chan, sd, pl_sd);
		if (ret == HCS_STATUS_NOPERM) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_NOT_OWNER), sub1);// You're not the owner of channel '%s'
			clif->message(fd, atcmd_output);
			return false;
		}
		if (ret == HCS_STATUS_ALREADY) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_PLAYER_NOT_BANNED), pl_sd->status.name);// Player '%s' is not banned from this channel
			clif->message(fd, atcmd_output);
			return false;
		}

		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_UNBAN_SUCCESS),pl_sd->status.name,sub1); // Player '%s' has now been unbanned from the '%s' channel
		clif->message(fd, atcmd_output);
	} else if (strcmpi(subcmd,"unbanall") == 0) {
		enum channel_operation_status ret = HCS_STATUS_OK;
		// sub1 = channel name; sub2 = unused; sub3 = unused
		if (sub1[0] != '#') {
			clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_NAME_START));// Channel name must start with a '#'
			return false;
		}
		if (!(chan = channel->search(sub1, sd))) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_NOT_AVAILABLE), sub1);// Channel '%s' is not available
			clif->message(fd, atcmd_output);
			return false;
		}
		ret = channel->unban(chan, sd, NULL);
		if (ret == HCS_STATUS_NOPERM) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_NOT_OWNER), sub1);// You're not the owner of channel '%s'
			clif->message(fd, atcmd_output);
			return false;
		}
		if (ret == HCS_STATUS_ALREADY) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_NO_BANNED_PLAYERS), sub1);// Channel '%s' has no banned players
			clif->message(fd, atcmd_output);
			return false;
		}

		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_UNBAN_ALL_SUCCESS),sub1); // Removed all bans from '%s' channel
		clif->message(fd, atcmd_output);
	} else if (strcmpi(subcmd,"banlist") == 0) {
		// sub1 = channel name; sub2 = unused; sub3 = unused
		struct DBIterator *iter = NULL;
		union DBKey key;
		struct DBData *data;
		bool isA = pc_has_permission(sd, PC_PERM_HCHSYS_ADMIN)?true:false;
		if (sub1[0] != '#') {
			clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_NAME_START));// Channel name must start with a '#'
			return false;
		}
		if (!(chan = channel->search(sub1, sd))) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_NOT_AVAILABLE), sub1);// Channel '%s' is not available
			clif->message(fd, atcmd_output);
			return false;
		}
		if (chan->owner != sd->status.char_id && !isA) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_NOT_OWNER), sub1);// You're not the owner of channel '%s'
			clif->message(fd, atcmd_output);
			return false;
		}
		if (!chan->banned) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_NO_BANNED_PLAYERS), sub1);// Channel '%s' has no banned players
			clif->message(fd, atcmd_output);
			return false;
		}
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_BAN_LIST), chan->name);// -- '%s' ban list
		clif->message(fd, atcmd_output);

		iter = db_iterator(chan->banned);
		for (data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key)) {
			struct channel_ban_entry *entry = DB->data2ptr(data);

			if (!isA)
				snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_BAN_LIST_ENTRY), entry->name);// - %s %s
			else
				snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_BAN_LIST_ENTRY_VALUE), entry->name, key.i);// - %s (%d)

			clif->message(fd, atcmd_output);
		}
		dbi_destroy(iter);
	} else if (strcmpi(subcmd,"setopt") == 0) {
		// sub1 = channel name; sub2 = option name; sub3 = value
		int k;
		const char* opt_str[3] = {
			"None",
			"JoinAnnounce",
			"MessageDelay",
		};
		if (sub1[0] != '#') {
			clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_NAME_START));// Channel name must start with a '#'
			return false;
		}
		if (!(chan = channel->search(sub1, sd))) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_NOT_AVAILABLE), sub1);// Channel '%s' is not available
			clif->message(fd, atcmd_output);
			return false;
		}
		if (chan->owner != sd->status.char_id && !pc_has_permission(sd, PC_PERM_HCHSYS_ADMIN)) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_NOT_OWNER), sub1);// You're not the owner of channel '%s'
			clif->message(fd, atcmd_output);
			return false;
		}
		if (sub2[0] == '\0') {
			clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_OPT_NO_INPUT));// You need to input a option
			return false;
		}
		for (k = 1; k < 3; k++) {
			if (strcmpi(sub2,opt_str[k]) == 0)
				break;
		}
		if (k == 3) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_UNKNOWN_OPT), sub2);// '%s' is not a known channel option
			clif->message(fd, atcmd_output);
			clif->message(fd, msg_fd(fd, MSGTBL_CHANNEL_AVAILABLE_OPT2)); // -- Available options
			for (k = 1; k < 3; k++) {
				snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_BAN_LIST_ENTRY), opt_str[k]);// - '%s'
				clif->message(fd, atcmd_output);
			}
			return false;
		}
		if (sub3[0] == '\0') {
			if (k == HCS_OPT_MSG_DELAY) {
				snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_SECONDS_RANGE), opt_str[k]);// For '%s' you need the amount of seconds (from 0 to 10)
				clif->message(fd, atcmd_output);
				return false;
			} else if (chan->options & k) {
				snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_OPT_ALREADY_ENABLED), opt_str[k],opt_str[k]); // option '%s' is already enabled, if you'd like to disable it type '@channel setopt %s 0'
				clif->message(fd, atcmd_output);
				return false;
			} else {
				channel->set_options(chan, chan->options | k);
				snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_OPT_ENABLED), opt_str[k],chan->name);//option '%s' is now enabled for channel '%s'
				clif->message(fd, atcmd_output);
				return true;
			}
		} else {
			int v = atoi(sub3);
			if (k == HCS_OPT_MSG_DELAY) {
				if (v < 0 || v > channel->config->channel_opt_msg_delay) {
					snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_OPT_VALUE_RANGE), v, opt_str[k], channel->config->channel_opt_msg_delay);// value '%d' for option '%s' is out of range (limit is 0-%d)
					clif->message(fd, atcmd_output);
					return false;
				}
				if (v == 0) {
					channel->set_options(chan, chan->options&~k);
					chan->msg_delay = 0;
					snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_OPT_DISABLED), opt_str[k],chan->name,v);// option '%s' is now disabled for channel '%s'
					clif->message(fd, atcmd_output);
					return true;
				} else {
					channel->set_options(chan, chan->options | k);
					chan->msg_delay = v;
					snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_OPT_WITH_DURATION), opt_str[k],chan->name,v);// option '%s' is now enabled for channel '%s' with %d seconds
					clif->message(fd, atcmd_output);
					return true;
				}
			} else {
				if (v) {
					if (chan->options & k) {
						snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_OPT_ALREADY_ENABLED), opt_str[k],opt_str[k]); // option '%s' is already enabled, if you'd like to disable it type '@channel opt %s 0'
						clif->message(fd, atcmd_output);
						return false;
					} else {
						channel->set_options(chan, chan->options | k);
						snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_OPT_ENABLED), opt_str[k],chan->name);//option '%s' is now enabled for channel '%s'
						clif->message(fd, atcmd_output);
					}
				} else {
					if (!(chan->options & k)) {
						snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_OPT_NOT_ENABLED), opt_str[k],chan->name); // option '%s' is not enabled on channel '%s'
						clif->message(fd, atcmd_output);
						return false;
					} else {
						channel->set_options(chan, chan->options&~k);
						snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_OPT_DISABLED), opt_str[k],chan->name);// option '%s' is now disabled for channel '%s'
						clif->message(fd, atcmd_output);
						return true;
					}
				}
			}
		}
	} else {
		atcommand->channel_help(fd, command, (channel->config->allow_user_channel_creation || pc_has_permission(sd, PC_PERM_HCHSYS_ADMIN)));
	}
	return true;
}

/* debug only, delete after */
ACMD(fontcolor)
{
	unsigned char k;

	if (!*message) {
		for (k = 0; k < channel->config->colors_count; k++) {
			snprintf(atcmd_output, sizeof(atcmd_output), "[ %s ] : %s", command, channel->config->colors_name[k]);
			clif->messagecolor_self(fd, channel->config->colors[k], atcmd_output);
		}
		return false;
	}

	if( message[0] == '0' ) {
		sd->fontcolor = 0;
		return true;
	}

	for( k = 0; k < channel->config->colors_count; k++ ) {
		if (strcmpi(message, channel->config->colors_name[k]) == 0)
			break;
	}
	if( k == channel->config->colors_count ) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_CHANNEL_UNKNOWN_COLOR), message);// Unknown color '%s'
		clif->message(fd, atcmd_output);
		return false;
	}

	sd->fontcolor = k + 1;
	snprintf(atcmd_output, sizeof(atcmd_output), "Color changed to '%s'", channel->config->colors_name[k]);
	clif->messagecolor_self(fd, channel->config->colors[k], atcmd_output);

	return true;
}

ACMD(searchstore)
{
	int val = atoi(message);

	switch (val) {
		case 0://EFFECTTYPE_NORMAL
		case 1://EFFECTTYPE_CASH
			break;
		default:
			val = 0;
			break;
	}

	searchstore->open(sd, 99, val);
	return true;
}

/*==========================================
* @costume
*------------------------------------------*/
ACMD(costume)
{
	const char* names[] = {
		"Wedding",
		"Xmas",
		"Summer",
		"Hanbok",
#if PACKETVER >= 20131218
		"Oktoberfest",
#endif
#if PACKETVER >= 20141022
		"Summer2",
#endif
	};
	const int name2id[] = {
		SC_WEDDING,
		SC_XMAS,
		SC_SUMMER,
		SC_HANBOK,
#if PACKETVER >= 20131218
		SC_OKTOBERFEST,
#endif
#if PACKETVER >= 20141022
		SC_DRESS_UP,
#endif
	};
	unsigned short k = 0, len = ARRAYLENGTH(names);

	bool isChangeDress = (strcmpi(info->command, "changedress") == 0 || strcmpi(info->command, "nocosplay") == 0);

	if (!*message) {
		for (k = 0; k < len; k++) {
			if (sd->sc.data[name2id[k]]) {
				snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_COSTUME_REMOVED), names[k]); // Costume '%s' removed.
				clif->message(sd->fd, atcmd_output);
				status_change_end(&sd->bl, name2id[k], INVALID_TIMER);
				return true;
			}
		}

		if (isChangeDress)
			return true;
		clif->message(sd->fd, msg_fd(fd, MSGTBL_COSTUME_AVAILABLE)); // - Available Costumes

		for (k = 0; k < len; k++) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_COSTUME_HEADER), names[k]); //-- %s
			clif->message(sd->fd, atcmd_output);
		}
		return false;
	}

	if (isChangeDress)
		return true;

	for (k = 0; k < len; k++) {
		if (sd->sc.data[name2id[k]]) {
			snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_COSTUME_ALREADY), names[k]); // You're already with a '%s' costume, type '@costume' to remove it.
			clif->message(sd->fd, atcmd_output);
			return false;
		}
	}

	for (k = 0; k < len; k++) {
		if (strcmpi(message,names[k]) == 0)
			break;
	}

	if (k == len) {
		snprintf(atcmd_output, sizeof(atcmd_output), msg_fd(fd, MSGTBL_COSTUME_UNKNOWN), message); // '%s' is not a known costume
		clif->message(sd->fd, atcmd_output);
		return false;
	}

	sc_start(NULL, &sd->bl, name2id[k], 100, 0, INFINITE_DURATION, 0);

	return true;
}

/* for debugging purposes (so users can easily provide us with debug info) */
/* should be trashed as soon as its no longer necessary */
ACMD(skdebug)
{
	snprintf(atcmd_output, sizeof(atcmd_output),"second: %d; third: %d", sd->sktree.second, sd->sktree.third);
	clif->message(fd,atcmd_output);
	snprintf(atcmd_output, sizeof(atcmd_output),"pc_calc_skilltree_normalize_job: %d",pc->calc_skilltree_normalize_job(sd));
	clif->message(fd,atcmd_output);
	snprintf(atcmd_output, sizeof(atcmd_output),"change_lv_2nd/3rd: %d/%d",sd->change_level_2nd,sd->change_level_3rd);
	clif->message(fd,atcmd_output);
	snprintf(atcmd_output, sizeof(atcmd_output),"pc_calc_skillpoint:%d",pc->calc_skillpoint(sd));
	clif->message(fd,atcmd_output);
	return true;
}

/**
 * cooldown-debug
 **/
ACMD(cddebug)
{
	int i;
	struct skill_cd* cd = NULL;

	if (!(cd = idb_get(skill->cd_db,sd->status.char_id))) {
		clif->message(fd,"No cool down list found");
	} else {
		clif->messages(fd,"Found %d registered cooldowns",cd->cursor);
		for(i = 0; i < cd->cursor; i++) {
			if( cd->entry[i] ) {
				const struct TimerData *td = timer->get(cd->entry[i]->timer);

				if( !td || td->func != skill->blockpc_end ) {
					clif->messages(fd,"Found invalid entry in slot %d for skill %s",i,skill->dbs->db[cd->entry[i]->skidx].name);
					sd->blockskill[cd->entry[i]->skidx] = false;
				}
			}
		}
	}

	if (!cd || (*message && !strcmpi(message,"reset"))) {
		for (i = 0; i < MAX_SKILL_DB; i++) {
			if( sd->blockskill[i] ) {
				clif->messages(fd,"Found skill '%s', unblocking...",skill->dbs->db[i].name);
				sd->blockskill[i] = false;
			}
		}
		if( cd ) {//reset
			for(i = 0; i < cd->cursor; i++) {
				if( !cd->entry[i] ) continue;
				timer->delete(cd->entry[i]->timer,skill->blockpc_end);
				ers_free(skill->cd_entry_ers, cd->entry[i]);
			}

			idb_remove(skill->cd_db,sd->status.char_id);
			ers_free(skill->cd_ers, cd);
		}
	}

	return true;
}

/**
 *
 **/
ACMD(lang)
{
	uint8 i;

	if (!*message) {
		clif->messages(fd,"Usage: @%s <Language>",info->command);
		clif->messages(fd,"There are %d languages available:",script->max_lang_id);
		for(i = 0; i < script->max_lang_id; i++)
			clif->messages(fd,"- %s",script->languages[i]);
		return false;
	}

	for(i = 0; i < script->max_lang_id; i++) {
		if( strcmpi(message,script->languages[i]) == 0 ) {
			if( i == sd->lang_id ) {
				clif->messages(fd,"%s is already set as your language",script->languages[i]);
			} else {
				clif->messages(fd,"Your language has been changed from '%s' to '%s'",script->languages[sd->lang_id],script->languages[i]);
				sd->lang_id = i;
			}
			break;
		}
	}

	if( i == script->max_lang_id ) {
		clif->messages(fd,"'%s' did not match any language available",message);
		clif->messages(fd,"There are %d languages available:",script->max_lang_id);
		for(i = 0; i < script->max_lang_id; i++)
			clif->messages(fd,"- %s",script->languages[i]);
	}

	return true;
}

ACMD(claninfo)
{
	struct DBIterator *iter = db_iterator(clan->db);
	struct clan *c;
	int i, count;

	for (c = dbi_first(iter); dbi_exists(iter); c = dbi_next(iter)) {
		snprintf(atcmd_output, sizeof(atcmd_output), "Clan #%d:", c->clan_id);
		clif->messagecolor_self(fd, COLOR_DEFAULT, atcmd_output);

		snprintf(atcmd_output, sizeof(atcmd_output), "- Name: %s", c->name);
		clif->messagecolor_self(fd, COLOR_DEFAULT, atcmd_output);

		snprintf(atcmd_output, sizeof(atcmd_output), "- Constant: %s", c->constant);
		clif->messagecolor_self(fd, COLOR_DEFAULT, atcmd_output);

		snprintf(atcmd_output, sizeof(atcmd_output), "- Master: %s", c->master);
		clif->messagecolor_self(fd, COLOR_DEFAULT, atcmd_output);

		snprintf(atcmd_output, sizeof(atcmd_output), "- Map: %s", c->map);
		clif->messagecolor_self(fd, COLOR_DEFAULT, atcmd_output);

		snprintf(atcmd_output, sizeof(atcmd_output), "- Max Member: %d", c->max_member);
		clif->messagecolor_self(fd, COLOR_DEFAULT, atcmd_output);

		snprintf(atcmd_output, sizeof(atcmd_output), "- Kick Time: %dh", c->kick_time / 3600);
		clif->messagecolor_self(fd, COLOR_DEFAULT, atcmd_output);

		snprintf(atcmd_output, sizeof(atcmd_output), "- Check Time: %dh", c->check_time / 3600000);
		clif->messagecolor_self(fd, COLOR_DEFAULT, atcmd_output);

		snprintf(atcmd_output, sizeof(atcmd_output), "- Connected Members: %d", c->connect_member);
		clif->messagecolor_self(fd, COLOR_DEFAULT, atcmd_output);

		snprintf(atcmd_output, sizeof(atcmd_output), "- Total Members: %d", c->member_count);
		clif->messagecolor_self(fd, COLOR_DEFAULT, atcmd_output);

		snprintf(atcmd_output, sizeof(atcmd_output), "- Allies: %d", VECTOR_LENGTH(c->allies));
		clif->messagecolor_self(fd, COLOR_DEFAULT, atcmd_output);

		count = 0;
		for (i = 0; i < VECTOR_LENGTH(c->allies); i++) {
			struct clan_relationship *ally = &VECTOR_INDEX(c->allies, i);

			snprintf(atcmd_output, sizeof(atcmd_output), "- - Ally #%d (Id: %d): %s", i + 1, ally->clan_id, ally->constant);
			clif->messagecolor_self(fd, COLOR_DEFAULT, atcmd_output);
			count++;
		}

		if (count == 0) {
			clif->messagecolor_self(fd, COLOR_DEFAULT, "- - No Allies Found!");
		}

		snprintf(atcmd_output, sizeof(atcmd_output), "- Antagonists: %d", VECTOR_LENGTH(c->antagonists));
		clif->messagecolor_self(fd, COLOR_DEFAULT, atcmd_output);

		count = 0;
		for (i = 0; i < VECTOR_LENGTH(c->antagonists); i++) {
			struct clan_relationship *antagonist = &VECTOR_INDEX(c->antagonists, i);

			snprintf(atcmd_output, sizeof(atcmd_output), "- - Antagonist #%d (Id: %d): %s", i + 1, antagonist->clan_id, antagonist->constant);
			clif->messagecolor_self(fd, COLOR_DEFAULT, atcmd_output);
			count++;
		}

		if (count == 0) {
			clif->messagecolor_self(fd, COLOR_DEFAULT, "- - No Antagonists Found!");
		}

		clif->messagecolor_self(fd, COLOR_DEFAULT, "============================");
	}
	dbi_destroy(iter);
	return true;
}

/**
 * Clan System: Joins in the given clan
 */
ACMD(joinclan)
{
	int clan_id;

	if (*message == '\0') {
		clif->message(fd, msg_sd(sd, MSGTBL_JOINCLAN_USAGE)); // "Please enter a Clan ID (usage: @joinclan <clan ID>)."
		return false;
	}
	if (sd->status.clan_id != 0) {
		clif->messagecolor_self(fd, COLOR_RED, msg_sd(sd, MSGTBL_JOINCLAN_ALREADY_IN_CLAN)); // "You are already in a clan."
		return false;
	} else if (sd->status.guild_id != 0) {
		clif->messagecolor_self(fd, COLOR_RED, msg_sd(sd, MSGTBL_JOINCLAN_MUST_LEAVE_GUILD)); // "You must leave your guild before enter in a clan."
		return false;
	}

	clan_id = atoi(message);
	if (clan_id <= 0) {
		clif->messagecolor_self(fd, COLOR_RED, msg_sd(sd, MSGTBL_JOINCLAN_INVALID_CLAN)); // "Invalid Clan ID."
		return false;
	}
	if (!clan->join(sd, clan_id)) {
		clif->messagecolor_self(fd, COLOR_RED, msg_sd(sd, MSGTBL_JOINCLAN_FAILED)); // "The clan couldn't be joined."
		return false;
	}
	return true;
}

/**
 * Clan System: Leaves current clan
 */
ACMD(leaveclan)
{
	if (sd->status.clan_id == 0) {
		clif->messagecolor_self(fd, COLOR_RED, msg_sd(sd, MSGTBL_LEAVECLAN_NOT_IN_CLAN)); // "You aren't in a clan."
		return false;
	}
	if (!clan->leave(sd, false)) {
		clif->messagecolor_self(fd, COLOR_RED, msg_sd(sd, MSGTBL_LEAVECLAN_FAILED)); // "Failed on leaving clan."
		return false;
	}
	return true;
}

/**
 * Clan System: Reloads clan db
 */
ACMD(reloadclans)
{
	clan->reload();
	clif->messagecolor_self(fd, COLOR_DEFAULT, msg_sd(sd, MSGTBL_RELOAD_CLAN_RELOADED)); // "Clan configuration and database have been reloaded."
	return true;
}

// show camera window or change camera parameters
ACMD(camerainfo)
{
	if (*message == '\0') {
		clif->camera_showWindow(sd);
		return true;
	}
	float range = 0;
	float rotation = 0;
	float latitude = 0;
	if (sscanf(message, "%15f %15f %15f", &range, &rotation, &latitude) < 3) {
		clif->message(fd, msg_fd(fd, MSGTBL_CAMERAINFO_USAGE));  // usage @camerainfo range rotation latitude
		return false;
	}
	clif->camera_change(sd, range, rotation, latitude, SELF);
	return true;
}

ACMD(refineryui)
{
#if PACKETVER_MAIN_NUM >= 20161005 || PACKETVER_RE_NUM >= 20161005 || defined(PACKETVER_ZERO)
	if (battle_config.enable_refinery_ui == 0) {
		clif->message(fd, msg_fd(fd, MSGTBL_REFINEUI_NOT_AVAILABLE));
		return false;
	}

	clif->OpenRefineryUI(sd);
	return true;
#else
	clif->message(fd, msg_fd(fd, MSGTBL_REFINEUI_NOT_AVAILABLE));
	return false;
#endif
}

ACMD(gradeui)
{
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
	clif->open_ui_send(sd, ZC_GRADE_ENCHANT_UI);
	return true;
#else
	clif->message(fd, "Grade Enchant UI is not supported.");
	return false;
#endif
}

ACMD(reloadgradedb)
{
	grader->reload_db();
	clif->message(fd, "Grade Database has been reloaded.");
	return true;
}

ACMD(itemreform)
{
#if PACKETVER_MAIN_NUM >= 20201118 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
	struct item_data *item_data;

	if ((item_data = itemdb->search_name(message)) == NULL &&
		(item_data = itemdb->exists(atoi(message))) == NULL)
	{
		clif->message(fd, msg_fd(fd, MSGTBL_INVALID_ITEMID_NAME)); // Invalid item ID or name.
		return false;
	}
	if (VECTOR_LENGTH(item_data->reform_list) == 0) {
		clif->message(fd, "No reforms available for given item.");
		return false;
	}

	clif->item_reform_open(sd, item_data->nameid);
	return true;
#else
	clif->message(fd, "Item Reform UI is not supported.");
	return false;
#endif
}

ACMD(enchantui)
{
#if PACKETVER_MAIN_NUM >= 20210203 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
	int egroup = 0;
	if (!*message || !(egroup = atoi(message))) {
		clif->message(fd, msg_fd(fd, MSGTBL_ENCHANT_GROUP_ID_REQUIRED));
		return false;
	}

	clif->enchantui_open(sd, egroup);
		return true;
#else
	clif->message(fd, "Enchant UI is not supported.");
	return false;
#endif
}

/**
 * Fills the reference of available commands in atcommand DBMap
 **/
#define ACMD_DEF(x) { #x, atcommand_ ## x, NULL, NULL, NULL, true }
#define ACMD_DEF2(x2, x) { x2, atcommand_ ## x, NULL, NULL, NULL, true }
static void atcommand_basecommands(void)
{
	/**
	 * Command reference list, place the base of your commands here
	 **/
	AtCommandInfo atcommand_base[] = {
		ACMD_DEF2("warp", mapmove),
		ACMD_DEF(where),
		ACMD_DEF(jumpto),
		ACMD_DEF(jump),
		ACMD_DEF(who),
		ACMD_DEF2("who2", who),
		ACMD_DEF2("who3", who),
		ACMD_DEF2("whomap", who),
		ACMD_DEF2("whomap2", who),
		ACMD_DEF2("whomap3", who),
		ACMD_DEF(whogm),
		ACMD_DEF(save),
		ACMD_DEF(load),
		ACMD_DEF(speed),
		ACMD_DEF(storage),
		ACMD_DEF(guildstorage),
		ACMD_DEF(option),
		ACMD_DEF(hide), // + /hide
		ACMD_DEF(jobchange),
		ACMD_DEF(kill),
		ACMD_DEF(alive),
		ACMD_DEF(kami),
		ACMD_DEF2("kamib", kami),
		ACMD_DEF2("kamic", kami),
		ACMD_DEF2("lkami", kami),
		ACMD_DEF(heal),
		ACMD_DEF(item),
		ACMD_DEF(item2),
		ACMD_DEF2("itembound", item),
		ACMD_DEF2("itembound2", item2),
		ACMD_DEF(itemreset),
		ACMD_DEF(clearstorage),
		ACMD_DEF(cleargstorage),
		ACMD_DEF(clearcart),
		ACMD_DEF2("blvl", baselevelup),
		ACMD_DEF2("jlvl", joblevelup),
		ACMD_DEF(help),
		ACMD_DEF(pvpoff),
		ACMD_DEF(pvpon),
		ACMD_DEF(gvgoff),
		ACMD_DEF(gvgon),
		ACMD_DEF(model),
		ACMD_DEF(go),
		ACMD_DEF(monster),
		ACMD_DEF2("monstersmall", monster),
		ACMD_DEF2("monsterbig", monster),
		ACMD_DEF(killmonster),
		ACMD_DEF2("killmonster2", killmonster),
		ACMD_DEF(refine),
		ACMD_DEF(grade),
		ACMD_DEF(produce),
		ACMD_DEF(memo),
		ACMD_DEF(gat),
		ACMD_DEF(displaystatus),
		ACMD_DEF2("stpoint", statuspoint),
		ACMD_DEF2("skpoint", skillpoint),
		ACMD_DEF(zeny),
		ACMD_DEF2("str", param),
		ACMD_DEF2("agi", param),
		ACMD_DEF2("vit", param),
		ACMD_DEF2("int", param),
		ACMD_DEF2("dex", param),
		ACMD_DEF2("luk", param),
		ACMD_DEF2("glvl", guildlevelup),
		ACMD_DEF(makeegg),
		ACMD_DEF(hatch),
		ACMD_DEF(petfriendly),
		ACMD_DEF(pethungry),
		ACMD_DEF(petrename),
		ACMD_DEF(recall), // + /recall
		ACMD_DEF(night),
		ACMD_DEF(day),
		ACMD_DEF(doom),
		ACMD_DEF(doommap),
		ACMD_DEF(raise),
		ACMD_DEF(raisemap),
		ACMD_DEF(kick), // + right click menu for GM "(name) force to quit"
		ACMD_DEF(kickall),
		ACMD_DEF(allskill),
		ACMD_DEF(questskill),
		ACMD_DEF(lostskill),
		ACMD_DEF(spiritball),
		ACMD_DEF(soulball),
		ACMD_DEF(party),
		ACMD_DEF(guild),
		ACMD_DEF(breakguild),
		ACMD_DEF(agitstart),
		ACMD_DEF(agitend),
		ACMD_DEF(mapexit),
		ACMD_DEF(idsearch),
		ACMD_DEF(broadcast), // + /b and /nb
		ACMD_DEF(localbroadcast), // + /lb and /nlb
		ACMD_DEF(recallall),
		ACMD_DEF(reloaditemdb),
		ACMD_DEF(reloadmobdb),
		ACMD_DEF(reloadskilldb),
		ACMD_DEF(reloadscript),
		ACMD_DEF(reloadatcommand),
		ACMD_DEF(reloadbattleconf),
		ACMD_DEF(reloadstatusdb),
		ACMD_DEF(reloadpcdb),
		ACMD_DEF(mapinfo),
		ACMD_DEF(dye),
		ACMD_DEF2("hairstyle", hair_style),
		ACMD_DEF2("haircolor", hair_color),
		ACMD_DEF2("allstats", stat_all),
		ACMD_DEF2("block", char_block),
		ACMD_DEF2("ban", char_ban),
		ACMD_DEF2("charban", char_ban),/* char-specific ban time */
		ACMD_DEF2("unblock", char_unblock),
		ACMD_DEF2("charunban", char_unban),/* char-specific ban time */
		ACMD_DEF2("unban", char_unban),
		ACMD_DEF2("mount", mount_peco),
		ACMD_DEF(guildspy),
		ACMD_DEF(partyspy),
		ACMD_DEF(repairall),
		ACMD_DEF(guildrecall),
		ACMD_DEF(partyrecall),
		ACMD_DEF(nuke),
		ACMD_DEF(shownpc),
		ACMD_DEF(hidenpc),
		ACMD_DEF(loadnpc),
		ACMD_DEF(unloadnpc),
		ACMD_DEF2("time", servertime),
		ACMD_DEF(jail),
		ACMD_DEF(unjail),
		ACMD_DEF(jailfor),
		ACMD_DEF(jailtime),
		ACMD_DEF(disguise),
		ACMD_DEF(undisguise),
		ACMD_DEF(email),
		ACMD_DEF(effect),
		ACMD_DEF(follow),
		ACMD_DEF(addwarp),
		ACMD_DEF(skillon),
		ACMD_DEF(skilloff),
		ACMD_DEF(killer),
		ACMD_DEF(npcmove),
		ACMD_DEF(killable),
		ACMD_DEF(dropall),
		ACMD_DEF(storeall),
		ACMD_DEF(skillid),
		ACMD_DEF(useskill),
		ACMD_DEF(displayskill),
		ACMD_DEF(snow),
		ACMD_DEF(sakura),
		ACMD_DEF(clouds),
		ACMD_DEF(clouds2),
		ACMD_DEF(fog),
		ACMD_DEF(fireworks),
		ACMD_DEF(leaves),
		ACMD_DEF(summon),
		ACMD_DEF(adjgroup),
		ACMD_DEF(trade),
		ACMD_DEF(send),
		ACMD_DEF(setbattleflag),
		ACMD_DEF(unmute),
		ACMD_DEF(clearweather),
		ACMD_DEF(uptime),
		ACMD_DEF(changesex),
		ACMD_DEF(changecharsex),
		ACMD_DEF(unequipall),
		ACMD_DEF(mute),
		ACMD_DEF(refresh),
		ACMD_DEF(refreshall),
		ACMD_DEF(identify),
		ACMD_DEF2("identifyall", identify),
		ACMD_DEF(misceffect),
		ACMD_DEF(mobsearch),
		ACMD_DEF(cleanmap),
		ACMD_DEF(cleanarea),
		ACMD_DEF(npctalk),
		ACMD_DEF(pettalk),
		ACMD_DEF(users),
		ACMD_DEF(reset),
		ACMD_DEF(skilltree),
		ACMD_DEF(marry),
		ACMD_DEF(divorce),
		ACMD_DEF(sound),
		ACMD_DEF(undisguiseall),
		ACMD_DEF(disguiseall),
		ACMD_DEF(changelook),
		ACMD_DEF(autoloot),
		ACMD_DEF2("alootid", autolootitem),
		ACMD_DEF(autoloottype),
		ACMD_DEF(mobinfo),
		ACMD_DEF(exp),
		ACMD_DEF(version),
		ACMD_DEF(mutearea),
		ACMD_DEF(rates),
		ACMD_DEF(iteminfo),
		ACMD_DEF(whodrops),
		ACMD_DEF(whereis),
		ACMD_DEF(mapflag),
		ACMD_DEF(me),
		ACMD_DEF(monsterignore),
		ACMD_DEF(fakename),
		ACMD_DEF(size),
		ACMD_DEF(showexp),
		ACMD_DEF(showzeny),
		ACMD_DEF(showdelay),
		ACMD_DEF(autotrade),
		ACMD_DEF(changegm),
		ACMD_DEF(changeleader),
		ACMD_DEF(partyoption),
		ACMD_DEF(invite),
		ACMD_DEF(duel),
		ACMD_DEF(leave),
		ACMD_DEF(accept),
		ACMD_DEF(reject),
		ACMD_DEF(clone),
		ACMD_DEF2("slaveclone", clone),
		ACMD_DEF2("evilclone", clone),
		ACMD_DEF(tonpc),
		ACMD_DEF(commands),
		ACMD_DEF(noask),
		ACMD_DEF(request),
		ACMD_DEF(homlevel),
		ACMD_DEF(homevolution),
		ACMD_DEF(hommutate),
		ACMD_DEF(makehomun),
		ACMD_DEF(homfriendly),
		ACMD_DEF(homhungry),
		ACMD_DEF(homtalk),
		ACMD_DEF(hominfo),
		ACMD_DEF(homstats),
		ACMD_DEF(homshuffle),
		ACMD_DEF(showmobs),
		ACMD_DEF(feelreset),
		ACMD_DEF(hatereset),
		ACMD_DEF(auction),
		ACMD_DEF(mail),
		ACMD_DEF2("noks", ksprotection),
		ACMD_DEF(allowks),
		ACMD_DEF(cash),
		ACMD_DEF2("points", cash),
		ACMD_DEF(agitstart2),
		ACMD_DEF(agitend2),
		ACMD_DEF2("skreset", resetskill),
		ACMD_DEF2("streset", resetstat),
		ACMD_DEF2("storagelist", itemlist),
		ACMD_DEF2("cartlist", itemlist),
		ACMD_DEF2("itemlist", itemlist),
		ACMD_DEF(stats),
		ACMD_DEF(delitem),
		ACMD_DEF(charcommands),
		ACMD_DEF(font),
		ACMD_DEF(accinfo),
		ACMD_DEF(set),
		ACMD_DEF(quest),
		ACMD_DEF(reloadquestdb),
		ACMD_DEF(undisguiseguild),
		ACMD_DEF(disguiseguild),
		ACMD_DEF(sizeall),
		ACMD_DEF(sizeguild),
		ACMD_DEF(addperm),
		ACMD_DEF2("rmvperm", addperm),
		ACMD_DEF(unloadnpcfile),
		ACMD_DEF(reloadnpc),
		ACMD_DEF(cart),
		ACMD_DEF(cashmount),
		ACMD_DEF(join),
		ACMD_DEF(channel),
		ACMD_DEF(fontcolor),
		ACMD_DEF(searchstore),
		ACMD_DEF(costume),
		ACMD_DEF2("changedress", costume),
		ACMD_DEF2("nocosplay", costume),
		ACMD_DEF(skdebug),
		ACMD_DEF(cddebug),
		ACMD_DEF(lang),
		ACMD_DEF(bodystyle),
		ACMD_DEF(cvcoff),
		ACMD_DEF(cvcon),
		ACMD_DEF(claninfo),
		ACMD_DEF(joinclan),
		ACMD_DEF(leaveclan),
		ACMD_DEF(reloadclans),
		ACMD_DEF(setzone),
		ACMD_DEF(camerainfo),
		ACMD_DEF(refineryui),
		ACMD_DEF(gradeui),
		ACMD_DEF(reloadgradedb),
		ACMD_DEF(itemreform),
		ACMD_DEF(enchantui),
	};
	int i;

	for( i = 0; i < ARRAYLENGTH(atcommand_base); i++ ) {
		if(!atcommand->add(atcommand_base[i].command,atcommand_base[i].func,false)) { // Should not happen if atcommand_base[] array is OK
			ShowDebug("atcommand_basecommands: duplicate ACMD_DEF for '%s'.\n", atcommand_base[i].command);
			continue;
		}
	}

	/* @commands from plugins */
	HPM_map_atcommands();

	return;
}
#undef ACMD_DEF
#undef ACMD_DEF2

static bool atcommand_add(char *name, AtCommandFunc func, bool replace)
{
	AtCommandInfo* cmd;

	nullpo_retr(false, name);
	if( (cmd = atcommand->exists(name)) ) { //caller will handle/display on false
		if( !replace )
			return false;
	} else {
		CREATE(cmd, AtCommandInfo, 1);
		strdb_put(atcommand->db, name, cmd);
	}

	safestrncpy(cmd->command, name, sizeof(cmd->command));
	cmd->func = func;
	cmd->help = NULL;
	cmd->log = true;

	return true;
}

/*==========================================
 * Command lookup functions
 *------------------------------------------*/
static AtCommandInfo *atcommand_exists(const char *name)
{
	return strdb_get(atcommand->db, name);
}

static AtCommandInfo *get_atcommandinfo_byname(const char *name)
{
	AtCommandInfo *cmd;
	if ((cmd = strdb_get(atcommand->db, name)))
		return cmd;
	return NULL;
}

static const char *atcommand_checkalias(const char *aliasname)
{
	AliasInfo *alias_info = NULL;
	if ((alias_info = (AliasInfo*)strdb_get(atcommand->alias_db, aliasname)) != NULL)
		return alias_info->command->command;
	return aliasname;
}

/// AtCommand suggestion
static void atcommand_get_suggestions(struct map_session_data *sd, const char *name, bool is_atcmd_cmd)
{
	struct DBIterator *atcommand_iter, *alias_iter;
	AtCommandInfo* command_info = NULL;
	AliasInfo* alias_info = NULL;
	AtCommandType type = is_atcmd_cmd ? COMMAND_ATCOMMAND : COMMAND_CHARCOMMAND;
	char* full_match[MAX_SUGGESTIONS];
	char* suggestions[MAX_SUGGESTIONS];
	char* match;
	int prefix_count = 0, full_count = 0;
	bool can_use;

	if (!battle_config.atcommand_suggestions_enabled)
		return;

	atcommand_iter = db_iterator(atcommand->db);
	alias_iter = db_iterator(atcommand->alias_db);

	// Build the matches
	for (command_info = dbi_first(atcommand_iter); dbi_exists(atcommand_iter); command_info = dbi_next(atcommand_iter))     {
		match = strstr(command_info->command, name);
		can_use = atcommand->can_use2(sd, command_info->command, type);
		if ( prefix_count < MAX_SUGGESTIONS && match == command_info->command && can_use ) {
			suggestions[prefix_count] = command_info->command;
			++prefix_count;
		}
		if ( full_count < MAX_SUGGESTIONS && match != NULL && match != command_info->command && can_use ) {
			full_match[full_count] = command_info->command;
			++full_count;
		}
	}

	for (alias_info = dbi_first(alias_iter); dbi_exists(alias_iter); alias_info = dbi_next(alias_iter)) {
		match = strstr(alias_info->alias, name);
		can_use = atcommand->can_use2(sd, alias_info->command->command,type);
		if ( prefix_count < MAX_SUGGESTIONS && match == alias_info->alias && can_use) {
			suggestions[prefix_count] = alias_info->alias;
			++prefix_count;
		}
		if ( full_count < MAX_SUGGESTIONS && match != NULL && match != alias_info->alias && can_use ) {
			full_match[full_count] = alias_info->alias;
			++full_count;
		}
	}

	if ((full_count+prefix_count) > 0) {
		char buffer[512];
		int i;

		// Merge full match and prefix match results
		if (prefix_count < MAX_SUGGESTIONS) {
			memmove(&suggestions[prefix_count], full_match, sizeof(char*) * (MAX_SUGGESTIONS-prefix_count));
			prefix_count = min(prefix_count+full_count, MAX_SUGGESTIONS);
		}

		// Build the suggestion string
		strcpy(buffer, msg_sd(sd, MSGTBL_MAYBE_YOU_MEANT));
		strcat(buffer,"\n");

		for(i=0; i < prefix_count; ++i) {
			strcat(buffer,suggestions[i]);
			strcat(buffer," ");
		}

		clif->message(sd->fd, buffer);
	}

	dbi_destroy(atcommand_iter);
	dbi_destroy(alias_iter);
}

/**
 * Executes an at-command.
 *
 * @param fd             fd associated to the invoking character
 * @param sd             sd associated to the invoking character
 * @param message        atcommand arguments
 * @param player_invoked true if the command was invoked by a player, false if invoked by the server (bypassing any restrictions)
 *
 * @retval true if the message was recognized as atcommand.
 * @retval false if the message should be considered a non-command message.
 */
static bool atcommand_exec(const int fd, struct map_session_data *sd, const char *message, bool player_invoked)
{
	char params[100], command[100];
	char output[CHAT_SIZE_MAX];
	bool logCommand;

	// Reconstructed message
	char atcmd_msg[CHAT_SIZE_MAX];

	struct map_session_data *ssd = NULL; //sd for target
	AtCommandInfo *info;

	bool is_atcommand = true; // false if it's a charcommand

	nullpo_retr(false, sd);

	// Shouldn't happen
	if (message == NULL || *message == '\0')
		return false;

	// Block NOCHAT but do not display it as a normal message
	if (pc_ismuted(&sd->sc, MANNER_NOCOMMAND))
		return true;

	// skip 10/11-langtype's codepage indicator, if detected
	if (message[0] == '|' && strlen(message) >= 4 && (message[3] == atcommand->at_symbol || message[3] == atcommand->char_symbol))
		message += 3;

	// Should display as a normal message
	if (*message != atcommand->at_symbol && *message != atcommand->char_symbol)
		return false;

	if (player_invoked) {
		// Commands are disabled on maps flagged as 'nocommand'
		if (map->list[sd->bl.m].nocommand && pc_get_group_level(sd) < map->list[sd->bl.m].nocommand) {
			clif->message(fd, msg_fd(fd, MSGTBL_COMMANDS_DISABLE_IN_MAP));
			return false;
		}
		if (sd->block_action.commands) // *pcblock script command
			return false;
	}

	if (*message == atcommand->char_symbol)
		is_atcommand = false;

	if (is_atcommand) { // #command
		sprintf(atcmd_msg, "%s", message);
		ssd = sd;
	} else { // @command
		char charname[NAME_LENGTH];
		int n;

		// Checks to see if #command has a name or a name + parameters.
		if ((n = sscanf(message, "%99s \"%23[^\"]\" %99[^\n]", command, charname, params)) < 2
		 && (n = sscanf(message, "%99s %23s %99[^\n]", command, charname, params)) < 2
		) {
			if (pc_get_group_level(sd) == 0) {
				if (n < 1)
					return false; // no command found. Display as normal message

				info = atcommand->get_info_byname(atcommand->check_alias(command + 1));
				if (info == NULL || info->char_groups[pcg->get_idx(sd->group)] == 0) {
					/* if we can't use or doesn't exist: don't even display the command failed message */
					return false;
				}
			}

			sprintf(output, msg_fd(fd, MSGTBL_CHARCOMMAND_FAILED), atcommand->char_symbol); // Charcommand failed (usage: %c<command> <char name> <parameters>).
			clif->message(fd, output);
			return true;
		}

		ssd = map->nick2sd(charname, true);
		if (ssd == NULL) {
			sprintf(output, msg_fd(fd, MSGTBL_CHARCOMMAND_PLAYER_NOT_FOUND), command); // %s failed. Player not found.
			clif->message(fd, output);
			return true;
		}

		if (n > 2)
			sprintf(atcmd_msg, "%s %s", command, params);
		else
			sprintf(atcmd_msg, "%s", command);
	}

	pc->update_idle_time(sd, BCIDLE_ATCOMMAND);

	//Clearing these to be used once more.
	memset(command, '\0', sizeof command);
	memset(params, '\0', sizeof params);

	//check to see if any params exist within this command
	if (sscanf(atcmd_msg, "%99s %99[^\n]", command, params) < 2)
		params[0] = '\0';

	// @commands (script based)
	if (player_invoked && atcommand->binding_count > 0) {
		// Get atcommand binding
		struct atcmd_binding_data *binding = atcommand->get_bind_byname(command);

		// Check if the binding isn't NULL and there is a NPC event, level of usage met, et cetera
		if (binding != NULL && binding->npc_event[0] != '\0'
		 && (
		       (is_atcommand && pc_get_group_level(sd) >= binding->group_lv)
		    || (!is_atcommand && pc_get_group_level(sd) >= binding->group_lv_char)
		    || (is_atcommand && binding->at_groups[pcg->get_idx(sd->group)] > 0)
		    || (!is_atcommand && binding->char_groups[pcg->get_idx(sd->group)] > 0)
		    )
		) {
			if (binding->log) /* log only if this command should be logged [Ind/Hercules] */
				logs->atcommand(sd, atcmd_msg);

			npc->do_atcmd_event(ssd, command, params, binding->npc_event);
			return true;
		}
	}

	//Grab the command information and check for the proper GM level required to use it or if the command exists
	info = atcommand->get_info_byname(atcommand->check_alias(command + 1));
	if (info == NULL) {
		if (pc_get_group_level(sd) == 0) // TODO: remove or replace with proper permission
			return false;

		sprintf(output, msg_fd(fd, MSGTBL_UNKNOWN_COMMAND), command); // "%s is Unknown Command."
		clif->message(fd, output);
		atcommand->get_suggestions(sd, command + 1, is_atcommand);
		return true;
	}

	if (player_invoked) {
		int i;
		if ((is_atcommand && info->at_groups[pcg->get_idx(sd->group)] == 0)
		 || (!is_atcommand && info->char_groups[pcg->get_idx(sd->group)] == 0))
			return false;

		if (pc_isdead(sd) && pc_has_permission(sd,PC_PERM_DISABLE_CMD_DEAD)) {
			clif->message(fd, msg_fd(fd, MSGTBL_IS_ATCOMMAND_DEAD)); // You can't use commands while dead
			return true;
		}
		for (i = 0; i < map->list[sd->bl.m].zone->disabled_commands_count; i++) {
			if (info->func == map->list[sd->bl.m].zone->disabled_commands[i]->cmd) {
				if (pc_get_group_level(sd) < map->list[sd->bl.m].zone->disabled_commands[i]->group_lv) {
					clif->messagecolor_self(sd->fd, COLOR_RED, msg_fd(fd, MSGTBL_COMMAND_DISABLED_IN_AREA)); // "This command is disabled in this area."
					return true;
				}
				break; /* already found the matching command, no need to keep checking -- just go on */
			}
		}
	}

	logCommand = info->log;
	//Attempt to use the command
	if ((info->func(fd, ssd, command, params,info) != true)) {
#ifdef AUTOTRADE_PERSISTENCY
		if (info->func == atcommand_autotrade) /* autotrade deletes caster, so we got nothing more to do here */
			return true;
#endif
		sprintf(output,msg_fd(fd, MSGTBL_COMMAND_FAILED), command); // %s failed.
		clif->message(fd, output);
		return true;
	}

	// info->log cant be used here, because info can be freed [4144]
	if (logCommand) /* log only if this command should be logged [Ind/Hercules] */
		logs->atcommand(sd, is_atcommand ? atcmd_msg : message);

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
static void atcommand_config_read(const char *config_filename)
{
	struct config_t atcommand_config;
	struct config_setting_t *aliases = NULL, *help = NULL, *nolog = NULL;
	const char *symbol = NULL;
	int num_aliases = 0;

	nullpo_retv(config_filename);
	if (!libconfig->load_file(&atcommand_config, config_filename))
		return;

	// Command symbols
	if (libconfig->lookup_string(&atcommand_config, "atcommand_symbol", &symbol)) {
		if (ISPRINT(*symbol) && // no control characters
			*symbol != '/' && // symbol of client commands
			*symbol != '%' && // symbol of party chat
			*symbol != '$' && // symbol of guild chat
			*symbol != atcommand->char_symbol)
			atcommand->at_symbol = *symbol;
	}

	if (libconfig->lookup_string(&atcommand_config, "charcommand_symbol", &symbol)) {
		if (ISPRINT(*symbol) && // no control characters
			*symbol != '/' && // symbol of client commands
			*symbol != '%' && // symbol of party chat
			*symbol != '$' && // symbol of guild chat
			*symbol != atcommand->at_symbol)
			atcommand->char_symbol = *symbol;
	}

	// Command aliases
	aliases = libconfig->lookup(&atcommand_config, "aliases");
	if (aliases != NULL) {
		int i = 0;
		int count = libconfig->setting_length(aliases);

		for (i = 0; i < count; ++i) {
			struct config_setting_t *command;
			const char *commandname = NULL;
			int j = 0, alias_count = 0;
			AtCommandInfo *commandinfo = NULL;

			command = libconfig->setting_get_elem(aliases, i);
			if (config_setting_type(command) != CONFIG_TYPE_ARRAY)
				continue;
			commandname = config_setting_name(command);
			if ( !( commandinfo = atcommand->exists(commandname) ) ) {
				ShowConfigWarning(command, "atcommand_config_read: can not set alias for non-existent command %s", commandname);
				continue;
			}
			alias_count = libconfig->setting_length(command);
			for (j = 0; j < alias_count; ++j) {
				const char *alias = libconfig->setting_get_string_elem(command, j);
				if (alias != NULL) {
					AliasInfo *alias_info;
					if (strdb_exists(atcommand->alias_db, alias)) {
						ShowConfigWarning(command, "atcommand_config_read: alias %s already exists", alias);
						continue;
					}
					CREATE(alias_info, AliasInfo, 1);
					alias_info->command = commandinfo;
					safestrncpy(alias_info->alias, alias, sizeof(alias_info->alias));
					strdb_put(atcommand->alias_db, alias, alias_info);
					++num_aliases;
				}
			}
		}
	}

	nolog = libconfig->lookup(&atcommand_config, "nolog");
	if (nolog != NULL) {
		int i = 0;
		int count = libconfig->setting_length(nolog);

		for (i = 0; i < count; ++i) {
			struct config_setting_t *command;
			const char *commandname = NULL;
			AtCommandInfo *commandinfo = NULL;

			command = libconfig->setting_get_elem(nolog, i);
			commandname = config_setting_name(command);
			if ( !( commandinfo = atcommand->exists(commandname) ) ) {
				ShowConfigWarning(command, "atcommand_config_read: can not disable logging for non-existent command %s", commandname);
				continue;
			}
			commandinfo->log = false;
		}
	}

	// Commands help
	// We only check if all commands exist
	help = libconfig->lookup(&atcommand_config, "help");
	if (help != NULL) {
		int count = libconfig->setting_length(help);
		int i;

		for (i = 0; i < count; ++i) {
			struct config_setting_t *command;
			const char *commandname;
			AtCommandInfo *commandinfo = NULL;

			command = libconfig->setting_get_elem(help, i);
			commandname = config_setting_name(command);
			if ( !( commandinfo = atcommand->exists(commandname) ) )
				ShowConfigWarning(command, "atcommand_config_read: command %s does not exist", commandname);
			else {
				if( commandinfo->help == NULL ) {
					const char *str = libconfig->setting_get_string(command);
					size_t len = strlen(str);
					commandinfo->help = aMalloc(len + 1);
					safestrncpy(commandinfo->help, str, len + 1);
				}
			}
		}
	}

	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' command aliases in '"CL_WHITE"%s"CL_RESET"'.\n", num_aliases, config_filename);

	libconfig->destroy(&atcommand_config);
	return;
}

/**
 * In group configuration file, setting for each command is either
 * <commandname> : <bool> (only atcommand), or
 * <commandname> : [ <bool>, <bool> ] ([ atcommand, charcommand ])
 * Maps AtCommandType enums to indexes of <commandname> value array,
 * COMMAND_ATCOMMAND (1) being index 0, COMMAND_CHARCOMMAND (2) being index 1.
 * @private
 */
static inline int atcommand_command_type2idx(AtCommandType type)
{
	Assert_retr(0, type > 0);
	return (type-1);
}

/**
 * Loads permissions for groups to use commands.
 *
 */
static void atcommand_db_load_groups(GroupSettings **groups, struct config_setting_t **commands_, size_t sz)
{
	struct DBIterator *iter = db_iterator(atcommand->db);
	AtCommandInfo *atcmd;

	nullpo_retv(groups);
	nullpo_retv(commands_);
	for (atcmd = dbi_first(iter); dbi_exists(iter); atcmd = dbi_next(iter)) {
		int i;
		CREATE(atcmd->at_groups, char, sz);
		CREATE(atcmd->char_groups, char, sz);
		for (i = 0; i < sz; i++) {
			GroupSettings *group = groups[i];
			struct config_setting_t *commands = commands_[i];
			int result = 0;
			int idx = -1;

			if (group == NULL) {
				ShowError("atcommand_db_load_groups: group is NULL\n");
				continue;
			}

			idx = pcg->get_idx(group);
			if (idx < 0 || idx >= sz) {
				ShowError("atcommand_db_load_groups: index (%d) out of bounds [0,%"PRIuS"]\n", idx, sz - 1);
				continue;
			}

			if (pcg->has_permission(group, PC_PERM_USE_ALL_COMMANDS)) {
				atcmd->at_groups[idx] = atcmd->char_groups[idx] = 1;
				continue;
			}

			if (commands != NULL) {
				struct config_setting_t *cmd = NULL;

				// <commandname> : <bool> (only atcommand)
				if (config_setting_lookup_bool(commands, atcmd->command, &result) && result) {
					atcmd->at_groups[idx] = 1;
				}
				else
				// <commandname> : [ <bool>, <bool> ] ([ atcommand, charcommand ])
				if ((cmd = config_setting_get_member(commands, atcmd->command)) != NULL &&
				    config_setting_is_aggregate(cmd) &&
				    config_setting_length(cmd) == 2
				) {
					if (config_setting_get_bool_elem(cmd, atcommand_command_type2idx(COMMAND_ATCOMMAND))) {
						atcmd->at_groups[idx] = 1;
					}
					if (config_setting_get_bool_elem(cmd, atcommand_command_type2idx(COMMAND_CHARCOMMAND))) {
						atcmd->char_groups[idx] = 1;
					}
				}
			}
		}
	}
	dbi_destroy(iter);
	return;
}

static bool atcommand_can_use(struct map_session_data *sd, const char *command)
{
	AtCommandInfo *acmd_d;
	struct atcmd_binding_data *bcmd_d;

	nullpo_retr(false, sd);

	if ((acmd_d = atcommand->get_info_byname(atcommand->check_alias(command + 1))) != NULL) {
		return ((*command == atcommand->at_symbol && acmd_d->at_groups[pcg->get_idx(sd->group)] > 0) ||
				(*command == atcommand->char_symbol && acmd_d->char_groups[pcg->get_idx(sd->group)] > 0));
	} else if ((bcmd_d = atcommand->get_bind_byname(atcommand->check_alias(command + 1))) != NULL) {
		return ((*command == atcommand->at_symbol && bcmd_d->at_groups[pcg->get_idx(sd->group)] > 0) ||
				(*command == atcommand->char_symbol && bcmd_d->char_groups[pcg->get_idx(sd->group)] > 0));
	}

	return false;
}

static bool atcommand_can_use2(struct map_session_data *sd, const char *command, AtCommandType type)
{
	AtCommandInfo *acmd_d;
	struct atcmd_binding_data *bcmd_d;

	nullpo_retr(false, sd);

	if ((acmd_d = atcommand->get_info_byname(atcommand->check_alias(command))) != NULL) {
		return ((type == COMMAND_ATCOMMAND && acmd_d->at_groups[pcg->get_idx(sd->group)] > 0) ||
				(type == COMMAND_CHARCOMMAND && acmd_d->char_groups[pcg->get_idx(sd->group)] > 0));
	} else if ((bcmd_d = atcommand->get_bind_byname(atcommand->check_alias(command))) != NULL) {
		return ((type == COMMAND_ATCOMMAND && bcmd_d->at_groups[pcg->get_idx(sd->group)] > 0) ||
				(type == COMMAND_CHARCOMMAND && bcmd_d->char_groups[pcg->get_idx(sd->group)] > 0));
	}

	return false;
}

static bool atcommand_hp_add(char *name, AtCommandFunc func)
{
	/* if commands are added after group permissions are thrown in, they end up with no permissions */
	/* so we restrict commands to be linked in during boot */
	if( core->runflag == MAPSERVER_ST_RUNNING ) {
		ShowDebug("atcommand_hp_add: Commands can't be added after server is ready, skipping '%s'...\n",name);
		return false;
	}

	return HPM_map_add_atcommand(name,func);
}

/**
 * @see DBApply
 */
static int atcommand_db_clear_sub(union DBKey key, struct DBData *data, va_list args)
{
	AtCommandInfo *cmd = DB->data2ptr(data);
	aFree(cmd->at_groups);
	aFree(cmd->char_groups);
	if (cmd->help != NULL)
		aFree(cmd->help);
	return 0;
}

static void atcommand_db_clear(void)
{
	if( atcommand->db != NULL ) {
		atcommand->db->destroy(atcommand->db, atcommand->cmd_db_clear_sub);
		atcommand->db = NULL;
	}
	if( atcommand->alias_db != NULL ) {
		db_destroy(atcommand->alias_db);
		atcommand->alias_db = NULL;
	}
}

static void atcommand_doload(void)
{
	if( core->runflag >= MAPSERVER_ST_RUNNING )
		atcommand->cmd_db_clear();
	if( atcommand->db == NULL )
		atcommand->db = stridb_alloc(DB_OPT_DUP_KEY|DB_OPT_RELEASE_DATA, ATCOMMAND_LENGTH);
	if( atcommand->alias_db == NULL )
		atcommand->alias_db = stridb_alloc(DB_OPT_DUP_KEY|DB_OPT_RELEASE_DATA, ATCOMMAND_LENGTH);
	atcommand->base_commands(); //fills initial atcommand_db with known commands
	atcommand->config_read(map->ATCOMMAND_CONF_FILENAME);
}

static void atcommand_expand_message_table(void)
{
	RECREATE(atcommand->msg_table, char **, ++atcommand->max_message_table);
	CREATE(atcommand->msg_table[atcommand->max_message_table - 1], char *, MSGTBL_MAX);
}

static void do_init_atcommand(bool minimal)
{
	if (minimal)
		return;

	atcommand->at_symbol = '@';
	atcommand->char_symbol = '#';
	atcommand->binding_count = 0;

	atcommand->doload();
}

static void do_final_atcommand(void)
{
	atcommand->cmd_db_clear();
}

void atcommand_defaults(void)
{
	atcommand = &atcommand_s;

	atcommand->atcmd_output = &atcmd_output;
	atcommand->atcmd_player_name = &atcmd_player_name;

	atcommand->db = NULL;
	atcommand->alias_db = NULL;

	atcommand->init = do_init_atcommand;
	atcommand->final = do_final_atcommand;

	atcommand->exec = atcommand_exec;
	atcommand->create = atcommand_hp_add;
	atcommand->can_use = atcommand_can_use;
	atcommand->can_use2 = atcommand_can_use2;
	atcommand->load_groups = atcommand_db_load_groups;
	atcommand->exists = atcommand_exists;
	atcommand->msg_read = msg_config_read;
	atcommand->final_msg = do_final_msg;
	atcommand->get_bind_byname = get_atcommandbind_byname;
	atcommand->get_info_byname = get_atcommandinfo_byname;
	atcommand->check_alias = atcommand_checkalias;
	atcommand->get_suggestions = atcommand_get_suggestions;
	atcommand->config_read = atcommand_config_read;
	atcommand->stopattack = atcommand_stopattack;
	atcommand->pvpoff_sub = atcommand_pvpoff_sub;
	atcommand->pvpon_sub = atcommand_pvpon_sub;
	atcommand->atkillmonster_sub = atkillmonster_sub;
	atcommand->raise_sub = atcommand_raise_sub;
	atcommand->get_jail_time = get_jail_time;
	atcommand->cleanfloor_sub = atcommand_cleanfloor_sub;
	atcommand->mutearea_sub = atcommand_mutearea_sub;
	atcommand->commands_sub = atcommand_commands_sub;
	atcommand->getring = atcommand_getring;
	atcommand->channel_help = atcommand_channel_help;
	atcommand->quest_help = atcommand_quest_help;
	atcommand->cmd_db_clear = atcommand_db_clear;
	atcommand->cmd_db_clear_sub = atcommand_db_clear_sub;
	atcommand->doload = atcommand_doload;
	atcommand->base_commands = atcommand_basecommands;
	atcommand->add = atcommand_add;
	atcommand->msg = atcommand_msg;
	atcommand->expand_message_table = atcommand_expand_message_table;
	atcommand->msgfd = atcommand_msgfd;
	atcommand->msgsd = atcommand_msgsd;
}
