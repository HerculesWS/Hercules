// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#define HERCULES_CORE

#include "../config/core.h" // AUTOLOOTITEM_SIZE, AUTOTRADE_PERSISTENCY, MAX_SUGGESTIONS, MOB_FLEE(), MOB_HIT(), RENEWAL, RENEWAL_DROP, RENEWAL_EXP
#include "atcommand.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "HPMmap.h"
#include "battle.h"
#include "chat.h"
#include "chrif.h"
#include "clif.h"
#include "duel.h"
#include "elemental.h"
#include "guild.h"
#include "homunculus.h"
#include "intif.h"
#include "itemdb.h"
#include "log.h"
#include "mail.h"
#include "map.h"
#include "mapreg.h"
#include "mercenary.h"
#include "mob.h"
#include "npc.h"
#include "party.h"
#include "pc.h"
#include "pc_groups.h" // groupid2name
#include "pet.h"
#include "quest.h"
#include "script.h"
#include "searchstore.h"
#include "skill.h"
#include "status.h"
#include "storage.h"
#include "trade.h"
#include "unit.h"
#include "../common/cbasetypes.h"
#include "../common/conf.h"
#include "../common/core.h"
#include "../common/malloc.h"
#include "../common/mmo.h" // MAX_CARTS
#include "../common/nullpo.h"
#include "../common/random.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/sysinfo.h"
#include "../common/timer.h"
#include "../common/utils.h"

struct atcommand_interface atcommand_s;

static char atcmd_output[CHAT_SIZE_MAX];
static char atcmd_player_name[NAME_LENGTH];

// @commands (script-based)
struct atcmd_binding_data* get_atcommandbind_byname(const char* name) {
	int i = 0;

	if( *name == atcommand->at_symbol || *name == atcommand->char_symbol )
		name++; // for backwards compatibility

	ARR_FIND( 0, atcommand->binding_count, i, strcmpi(atcommand->binding[i]->command, name) == 0 );

	return ( i < atcommand->binding_count ) ? atcommand->binding[i] : NULL;
}

//-----------------------------------------------------------
// Return the message string of the specified number by [Yor]
//-----------------------------------------------------------
const char* atcommand_msg(int msg_number) {
	if (msg_number >= 0 && msg_number < MAX_MSG &&
	    atcommand->msg_table[msg_number] != NULL && atcommand->msg_table[msg_number][0] != '\0')
		return atcommand->msg_table[msg_number];

	return "??";
}

/**
 * Reads Message Data
 *
 * @param[in] cfg_name       configuration filename to read.
 * @param[in] allow_override whether to allow duplicate message IDs to override the original value.
 * @return success state.
 */
bool msg_config_read(const char *cfg_name, bool allow_override) {
	int msg_number;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;
	static int called = 1;

	if ((fp = fopen(cfg_name, "r")) == NULL) {
		ShowError("Messages file not found: %s\n", cfg_name);
		return false;
	}

	if ((--called) == 0)
		memset(atcommand->msg_table, 0, sizeof(atcommand->msg_table[0]) * MAX_MSG);

	while(fgets(line, sizeof(line), fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;
		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) != 2)
			continue;

		if (strcmpi(w1, "import") == 0) {
			msg_config_read(w2, true);
		} else {
			msg_number = atoi(w1);
			if (msg_number >= 0 && msg_number < MAX_MSG) {
				if (atcommand->msg_table[msg_number] != NULL) {
					if (!allow_override) {
						ShowError("Duplicate message: ID '%d' was already used for '%s'. Message '%s' will be ignored.\n",
						          msg_number, w2, atcommand->msg_table[msg_number]);
						continue;
					}
					aFree(atcommand->msg_table[msg_number]);
				}
				/* this could easily become consecutive memory like get_str() and save the malloc overhead for over 1k calls [Ind] */
				atcommand->msg_table[msg_number] = (char *)aMalloc((strlen(w2) + 1)*sizeof (char));
				strcpy(atcommand->msg_table[msg_number],w2);
			}
		}
	}

	fclose(fp);

	return true;
}

/*==========================================
 * Cleanup Message Data
 *------------------------------------------*/
void do_final_msg(void)
{
	int i;
	for (i = 0; i < MAX_MSG; i++)
		aFree(atcommand->msg_table[i]);
}

/**
 * retrieves the help string associated with a given command.
 */
static inline const char* atcommand_help_string(AtCommandInfo *info) {
	return info->help;
}



/*==========================================
 * @send (used for testing packet sends from the client)
 *------------------------------------------*/
ACMD(send)
{
	int len=0,off,end,type;
	long num;
	
	// read message type as hex number (without the 0x)
	if(!message || !*message ||
	   !((sscanf(message, "len %x", &type)==1 && (len=1))
		 || sscanf(message, "%x", &type)==1) ) {
		clif->message(fd, msg_txt(900)); // Usage:
		clif->message(fd, msg_txt(901)); // 	@send len <packet hex number>
		clif->message(fd, msg_txt(902)); // 	@send <packet hex number> {<value>}*
		clif->message(fd, msg_txt(903)); // 	Value: <type=B(default),W,L><number> or S<length>"<string>"
		return false;
	}

#define PARSE_ERROR(error,p) do {\
	clif->message(fd, (error));\
	sprintf(atcmd_output, ">%s", (p));\
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
	if(sscanf((p), "x%lx", &(num)) < 1 && sscanf((p), "%ld ", &(num)) < 1){\
		PARSE_ERROR("Invalid number in:",(p));\
		return false;\
	}\
} while(0) //define GET_VALUE

	if (type > 0 && type < MAX_PACKET_DB) {
		
		if(len)
		{// show packet length
			sprintf(atcmd_output, msg_txt(904), type, packet_db[type].len); // Packet 0x%x length: %d
			clif->message(fd, atcmd_output);
			return true;
		}
		
		len=packet_db[type].len;
		off=2;
		if(len == 0)
		{// unknown packet - ERROR
			sprintf(atcmd_output, msg_txt(905), type); // Unknown packet: 0x%x
			clif->message(fd, atcmd_output);
			return false;
		} else if(len == -1)
		{// dynamic packet
			len=SHRT_MAX-4; // maximum length
			off=4;
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
				++message;
				if(*message == '"'){
					num=0;
				} else {
					GET_VALUE(message,num);
					while(*message != '"')
					{// find start of string
						if(*message == 0 || ISSPACE(*message)){
							PARSE_ERROR(msg_txt(906),message); // Not a string:
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
									PARSE_ERROR(msg_txt(907),message); // Not a hexadecimal digit:
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
				PARSE_ERROR(msg_txt(908),message); // Unknown type of value in:
				return false;
			}
			SKIP_VALUE(message);
		}
		
		if(packet_db[type].len == -1) {// send dynamic packet
			WFIFOW(sd->fd,2)=TOW(off);
			WFIFOSET(sd->fd,off);
		} else {// send static packet
			if(off < len)
				memset(WFIFOP(sd->fd,off),0,len-off);
			WFIFOSET(sd->fd,len);
		}
	} else {
		clif->message(fd, msg_txt(259)); // Invalid packet
		return false;
	}
	sprintf (atcmd_output, msg_txt(258), type, type); // Sent packet 0x%x (%d)
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
ACMD(mapmove) {
	char map_name[MAP_NAME_LENGTH_EXT];
	unsigned short map_index;
	short x = 0, y = 0;
	int16 m = -1;
		
	memset(map_name, '\0', sizeof(map_name));
	
	if (!message || !*message ||
		(sscanf(message, "%15s %hd %hd", map_name, &x, &y) < 3 &&
		 sscanf(message, "%15[^,],%hd,%hd", map_name, &x, &y) < 1)) {
			
			clif->message(fd, msg_txt(909)); // Please enter a map (usage: @warp/@rura/@mapmove <mapname> <x> <y>).
			return false;
		}
	
	map_index = mapindex->name2id(map_name);
	if (map_index)
		m = map->mapindex2mapid(map_index);
	
	if (!map_index || m < 0) { // m < 0 means on different server or that map is disabled! [Kevin]
		clif->message(fd, msg_txt(1)); // Map not found.
		return false;
	}

	if( sd->bl.m == m && sd->bl.x == x && sd->bl.y == y ) {
		clif->message(fd, msg_txt(253)); // You already are at your destination!
		return false;
	}
	
	if ((x || y) && map->getcell(m, x, y, CELL_CHKNOPASS) && pc_get_group_level(sd) < battle_config.gm_ignore_warpable_area) {
		//This is to prevent the pc->setpos call from printing an error.
		clif->message(fd, msg_txt(2));
		if (!map->search_freecell(NULL, m, &x, &y, 10, 10, 1))
			x = y = 0; //Invalid cell, use random spot.
	}
	if (map->list[m].flag.nowarpto && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_txt(247));
		return false;
	}
	if (sd->bl.m >= 0 && map->list[sd->bl.m].flag.nowarp && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_txt(248));
		return false;
	}
	if (pc->setpos(sd, map_index, x, y, CLR_TELEPORT) != 0) {
		clif->message(fd, msg_txt(1)); // Map not found.
		return false;
	}
	
	clif->message(fd, msg_txt(0)); // Warped.
	return true;
}

/*==========================================
 * Displays where a character is. Corrected version by Silent. [Skotlex]
 *------------------------------------------*/
ACMD(where) {
	struct map_session_data* pl_sd;
	
	memset(atcmd_player_name, '\0', sizeof atcmd_player_name);
	
	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif->message(fd, msg_txt(910)); // Please enter a player name (usage: @where <char name>).
		return false;
	}
	
	pl_sd = map->nick2sd(atcmd_player_name);
	if (pl_sd == NULL ||
	    strncmp(pl_sd->status.name, atcmd_player_name, NAME_LENGTH) != 0 ||
	    (pc_has_permission(pl_sd, PC_PERM_HIDE_SESSION) && pc_get_group_level(pl_sd) > pc_get_group_level(sd) && !pc_has_permission(sd, PC_PERM_WHO_DISPLAY_AID))
		) {
		clif->message(fd, msg_txt(3)); // Character not found.
		return false;
	}
	
	snprintf(atcmd_output, sizeof atcmd_output, "%s %s %d %d", pl_sd->status.name, mapindex_id2name(pl_sd->mapindex), pl_sd->bl.x, pl_sd->bl.y);
	clif->message(fd, atcmd_output);
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(jumpto) {
	struct map_session_data *pl_sd = NULL;

	if (!message || !*message) {
		clif->message(fd, msg_txt(911)); // Please enter a player name (usage: @jumpto/@warpto/@goto <char name/ID>).
		return false;
	}

	if (sd->bl.m >= 0 && map->list[sd->bl.m].flag.nowarp && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_txt(248)); // You are not authorized to warp from your current map.
		return false;
	}

	if( pc_isdead(sd) ) {
		clif->message(fd, msg_txt(864)); // "You cannot use this command when dead."
		return false;
	}

	if((pl_sd=map->nick2sd((char *)message)) == NULL && (pl_sd=map->charid2sd(atoi(message))) == NULL) {
		clif->message(fd, msg_txt(3)); // Character not found.
		return false;
	}
	
	if (pl_sd->bl.m >= 0 && map->list[pl_sd->bl.m].flag.nowarpto && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_txt(247)); // You are not authorized to warp to this map.
		return false;
	}

	if( pl_sd->bl.m == sd->bl.m && pl_sd->bl.x == sd->bl.x && pl_sd->bl.y == sd->bl.y ) {
		clif->message(fd, msg_txt(253)); // You already are at your destination!
		return false;
	}

	pc->setpos(sd, pl_sd->mapindex, pl_sd->bl.x, pl_sd->bl.y, CLR_TELEPORT);
	sprintf(atcmd_output, msg_txt(4), pl_sd->status.name); // Jumped to %s
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
	
	sscanf(message, "%hd %hd", &x, &y);
	
	if (map->list[sd->bl.m].flag.noteleport && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_txt(248)); // You are not authorized to warp from your current map.
		return false;
	}
	
	if( pc_isdead(sd) ) {
		clif->message(fd, msg_txt(864)); // "You cannot use this command when dead."
		return false;
	}
	
	if ((x || y) && map->getcell(sd->bl.m, x, y, CELL_CHKNOPASS)) {
		//This is to prevent the pc->setpos call from printing an error.
		clif->message(fd, msg_txt(2));
		if (!map->search_freecell(NULL, sd->bl.m, &x, &y, 10, 10, 1))
			x = y = 0; //Invalid cell, use random spot.
	}

	if( x && y && sd->bl.x == x && sd->bl.y == y ) {
		clif->message(fd, msg_txt(253)); // You already are at your destination!
		return false;
	}
	
	pc->setpos(sd, sd->mapindex, x, y, CLR_TELEPORT);
	sprintf(atcmd_output, msg_txt(5), sd->bl.x, sd->bl.y); // Jumped to %d %d
	clif->message(fd, atcmd_output);
	return true;
}

/*==========================================
 * Display list of online characters with
 * various info.
 *------------------------------------------*/
ACMD(who) {
	struct map_session_data *pl_sd = NULL;
	struct s_mapiterator *iter = NULL;
	char map_name[MAP_NAME_LENGTH_EXT] = "";
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
	for (pl_sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); pl_sd = (TBL_PC*)mapit->next(iter))	{
		if (!((pc_has_permission(pl_sd, PC_PERM_HIDE_SESSION) || (pl_sd->sc.option & OPTION_INVISIBLE)) && pc_get_group_level(pl_sd) > level)) { // you can look only lower or same level
			if (stristr(pl_sd->status.name, player_name) == NULL // search with no case sensitive
				|| (map_id >= 0 && pl_sd->bl.m != map_id))
				continue;
			switch (display_type) {
				case 2: {
					StrBuf->Printf(&buf, msg_txt(343), pl_sd->status.name); // "Name: %s "
					if (pc_get_group_id(pl_sd) > 0) // Player title, if exists
						StrBuf->Printf(&buf, msg_txt(344), pcg->get_name(pl_sd->group)); // "(%s) "
					StrBuf->Printf(&buf, msg_txt(347), pl_sd->status.base_level, pl_sd->status.job_level,
									 pc->job_name(pl_sd->status.class_)); // "| Lv:%d/%d | Job: %s"
					break;
				}
				case 3: {
					if (pc_has_permission(sd, PC_PERM_WHO_DISPLAY_AID))
						StrBuf->Printf(&buf, msg_txt(912), pl_sd->status.char_id, pl_sd->status.account_id);	// "(CID:%d/AID:%d) "
					StrBuf->Printf(&buf, msg_txt(343), pl_sd->status.name); // "Name: %s "
					if (pc_get_group_id(pl_sd) > 0) // Player title, if exists
						StrBuf->Printf(&buf, msg_txt(344), pcg->get_name(pl_sd->group)); // "(%s) "
					StrBuf->Printf(&buf, msg_txt(348), mapindex_id2name(pl_sd->mapindex), pl_sd->bl.x, pl_sd->bl.y); // "| Location: %s %d %d"
					break;
				}
				default: {
					struct party_data *p = party->search(pl_sd->status.party_id);
					struct guild *g = pl_sd->guild;
					
					StrBuf->Printf(&buf, msg_txt(343), pl_sd->status.name); // "Name: %s "
					if (pc_get_group_id(pl_sd) > 0) // Player title, if exists
						StrBuf->Printf(&buf, msg_txt(344), pcg->get_name(pl_sd->group)); // "(%s) "
					if (p != NULL)
						StrBuf->Printf(&buf, msg_txt(345), p->party.name); // " | Party: '%s'"
					if (g != NULL)
						StrBuf->Printf(&buf, msg_txt(346), g->name); // " | Guild: '%s'"
					break;
				}
			}
			clif->message(fd, StrBuf->Value(&buf));
			StrBuf->Clear(&buf);
			count++;
		}
	}
	mapit->free(iter);
	
	if (map_id < 0) {
		if (count == 0)
			StrBuf->Printf(&buf, msg_txt(28)); // No player found.
		else if (count == 1)
			StrBuf->Printf(&buf, msg_txt(29)); // 1 player found.
		else
			StrBuf->Printf(&buf, msg_txt(30), count); // %d players found.
	} else {
		if (count == 0)
			StrBuf->Printf(&buf, msg_txt(54), map->list[map_id].name); // No player found in map '%s'.
		else if (count == 1)
			StrBuf->Printf(&buf, msg_txt(55), map->list[map_id].name); // 1 player found in map '%s'.
		else
			StrBuf->Printf(&buf, msg_txt(56), count, map->list[map_id].name); // %d players found in map '%s'.
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
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;
	int j, count;
	int pl_level, level;
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
	for( pl_sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); pl_sd = (TBL_PC*)mapit->next(iter) )
	{
		pl_level = pc_get_group_level(pl_sd);
		if (!pl_level)
			continue;
		
		if (match_text[0])
		{
			memcpy(player_name, pl_sd->status.name, NAME_LENGTH);
			for (j = 0; player_name[j]; j++)
				player_name[j] = TOLOWER(player_name[j]);
		  	// search with no case sensitive
			if (strstr(player_name, match_text) == NULL)
				continue;
		}
		if (pl_level > level) {
			if (pl_sd->sc.option & OPTION_INVISIBLE)
				continue;
			sprintf(atcmd_output, msg_txt(913), pl_sd->status.name); // Name: %s (GM)
			clif->message(fd, atcmd_output);
			count++;
			continue;
		}
		
		sprintf(atcmd_output, msg_txt(914), // Name: %s (GM:%d) | Location: %s %d %d
				pl_sd->status.name, pl_level,
				mapindex_id2name(pl_sd->mapindex), pl_sd->bl.x, pl_sd->bl.y);
		clif->message(fd, atcmd_output);
		
		sprintf(atcmd_output, msg_txt(915), // BLvl: %d | Job: %s (Lvl: %d)
				pl_sd->status.base_level,
				pc->job_name(pl_sd->status.class_), pl_sd->status.job_level);
		clif->message(fd, atcmd_output);
		
		p = party->search(pl_sd->status.party_id);
		g = pl_sd->guild;
		
		sprintf(atcmd_output,msg_txt(916),	// Party: '%s' | Guild: '%s'
				p?p->party.name:msg_txt(917), g?g->name:msg_txt(917));	// None.
		
		clif->message(fd, atcmd_output);
		count++;
	}
	mapit->free(iter);
	
	if (count == 0)
		clif->message(fd, msg_txt(150)); // No GM found.
	else if (count == 1)
		clif->message(fd, msg_txt(151)); // 1 GM found.
	else {
		sprintf(atcmd_output, msg_txt(152), count); // %d GMs found.
		clif->message(fd, atcmd_output);
	}
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(save) {
	
	pc->setsavepoint(sd, sd->mapindex, sd->bl.x, sd->bl.y);
	if (sd->status.pet_id > 0 && sd->pd)
		intif->save_petdata(sd->status.account_id, &sd->pd->pet);
	
	chrif->save(sd,0);
	
	clif->message(fd, msg_txt(6)); // Your save point has been changed.
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(load) {
	int16 m;
		
	m = map->mapindex2mapid(sd->status.save_point.map);
	if (m >= 0 && map->list[m].flag.nowarpto && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_txt(249)); // You are not authorized to warp to your save map.
		return false;
	}
	if (sd->bl.m >= 0 && map->list[sd->bl.m].flag.nowarp && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_txt(248)); // You are not authorized to warp from your current map.
		return false;
	}
	
	pc->setpos(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, CLR_OUTSIGHT);
	clif->message(fd, msg_txt(7)); // Warping to save point..
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(speed)
{
	int speed;
		
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	
	if (!message || !*message || sscanf(message, "%d", &speed) < 1) {
		sprintf(atcmd_output, msg_txt(918), MIN_WALK_SPEED, MAX_WALK_SPEED); // Please enter a speed value (usage: @speed <%d-%d>).
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
		clif->message(fd, msg_txt(8)); // Speed changed.
	} else
		clif->message(fd, msg_txt(172)); //Speed returned to normal.
	
	status_calc_bl(&sd->bl, SCB_SPEED);
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(storage)
{
	if (sd->npc_id || sd->state.vending || sd->state.buyingstore || sd->state.trading || sd->state.storage_flag)
		return false;
	
	if (storage->open(sd) == 1) { //Already open.
		clif->message(fd, msg_txt(250));
		return false;
	}
	
	clif->message(fd, msg_txt(919)); // Storage opened.
	
	return true;
}


/*==========================================
 *
 *------------------------------------------*/
ACMD(guildstorage)
{
	if (!sd->status.guild_id) {
		clif->message(fd, msg_txt(252));
		return false;
	}
	
	if (sd->npc_id || sd->state.vending || sd->state.buyingstore || sd->state.trading)
		return false;
	
	if (sd->state.storage_flag == 1) {
		clif->message(fd, msg_txt(250));
		return false;
	}
	
	if (sd->state.storage_flag == 2) {
		clif->message(fd, msg_txt(251));
		return false;
	}
	
	if( gstorage->open(sd) ) {
		clif->message(fd, msg_txt(1201)); // Your guild's storage has already been opened by another member, try again later.
		return false;
	}

	clif->message(fd, msg_txt(920)); // Guild storage opened.
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(option)
{
	int param1 = 0, param2 = 0, param3 = 0;
	
	if (!message || !*message || sscanf(message, "%d %d %d", &param1, &param2, &param3) < 1 || param1 < 0 || param2 < 0 || param3 < 0)
	{// failed to match the parameters so inform the user of the options
		const char* text;
		
		// attempt to find the setting information for this command
		text = atcommand_help_string( info );
		
		// notify the user of the requirement to enter an option
		clif->message(fd, msg_txt(921)); // Please enter at least one option.
		
		if( text ) {// send the help text associated with this command
			clif->messageln( fd, text );
		}
		
		return false;
	}
	
	sd->sc.opt1 = param1;
	sd->sc.opt2 = param2;
	pc->setoption(sd, param3);
	
	clif->message(fd, msg_txt(9)); // Options changed.
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(hide) {
	if (sd->sc.option & OPTION_INVISIBLE) {
		sd->sc.option &= ~OPTION_INVISIBLE;
		if (sd->disguise != -1 )
			status->set_viewdata(&sd->bl, sd->disguise);
		else
			status->set_viewdata(&sd->bl, sd->status.class_);
		clif->message(fd, msg_txt(10)); // Invisible: Off
		
		// increment the number of pvp players on the map
		map->list[sd->bl.m].users_pvp++;
		
		if( map->list[sd->bl.m].flag.pvp && !map->list[sd->bl.m].flag.pvp_nocalcrank ) {
			// register the player for ranking calculations
			sd->pvp_timer = timer->add( timer->gettick() + 200, pc->calc_pvprank_timer, sd->bl.id, 0 );
		}
		//bugreport:2266
		map->foreachinmovearea(clif->insight, &sd->bl, AREA_SIZE, sd->bl.x, sd->bl.y, BL_ALL, &sd->bl);
	} else {
		sd->sc.option |= OPTION_INVISIBLE;
		sd->vd.class_ = INVISIBLE_CLASS;
		clif->message(fd, msg_txt(11)); // Invisible: On
		
		// decrement the number of pvp players on the map
		map->list[sd->bl.m].users_pvp--;
		
		if( map->list[sd->bl.m].flag.pvp && !map->list[sd->bl.m].flag.pvp_nocalcrank && sd->pvp_timer != INVALID_TIMER ) {
			// unregister the player for ranking
			timer->delete( sd->pvp_timer, pc->calc_pvprank_timer );
			sd->pvp_timer = INVALID_TIMER;
		}
	}
	clif->changeoption(&sd->bl);
	
	return true;
}

/*==========================================
 * Changes a character's class
 *------------------------------------------*/
ACMD(jobchange) {
	int job = 0, upper = 0;
	const char* text;
	
	if (!message || !*message || sscanf(message, "%d %d", &job, &upper) < 1) {
		upper = 0;
	
	    if( message ) {
			int i;
			bool found = false;

			// Normal Jobs
			for( i = JOB_NOVICE; i < JOB_MAX_BASIC && !found; i++ ) {
				if (strncmpi(message, pc->job_name(i), 16) == 0) {
					job = i;
					found = true;
				}
			}
			
			// High Jobs, Babys and Third
			for( i = JOB_NOVICE_HIGH; i < JOB_MAX && !found; i++ ){
				if (strncmpi(message, pc->job_name(i), 16) == 0) {
					job = i;
					found = true;
				}
			}
			
			if (!found) {
				text = atcommand_help_string(info);
				if (text)
					clif->messageln(fd, text);
				return false;
			}
		}
	}
	/* WHY DO WE LIST THEM THEN? */
	// Deny direct transformation into dummy jobs
    if (job == JOB_KNIGHT2 || job == JOB_CRUSADER2 || job == JOB_WEDDING || job == JOB_XMAS || job == JOB_SUMMER
        || job == JOB_LORD_KNIGHT2 || job == JOB_PALADIN2 || job == JOB_BABY_KNIGHT2 || job == JOB_BABY_CRUSADER2 || job == JOB_STAR_GLADIATOR2
		|| (job >= JOB_RUNE_KNIGHT2 && job <= JOB_MECHANIC_T2) || (job >= JOB_BABY_RUNE2 && job <= JOB_BABY_MECHANIC2)
		) {
		clif->message(fd, msg_txt(923)); //"You can not change to this job by command."
		return true;
	}
	
	if (pcdb_checkid(job)) {
		if (pc->jobchange(sd, job, upper) == 0)
			clif->message(fd, msg_txt(12)); // Your job has been changed.
		else {
			clif->message(fd, msg_txt(155)); // You are unable to change your job.
			return false;
		}
	} else {
		text = atcommand_help_string(info);
        if (text)
            clif->messageln(fd, text);
		return false;
	}
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(kill)
{
	status_kill(&sd->bl);
	clif->message(sd->fd, msg_txt(13)); // A pity! You've died.
	if (fd != sd->fd)
		clif->message(fd, msg_txt(14)); // Character killed.
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(alive)
{
	if (!status->revive(&sd->bl, 100, 100)) {
		clif->message(fd, msg_txt(867)); // "You're not dead."
		return false;
	}
	clif->skill_nodamage(&sd->bl,&sd->bl,ALL_RESURRECTION,4,1);
	clif->message(fd, msg_txt(16)); // You've been revived! It's a miracle!
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
		if (!message || !*message) {
			clif->message(fd, msg_txt(980)); // Please enter a message (usage: @kami <message>).
			return false;
		}
		
		sscanf(message, "%199[^\n]", atcmd_output);
		if (stristr(info->command, "l") != NULL)
			clif->broadcast(&sd->bl, atcmd_output, strlen(atcmd_output) + 1, BC_DEFAULT, ALL_SAMEMAP);
		else
			intif->broadcast(atcmd_output, strlen(atcmd_output) + 1, (*(info->command + 4) == 'b' || *(info->command + 4) == 'B') ? BC_BLUE : BC_YELLOW);
	} else {
		if(!message || !*message || (sscanf(message, "%u %199[^\n]", &color, atcmd_output) < 2)) {
			clif->message(fd, msg_txt(981)); // Please enter color and message (usage: @kamic <color> <message>).
			return false;
		}
		
		if(color > 0xFFFFFF) {
			clif->message(fd, msg_txt(982)); // Invalid color.
			return false;
		}
		intif->broadcast2(atcmd_output, strlen(atcmd_output) + 1, color, 0x190, 12, 0, 0);
	}
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(heal)
{
	int hp = 0, sp = 0; // [Valaris] thanks to fov
	
	sscanf(message, "%d %d", &hp, &sp);
	
	// some overflow checks
	if( hp == INT_MIN ) hp++;
	if( sp == INT_MIN ) sp++;
	
	if ( hp == 0 && sp == 0 ) {
		if (!status_percent_heal(&sd->bl, 100, 100))
			clif->message(fd, msg_txt(157)); // HP and SP have already been recovered.
		else
			clif->message(fd, msg_txt(17)); // HP, SP recovered.
		return true;
	}
	
	if ( hp > 0 && sp >= 0 ) {
		if(!status->heal(&sd->bl, hp, sp, 0))
			clif->message(fd, msg_txt(157)); // HP and SP are already with the good value.
		else
			clif->message(fd, msg_txt(17)); // HP, SP recovered.
		return true;
	}
	
	if ( hp < 0 && sp <= 0 ) {
		status->damage(NULL, &sd->bl, -hp, -sp, 0, 0);
		clif->damage(&sd->bl,&sd->bl, 0, 0, -hp, 0, 4, 0);
		clif->message(fd, msg_txt(156)); // HP or/and SP modified.
		return true;
	}
	
	//Opposing signs.
	if ( hp ) {
		if (hp > 0)
			status->heal(&sd->bl, hp, 0, 0);
		else {
			status->damage(NULL, &sd->bl, -hp, 0, 0, 0);
			clif->damage(&sd->bl,&sd->bl, 0, 0, -hp, 0, 4, 0);
		}
	}
	
	if ( sp ) {
		if (sp > 0)
			status->heal(&sd->bl, 0, sp, 0);
		else
			status->damage(NULL, &sd->bl, 0, -sp, 0, 0);
	}
	
	clif->message(fd, msg_txt(156)); // HP or/and SP modified.
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

	if (!strcmpi(info->command,"itembound") && (!message || !*message || (
		sscanf(message, "\"%99[^\"]\" %d %d", item_name, &number, &bound) < 2 &&
		sscanf(message, "%99s %d %d", item_name, &number, &bound) < 2
	))) {
		clif->message(fd, msg_txt(295)); // Please enter an item name or ID (usage: @itembound <item name/ID> <quantity> <bound_type>).
		return false;
	} else if (!message || !*message || (
		sscanf(message, "\"%99[^\"]\" %d", item_name, &number) < 1 &&
		sscanf(message, "%99s %d", item_name, &number) < 1 ))
		{
 		clif->message(fd, msg_txt(983)); // Please enter an item name or ID (usage: @item <item name/ID> <quantity>).
		return false;
	}
	
	if (number <= 0)
		number = 1;
	
	if ((item_data = itemdb->search_name(item_name)) == NULL &&
	    (item_data = itemdb->exists(atoi(item_name))) == NULL)
	{
		clif->message(fd, msg_txt(19)); // Invalid item ID or name.
		return false;
	}
	
	if(!strcmpi(info->command,"itembound") ) {
		if( !(bound >= IBT_MIN && bound <= IBT_MAX) ) {
			clif->message(fd, msg_txt(298)); // Invalid bound type
			return false;
		}
		switch( (enum e_item_bound_type)bound ) {
			case IBT_CHARACTER:
			case IBT_ACCOUNT:
				break; /* no restrictions */
			case IBT_PARTY:
				if( !sd->status.party_id ) {
					clif->message(fd, msg_txt(1498)); //You can't add a party bound item to a character without party!
					return false;
				}
				break;
			case IBT_GUILD:
				if( !sd->status.guild_id ) {
					clif->message(fd, msg_txt(1499)); //You can't add a guild bound item to a character without guild!
					return false;
				}
				break;
		}
	}

	item_id = item_data->nameid;
	get_count = number;
	//Check if it's stackable.
	if (!itemdb->isstackable2(item_data)) {
		if( bound && (item_data->type == IT_PETEGG || item_data->type == IT_PETARMOR) ) {
			clif->message(fd, msg_txt(498)); // Cannot create bounded pet eggs or pet armors.
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
		clif->message(fd, msg_txt(18)); // Item created.
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
	int identify = 0, refine = 0, attr = 0;
	int c1 = 0, c2 = 0, c3 = 0, c4 = 0;
	
	memset(item_name, '\0', sizeof(item_name));
	
	if (!strcmpi(info->command,"itembound2") && (!message || !*message || (
		sscanf(message, "\"%99[^\"]\" %d %d %d %d %d %d %d %d %d", item_name, &number, &identify, &refine, &attr, &c1, &c2, &c3, &c4, &bound) < 10 &&
		sscanf(message, "%99s %d %d %d %d %d %d %d %d %d", item_name, &number, &identify, &refine, &attr, &c1, &c2, &c3, &c4, &bound) < 10 ))) {
		clif->message(fd, msg_txt(296)); // Please enter all parameters (usage: @itembound2 <item name/ID> <quantity>
		clif->message(fd, msg_txt(297)); //   <identify_flag> <refine> <attribute> <card1> <card2> <card3> <card4> <bound_type>).
		return false;
	} else if ( !message || !*message || (
 								  sscanf(message, "\"%99[^\"]\" %d %d %d %d %d %d %d %d", item_name, &number, &identify, &refine, &attr, &c1, &c2, &c3, &c4) < 9 &&
								  sscanf(message, "%99s %d %d %d %d %d %d %d %d", item_name, &number, &identify, &refine, &attr, &c1, &c2, &c3, &c4) < 9
								  )) {
		clif->message(fd, msg_txt(984)); // Please enter all parameters (usage: @item2 <item name/ID> <quantity>
		clif->message(fd, msg_txt(985)); //   <identify_flag> <refine> <attribute> <card1> <card2> <card3> <card4>).
		return false;
	}
	
	if (number <= 0)
		number = 1;

	if( !strcmpi(info->command,"itembound2") && !(bound >= IBT_MIN && bound <= IBT_MAX) ) {
		clif->message(fd, msg_txt(298)); // Invalid bound type
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
		if( !strcmpi(info->command,"itembound2") )
			bound = 1;
		if( !itemdb->isstackable2(item_data) ) {
			if( bound && (item_data->type == IT_PETEGG || item_data->type == IT_PETARMOR) ) {
				clif->message(fd, msg_txt(498)); // Cannot create bounded pet eggs or pet armors.
				return false;
			}
 			loop = number;
			get_count = 1;
			if (item_data->type == IT_PETEGG) {
				identify = 1;
				refine = 0;
			}
			if (item_data->type == IT_PETARMOR)
				refine = 0;
			if (refine > MAX_REFINE)
				refine = MAX_REFINE;
		} else {
			identify = 1;
			refine = attr = 0;
		}
		for (i = 0; i < loop; i++) {
			memset(&item_tmp, 0, sizeof(item_tmp));
			item_tmp.nameid = item_id;
			item_tmp.identify = identify;
			item_tmp.refine = refine;
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
			clif->message(fd, msg_txt(18)); // Item created.
	} else {
		clif->message(fd, msg_txt(19)); // Invalid item ID or name.
		return false;
	}
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(itemreset)
{
	int i;
	
	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].amount && sd->status.inventory[i].equip == 0) {
			pc->delitem(sd, i, sd->status.inventory[i].amount, 0, 0, LOG_TYPE_COMMAND);
		}
	}
	clif->message(fd, msg_txt(20)); // All of your items have been removed.
	
	return true;
}

/*==========================================
 * Atcommand @lvlup
 *------------------------------------------*/
ACMD(baselevelup)
{
	int level=0, i=0, status_point=0;

	level = atoi(message);
	
	if (!message || !*message || !level) {
		clif->message(fd, msg_txt(986)); // Please enter a level adjustment (usage: @lvup/@blevel/@baselvlup <number of levels>).
		return false;
	}
	
	if (level > 0) {
		if (sd->status.base_level >= pc->maxbaselv(sd)) { // check for max level by Valaris
			clif->message(fd, msg_txt(47)); // Base level can't go any higher.
			return false;
		} // End Addition
		if ((unsigned int)level > pc->maxbaselv(sd) || (unsigned int)level > pc->maxbaselv(sd) - sd->status.base_level) // fix positiv overflow
			level = pc->maxbaselv(sd) - sd->status.base_level;
		for (i = 0; i < level; i++)
			status_point += pc->gets_status_point(sd->status.base_level + i);
		
		sd->status.status_point += status_point;
		sd->status.base_level += (unsigned int)level;
		status_calc_pc(sd, SCO_FORCE);
		status_percent_heal(&sd->bl, 100, 100);
		clif->misceffect(&sd->bl, 0);
		clif->message(fd, msg_txt(21)); // Base level raised.
	} else {
		if (sd->status.base_level == 1) {
			clif->message(fd, msg_txt(158)); // Base level can't go any lower.
			return false;
		}
		level*=-1;
		if ((unsigned int)level >= sd->status.base_level)
			level = sd->status.base_level-1;
		for (i = 0; i > -level; i--)
			status_point += pc->gets_status_point(sd->status.base_level + i - 1);
		if (sd->status.status_point < status_point)
			pc->resetstate(sd);
		if (sd->status.status_point < status_point)
			sd->status.status_point = 0;
		else
			sd->status.status_point -= status_point;
		sd->status.base_level -= (unsigned int)level;
		clif->message(fd, msg_txt(22)); // Base level lowered.
		status_calc_pc(sd, SCO_FORCE);
	}
	sd->status.base_exp = 0;
	clif->updatestatus(sd, SP_STATUSPOINT);
	clif->updatestatus(sd, SP_BASELEVEL);
	clif->updatestatus(sd, SP_BASEEXP);
	clif->updatestatus(sd, SP_NEXTBASEEXP);
	pc->baselevelchanged(sd);
	if(sd->status.party_id)
		party->send_levelup(sd);
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(joblevelup)
{
	int level=0;
	
	level = atoi(message);
	
	if (!message || !*message || !level) {
		clif->message(fd, msg_txt(987)); // Please enter a level adjustment (usage: @joblvup/@jlevel/@joblvlup <number of levels>).
		return false;
	}
	if (level > 0) {
		if (sd->status.job_level >= pc->maxjoblv(sd)) {
			clif->message(fd, msg_txt(23)); // Job level can't go any higher.
			return false;
		}
		if ((unsigned int)level > pc->maxjoblv(sd) || (unsigned int)level > pc->maxjoblv(sd) - sd->status.job_level) // fix positiv overflow
			level = pc->maxjoblv(sd) - sd->status.job_level;
		sd->status.job_level += (unsigned int)level;
		sd->status.skill_point += level;
		clif->misceffect(&sd->bl, 1);
		clif->message(fd, msg_txt(24)); // Job level raised.
	} else {
		if (sd->status.job_level == 1) {
			clif->message(fd, msg_txt(159)); // Job level can't go any lower.
			return false;
		}
		level *=-1;
		if ((unsigned int)level >= sd->status.job_level) // fix negativ overflow
			level = sd->status.job_level-1;
		sd->status.job_level -= (unsigned int)level;
		if (sd->status.skill_point < level)
			pc->resetskill(sd,0);	//Reset skills since we need to substract more points.
		if (sd->status.skill_point < level)
			sd->status.skill_point = 0;
		else
			sd->status.skill_point -= level;
		clif->message(fd, msg_txt(25)); // Job level lowered.
	}
	sd->status.job_exp = 0;
	clif->updatestatus(sd, SP_JOBLEVEL);
	clif->updatestatus(sd, SP_JOBEXP);
	clif->updatestatus(sd, SP_NEXTJOBEXP);
	clif->updatestatus(sd, SP_SKILLPOINT);
	status_calc_pc(sd, SCO_FORCE);
	
	return true;
}

/*==========================================
 * @help
 *------------------------------------------*/
ACMD(help) {
	const char *command_name = NULL;
	char *default_command = "help";
	AtCommandInfo *tinfo = NULL;
			
	if (!message || !*message) {
		command_name = default_command; // If no command_name specified, display help for @help.
	} else {
		if (*message == atcommand->at_symbol || *message == atcommand->char_symbol)
			++message;
		command_name = atcommand->check_alias(message);
	}
	
	if (!atcommand->can_use2(sd, command_name, COMMAND_ATCOMMAND)) {
		sprintf(atcmd_output, msg_txt(153), message); // "%s is Unknown Command"
		clif->message(fd, atcmd_output);
		atcommand->get_suggestions(sd, command_name, true);
		return false;
	}
	
	tinfo = atcommand->get_info_byname(atcommand->check_alias(command_name));
	
	if ( !tinfo || tinfo->help == NULL ) {
		sprintf(atcmd_output, msg_txt(988), atcommand->at_symbol, command_name); // There is no help for %c%s.
		clif->message(fd, atcmd_output);
		atcommand->get_suggestions(sd, command_name, true);
		return false;
	}
	
	sprintf(atcmd_output, msg_txt(989), atcommand->at_symbol, command_name); // Help for command %c%s:
	clif->message(fd, atcmd_output);
	
	{   // Display aliases
		DBIterator* iter;
		AtCommandInfo *command_info;
		AliasInfo *alias_info = NULL;
		StringBuf buf;
		bool has_aliases = false;
		
		StrBuf->Init(&buf);
		StrBuf->AppendStr(&buf, msg_txt(990)); // Available aliases:
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
	clif->message(fd, tinfo->help);
	return true;
}

// helper function, used in foreach calls to stop auto-attack timers
// parameter: '0' - everyone, 'id' - only those attacking someone with that id
int atcommand_stopattack(struct block_list *bl,va_list ap)
{
	struct unit_data *ud = unit->bl2ud(bl);
	int id = va_arg(ap, int);
	if (ud && ud->attacktimer != INVALID_TIMER && (!id || id == ud->target))
	{
		unit->stop_attack(bl);
		return 1;
	}
	return 0;
}
/*==========================================
 *
 *------------------------------------------*/
int atcommand_pvpoff_sub(struct block_list *bl,va_list ap)
{
	TBL_PC* sd = (TBL_PC*)bl;
	clif->pvpset(sd, 0, 0, 2);
	if (sd->pvp_timer != INVALID_TIMER) {
		timer->delete(sd->pvp_timer, pc->calc_pvprank_timer);
		sd->pvp_timer = INVALID_TIMER;
	}
	return 0;
}

ACMD(pvpoff) {
	
	if (!map->list[sd->bl.m].flag.pvp) {
		clif->message(fd, msg_txt(160)); // PvP is already Off.
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
	clif->message(fd, msg_txt(31)); // PvP: Off.
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_pvpon_sub(struct block_list *bl,va_list ap)
{
	TBL_PC* sd = (TBL_PC*)bl;
	if (sd->pvp_timer == INVALID_TIMER) {
		sd->pvp_timer = timer->add(timer->gettick() + 200, pc->calc_pvprank_timer, sd->bl.id, 0);
		sd->pvp_rank = 0;
		sd->pvp_lastusers = 0;
		sd->pvp_point = 5;
		sd->pvp_won = 0;
		sd->pvp_lost = 0;
	}
	return 0;
}

ACMD(pvpon) {
	
	if (map->list[sd->bl.m].flag.pvp) {
		clif->message(fd, msg_txt(161)); // PvP is already On.
		return false;
	}
	
	map->zone_change2(sd->bl.m,strdb_get(map->zone_db, MAP_ZONE_PVP_NAME));
	map->list[sd->bl.m].flag.pvp = 1;
	
	if (!battle_config.pk_mode) {// display pvp circle and rank
		clif->map_property_mapall(sd->bl.m, MAPPROPERTY_FREEPVPZONE);
		clif->maptypeproperty2(&sd->bl,ALL_SAMEMAP);
		map->foreachinmap(atcommand->pvpon_sub,sd->bl.m, BL_PC);
	}
	
	clif->message(fd, msg_txt(32)); // PvP: On.
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(gvgoff) {
	
	if (!map->list[sd->bl.m].flag.gvg) {
		clif->message(fd, msg_txt(162)); // GvG is already Off.
		return false;
	}
	
	map->zone_change2(sd->bl.m,map->list[sd->bl.m].prev_zone);
	map->list[sd->bl.m].flag.gvg = 0;
	clif->map_property_mapall(sd->bl.m, MAPPROPERTY_NOTHING);
	clif->maptypeproperty2(&sd->bl,ALL_SAMEMAP);
	map->foreachinmap(atcommand->stopattack,sd->bl.m, BL_CHAR, 0);
	clif->message(fd, msg_txt(33)); // GvG: Off.
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(gvgon) {
	
	if (map->list[sd->bl.m].flag.gvg) {
		clif->message(fd, msg_txt(163)); // GvG is already On.
		return false;
	}
	
	map->zone_change2(sd->bl.m,strdb_get(map->zone_db, MAP_ZONE_GVG_NAME));
	map->list[sd->bl.m].flag.gvg = 1;
	clif->map_property_mapall(sd->bl.m, MAPPROPERTY_AGITZONE);
	clif->maptypeproperty2(&sd->bl,ALL_SAMEMAP);
	clif->message(fd, msg_txt(34)); // GvG: On.
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(model)
{
	int hair_style = 0, hair_color = 0, cloth_color = 0;
	
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	
	if (!message || !*message || sscanf(message, "%d %d %d", &hair_style, &hair_color, &cloth_color) < 1) {
		sprintf(atcmd_output, msg_txt(991), // Please enter at least one value (usage: @model <hair ID: %d-%d> <hair color: %d-%d> <clothes color: %d-%d>).
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
		clif->message(fd, msg_txt(36)); // Appearence changed.
	} else {
		clif->message(fd, msg_txt(37)); // An invalid number was specified.
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
	
	if (!message || !*message || sscanf(message, "%d", &cloth_color) < 1) {
		sprintf(atcmd_output, msg_txt(992), MIN_CLOTH_COLOR, MAX_CLOTH_COLOR); // Please enter a clothes color (usage: @dye/@ccolor <clothes color: %d-%d>).
		clif->message(fd, atcmd_output);
		return false;
	}
	
	if (cloth_color >= MIN_CLOTH_COLOR && cloth_color <= MAX_CLOTH_COLOR) {
		pc->changelook(sd, LOOK_CLOTHES_COLOR, cloth_color);
		clif->message(fd, msg_txt(36)); // Appearence changed.
	} else {
		clif->message(fd, msg_txt(37)); // An invalid number was specified.
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
	
	if (!message || !*message || sscanf(message, "%d", &hair_style) < 1) {
		sprintf(atcmd_output, msg_txt(993), MIN_HAIR_STYLE, MAX_HAIR_STYLE); // Please enter a hair style (usage: @hairstyle/@hstyle <hair ID: %d-%d>).
		clif->message(fd, atcmd_output);
		return false;
	}
	
	if (hair_style >= MIN_HAIR_STYLE && hair_style <= MAX_HAIR_STYLE) {
		pc->changelook(sd, LOOK_HAIR, hair_style);
		clif->message(fd, msg_txt(36)); // Appearence changed.
	} else {
		clif->message(fd, msg_txt(37)); // An invalid number was specified.
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
	
	if (!message || !*message || sscanf(message, "%d", &hair_color) < 1) {
		sprintf(atcmd_output, msg_txt(994), MIN_HAIR_COLOR, MAX_HAIR_COLOR); // Please enter a hair color (usage: @haircolor/@hcolor <hair color: %d-%d>).
		clif->message(fd, atcmd_output);
		return false;
	}
	
	if (hair_color >= MIN_HAIR_COLOR && hair_color <= MAX_HAIR_COLOR) {
		pc->changelook(sd, LOOK_HAIR_COLOR, hair_color);
		clif->message(fd, msg_txt(36)); // Appearence changed.
	} else {
		clif->message(fd, msg_txt(37)); // An invalid number was specified.
		return false;
	}
	
	return true;
}

/*==========================================
 * @go [city_number or city_name] - Updated by Harbin
 *------------------------------------------*/
ACMD(go)
{
	int i;
	int town;
	char map_name[MAP_NAME_LENGTH];
	int16 m;
	
	const struct {
		char map[MAP_NAME_LENGTH];
		int x, y;
	} data[] = {
		{ MAP_PRONTERA,    156, 191 }, //  0=Prontera
		{ MAP_MORROC,      156,  93 }, //  1=Morroc
		{ MAP_GEFFEN,      119,  59 }, //  2=Geffen
		{ MAP_PAYON,       162, 233 }, //  3=Payon
		{ MAP_ALBERTA,     192, 147 }, //  4=Alberta
#ifdef RENEWAL
		{ MAP_IZLUDE,      128, 146 }, //  5=Izlude (Renewal)
#else
		{ MAP_IZLUDE,      128, 114 }, //  5=Izlude
#endif
		{ MAP_ALDEBARAN,   140, 131 }, //  6=Al de Baran
		{ MAP_LUTIE,       147, 134 }, //  7=Lutie
		{ MAP_COMODO,      209, 143 }, //  8=Comodo
		{ MAP_YUNO,        157,  51 }, //  9=Yuno
		{ MAP_AMATSU,      198,  84 }, // 10=Amatsu
		{ MAP_GONRYUN,     160, 120 }, // 11=Gonryun
		{ MAP_UMBALA,       89, 157 }, // 12=Umbala
		{ MAP_NIFLHEIM,     21, 153 }, // 13=Niflheim
		{ MAP_LOUYANG,     217,  40 }, // 14=Louyang
		{ MAP_NOVICE,       53, 111 }, // 15=Training Grounds
		{ MAP_JAIL,         23,  61 }, // 16=Prison
		{ MAP_JAWAII,      249, 127 }, // 17=Jawaii
		{ MAP_AYOTHAYA,    151, 117 }, // 18=Ayothaya
		{ MAP_EINBROCH,     64, 200 }, // 19=Einbroch
		{ MAP_LIGHTHALZEN, 158,  92 }, // 20=Lighthalzen
		{ MAP_EINBECH,      70,  95 }, // 21=Einbech
		{ MAP_HUGEL,        96, 145 }, // 22=Hugel
		{ MAP_RACHEL,      130, 110 }, // 23=Rachel
		{ MAP_VEINS,       216, 123 }, // 24=Veins
		{ MAP_MOSCOVIA,    223, 184 }, // 25=Moscovia
		{ MAP_MIDCAMP,     180, 240 }, // 26=Midgard Camp
		{ MAP_MANUK,       282, 138 }, // 27=Manuk
		{ MAP_SPLENDIDE,   197, 176 }, // 28=Splendide
		{ MAP_BRASILIS,    182, 239 }, // 29=Brasilis
		{ MAP_DICASTES,    198, 187 }, // 30=El Dicastes
		{ MAP_MORA,         44, 151 }, // 31=Mora
		{ MAP_DEWATA,      200, 180 }, // 32=Dewata
		{ MAP_MALANGDO,    140, 114 }, // 33=Malangdo Island
		{ MAP_MALAYA,      242, 211 }, // 34=Malaya Port
		{ MAP_ECLAGE,      110,  39 }, // 35=Eclage
	};
			
	memset(map_name, '\0', sizeof(map_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	
	// get the number
	town = atoi(message);
	
	if (!message || !*message || sscanf(message, "%11s", map_name) < 1 || town < 0 || town >= ARRAYLENGTH(data))
	{// no value matched so send the list of locations
		const char* text;
		
		// attempt to find the text help string
		text = atcommand_help_string( info );
		
		clif->message(fd, msg_txt(38)); // Invalid location number, or name.
		
		if( text ) {// send the text to the client
			clif->messageln( fd, text );
		}
		
		return false;
	}
	
	// get possible name of the city
	map_name[MAP_NAME_LENGTH-1] = '\0';
	for (i = 0; map_name[i]; i++)
		map_name[i] = TOLOWER(map_name[i]);
	// try to identify the map name
	if (strncmp(map_name, "prontera", 3) == 0) {
		town = 0;
	} else if (strncmp(map_name, "morocc", 4) == 0 ||
	           strncmp(map_name, "morroc", 4) == 0) {
		town = 1;
	} else if (strncmp(map_name, "geffen", 3) == 0) {
		town = 2;
	} else if (strncmp(map_name, "payon", 3) == 0) {
		town = 3;
	} else if (strncmp(map_name, "alberta", 3) == 0) {
		town = 4;
	} else if (strncmp(map_name, "izlude", 3) == 0) {
		town = 5;
	} else if (strncmp(map_name, "aldebaran", 3) == 0) {
		town = 6;
	} else if (strncmp(map_name, "lutie", 3) == 0 ||
	           strcmp(map_name,  "christmas") == 0 ||
	           strncmp(map_name, "xmas", 3) == 0 ||
	           strncmp(map_name, "x-mas", 3) == 0) {
		town = 7;
	} else if (strncmp(map_name, "comodo", 3) == 0) {
		town = 8;
	} else if (strncmp(map_name, "juno", 3) == 0 ||
	           strncmp(map_name, "yuno", 3) == 0) {
		town = 9;
	} else if (strncmp(map_name, "amatsu", 3) == 0) {
		town = 10;
	} else if (strncmp(map_name, "kunlun", 3) == 0 ||
	           strncmp(map_name, "gonryun", 3) == 0) {
		town = 11;
	} else if (strncmp(map_name, "umbala", 3) == 0) {
		town = 12;
	} else if (strncmp(map_name, "niflheim", 3) == 0) {
		town = 13;
	} else if (strncmp(map_name, "louyang", 3) == 0) {
		town = 14;
	} else if (strncmp(map_name, "new_1-1", 3) == 0 ||
	           strncmp(map_name, "startpoint", 3) == 0 ||
	           strncmp(map_name, "beginning", 3) == 0) {
		town = 15;
	} else if (strncmp(map_name, "sec_pri", 3) == 0 ||
	           strncmp(map_name, "prison", 3) == 0 ||
	           strncmp(map_name, "jail", 3) == 0) {
		town = 16;
	} else if (strncmp(map_name, "jawaii", 3) == 0) {
		town = 17;
	} else if (strncmp(map_name, "ayothaya", 3) == 0) {
		town = 18;
	} else if (strncmp(map_name, "einbroch", 5) == 0) {
		town = 19;
	} else if (strncmp(map_name, "lighthalzen", 3) == 0) {
		town = 20;
	} else if (strncmp(map_name, "einbech", 5) == 0) {
		town = 21;
	} else if (strncmp(map_name, "hugel", 3) == 0) {
		town = 22;
	} else if (strncmp(map_name, "rachel", 3) == 0) {
		town = 23;
	} else if (strncmp(map_name, "veins", 3) == 0) {
		town = 24;
	} else if (strncmp(map_name, "moscovia", 3) == 0) {
		town = 25;
	} else if (strncmp(map_name, "mid_camp", 3) == 0) {
		town = 26;
	} else if (strncmp(map_name, "manuk", 3) == 0) {
		town = 27;
	} else if (strncmp(map_name, "splendide", 3) == 0) {
		town = 28;
	} else if (strncmp(map_name, "brasilis", 3) == 0) {
		town = 29;
	} else if (strncmp(map_name, "dicastes01", 3) == 0) {
		town = 30;
	} else if (strcmp(map_name,  "mora") == 0) {
		town = 31;
	} else if (strncmp(map_name, "dewata", 3) == 0) {
		town = 32;
	} else if (strncmp(map_name, "malangdo", 5) == 0) {
		town = 33;
	} else if (strncmp(map_name, "malaya", 5) == 0) {
		town = 34;
	} else if (strncmp(map_name, "eclage", 3) == 0) {
		town = 35;
	}
	
	if (town >= 0 && town < ARRAYLENGTH(data)) {
		m = map->mapname2mapid(data[town].map);
		if (m >= 0 && map->list[m].flag.nowarpto && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
			clif->message(fd, msg_txt(247));
			return false;
		}
		if (sd->bl.m >= 0 && map->list[sd->bl.m].flag.nowarp && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
			clif->message(fd, msg_txt(248));
			return false;
		}
		if (pc->setpos(sd, mapindex->name2id(data[town].map), data[town].x, data[town].y, CLR_TELEPORT) == 0) {
			clif->message(fd, msg_txt(0)); // Warped.
		} else {
			clif->message(fd, msg_txt(1)); // Map not found.
			return false;
		}
	} else { // if you arrive here, you have an error in town variable when reading of names
		clif->message(fd, msg_txt(38)); // Invalid location number or name.
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
	int i, k, range;
	short mx, my;
	unsigned int size;
	
	memset(name, '\0', sizeof(name));
	memset(monster, '\0', sizeof(monster));
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	
	if (!message || !*message) {
		clif->message(fd, msg_txt(80)); // Please specify a display name or monster name/id.
		return false;
	}
	if (sscanf(message, "\"%23[^\"]\" %23s %d", name, monster, &number) > 1 ||
		sscanf(message, "%23s \"%23[^\"]\" %d", monster, name, &number) > 1) {
		//All data can be left as it is.
	} else if ((count=sscanf(message, "%23s %d %23s", monster, &number, name)) > 1) {
		//Here, it is possible name was not given and we are using monster for it.
		if (count < 3) //Blank mob's name.
			name[0] = '\0';
	} else if (sscanf(message, "%23s %23s %d", name, monster, &number) > 1) {
		//All data can be left as it is.
	} else if (sscanf(message, "%23s", monster) > 0) {
		//As before, name may be already filled.
		name[0] = '\0';
	} else {
		clif->message(fd, msg_txt(80)); // Give a display name and monster name/id please.
		return false;
	}
	
	if ((mob_id = mob->db_searchname(monster)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = mob->db_checkid(atoi(monster));
	
	if (mob_id == 0) {
		clif->message(fd, msg_txt(40)); // Invalid monster ID or name.
		return false;
	}
		
	if (number <= 0)
		number = 1;
	
	if( !name[0] )
		strcpy(name, "--ja--");
	
	// If value of atcommand_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
	if (battle_config.atc_spawn_quantity_limit && number > battle_config.atc_spawn_quantity_limit)
		number = battle_config.atc_spawn_quantity_limit;
	
	if (strcmpi(info->command, "monstersmall") == 0)
		size = SZ_SMALL;
	else if (strcmpi(info->command, "monsterbig") == 0)
		size = SZ_BIG;
	else
		size = SZ_MEDIUM;
	
	if (battle_config.etc_log)
		ShowInfo("%s monster='%s' name='%s' id=%d count=%d (%d,%d)\n", command, monster, name, mob_id, number, sd->bl.x, sd->bl.y);
	
	count = 0;
	range = (int)sqrt((float)number) +2; // calculation of an odd number (+ 4 area around)
	for (i = 0; i < number; i++) {
		map->search_freecell(&sd->bl, 0, &mx,  &my, range, range, 0);
		k = mob->once_spawn(sd, sd->bl.m, mx, my, name, mob_id, 1, eventname, size, AI_NONE|(mob_id == MOBID_EMPERIUM?0x200:0x0));
		count += (k != 0) ? 1 : 0;
	}
	
	if (count != 0)
		if (number == count)
			clif->message(fd, msg_txt(39)); // All monster summoned!
		else {
			sprintf(atcmd_output, msg_txt(240), count); // %d monster(s) summoned!
			clif->message(fd, atcmd_output);
		}
		else {
			clif->message(fd, msg_txt(40)); // Invalid monster ID or name.
			return false;
		}
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
int atkillmonster_sub(struct block_list *bl, va_list ap)
{
	struct mob_data *md;
	int flag;
	
	nullpo_ret(md=(struct mob_data *)bl);
	flag = va_arg(ap, int);
	
	if (md->guardian_data)
		return 0; //Do not touch WoE mobs!
	
	if (flag)
		status_zap(bl,md->status.hp, 0);
	else
		status_kill(bl);
	return 1;
}

ACMD(killmonster) {
	int map_id, drop_flag;
	char map_name[MAP_NAME_LENGTH_EXT];
	
	memset(map_name, '\0', sizeof(map_name));
	
	if (!message || !*message || sscanf(message, "%15s", map_name) < 1)
		map_id = sd->bl.m;
	else {
		if ((map_id = map->mapname2mapid(map_name)) < 0)
			map_id = sd->bl.m;
	}
	
	drop_flag = strcmpi(info->command, "killmonster2");
	
	map->foreachinmap(atcommand->atkillmonster_sub, map_id, BL_MOB, -drop_flag);
	
	clif->message(fd, msg_txt(165)); // All monsters killed!
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(refine)
{
	int i,j, position = 0, refine = 0, current_position, final_refine;
	int count;
	
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	
	if (!message || !*message || sscanf(message, "%d %d", &position, &refine) < 2) {
		clif->message(fd, msg_txt(996)); // Please enter a position and an amount (usage: @refine <equip position> <+/- amount>).
		sprintf(atcmd_output, msg_txt(997), EQP_HEAD_LOW); // %d: Lower Headgear
		clif->message(fd, atcmd_output);
		sprintf(atcmd_output, msg_txt(998), EQP_HAND_R); // %d: Right Hand
		clif->message(fd, atcmd_output);
		sprintf(atcmd_output, msg_txt(999), EQP_GARMENT); // %d: Garment
		clif->message(fd, atcmd_output);
		sprintf(atcmd_output, msg_txt(1000), EQP_ACC_L); // %d: Left Accessory
		clif->message(fd, atcmd_output);
		sprintf(atcmd_output, msg_txt(1001), EQP_ARMOR); // %d: Body Armor
		clif->message(fd, atcmd_output);
		sprintf(atcmd_output, msg_txt(1002), EQP_HAND_L); // %d: Left Hand
		clif->message(fd, atcmd_output);
		sprintf(atcmd_output, msg_txt(1003), EQP_SHOES); // %d: Shoes
		clif->message(fd, atcmd_output);
		sprintf(atcmd_output, msg_txt(1004), EQP_ACC_R); // %d: Right Accessory
		clif->message(fd, atcmd_output);
		sprintf(atcmd_output, msg_txt(1005), EQP_HEAD_TOP); // %d: Top Headgear
		clif->message(fd, atcmd_output);
		sprintf(atcmd_output, msg_txt(1006), EQP_HEAD_MID); // %d: Mid Headgear
		clif->message(fd, atcmd_output);
		return false;
	}
	
	refine = cap_value(refine, -MAX_REFINE, MAX_REFINE);
	
	count = 0;
	for (j = 0; j < EQI_MAX; j++) {
		if ((i = sd->equip_index[j]) < 0)
			continue;
		if(j == EQI_AMMO) continue; /* can't equip ammo */
		if(j == EQI_HAND_R && sd->equip_index[EQI_HAND_L] == i)
			continue;
		if(j == EQI_HEAD_MID && sd->equip_index[EQI_HEAD_LOW] == i)
			continue;
		if(j == EQI_HEAD_TOP && (sd->equip_index[EQI_HEAD_MID] == i || sd->equip_index[EQI_HEAD_LOW] == i))
			continue;
		
		if(position && !(sd->status.inventory[i].equip & position))
			continue;
		
		final_refine = cap_value(sd->status.inventory[i].refine + refine, 0, MAX_REFINE);
		if (sd->status.inventory[i].refine != final_refine) {
			sd->status.inventory[i].refine = final_refine;
			current_position = sd->status.inventory[i].equip;
			pc->unequipitem(sd, i, 3);
			clif->refine(fd, 0, i, sd->status.inventory[i].refine);
			clif->delitem(sd, i, 1, 3);
			clif->additem(sd, i, 1, 0);
			pc->equipitem(sd, i, current_position);
			clif->misceffect(&sd->bl, 3);
			count++;
		}
	}
	
	if (count == 0)
		clif->message(fd, msg_txt(166)); // No item has been refined.
	else if (count == 1)
		clif->message(fd, msg_txt(167)); // 1 item has been refined.
	else {
		sprintf(atcmd_output, msg_txt(168), count); // %d items have been refined.
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
	
	if (!message || !*message || (
								  sscanf(message, "\"%99[^\"]\" %d %d", item_name, &attribute, &star) < 1 &&
								  sscanf(message, "%99s %d %d", item_name, &attribute, &star) < 1
								  )) {
		clif->message(fd, msg_txt(1007)); // Please enter at least one item name/ID (usage: @produce <equip name/ID> <element> <# of very's>).
		return false;
	}
	
	if ( (item_data = itemdb->search_name(item_name)) == NULL &&
		(item_data = itemdb->exists(atoi(item_name))) == NULL ) {
		clif->message(fd, msg_txt(170)); //This item is not an equipment.
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
		sprintf(atcmd_output, msg_txt(169), item_id, item_data->name); // The item (%d: '%s') is not equipable.
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
	
	if( !message || !*message || sscanf(message, "%d", &position) < 1 )
	{
		int i;
		clif->message(sd->fd,  msg_txt(868)); // "Your current memo positions are:"
		for( i = 0; i < MAX_MEMOPOINTS; i++ )
		{
			if( sd->status.memo_point[i].map )
				sprintf(atcmd_output, "%d - %s (%d,%d)", i, mapindex_id2name(sd->status.memo_point[i].map), sd->status.memo_point[i].x, sd->status.memo_point[i].y);
			else
				sprintf(atcmd_output, msg_txt(171), i); // %d - void
			clif->message(sd->fd, atcmd_output);
 		}
		return true;
 	}
	
	if( position < 0 || position >= MAX_MEMOPOINTS )
	{
		sprintf(atcmd_output, msg_txt(1008), 0, MAX_MEMOPOINTS-1); // Please enter a valid position (usage: @memo <memo_position:%d-%d>).
		clif->message(fd, atcmd_output);
		return false;
	}
	
	pc->memo(sd, position);
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(gat) {
	int y;
	
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	
	for (y = 2; y >= -2; y--) {
		sprintf(atcmd_output, "%s (x= %d, y= %d) %02X %02X %02X %02X %02X",
				map->list[sd->bl.m].name, sd->bl.x - 2, sd->bl.y + y,
				map->getcell(sd->bl.m,  sd->bl.x - 2, sd->bl.y + y, CELL_GETTYPE),
				map->getcell(sd->bl.m,  sd->bl.x - 1, sd->bl.y + y, CELL_GETTYPE),
				map->getcell(sd->bl.m,  sd->bl.x,     sd->bl.y + y, CELL_GETTYPE),
				map->getcell(sd->bl.m,  sd->bl.x + 1, sd->bl.y + y, CELL_GETTYPE),
				map->getcell(sd->bl.m,  sd->bl.x + 2, sd->bl.y + y, CELL_GETTYPE));
		
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
	
	if (!message || !*message || (i = sscanf(message, "%d %d %d %d %d %d", &type, &flag, &tick, &val1, &val2, &val3)) < 1) {
		clif->message(fd, msg_txt(1009)); // Please enter a status type/flag (usage: @displaystatus <status type> <flag> <tick> {<val1> {<val2> {<val3>}}}).
		return false;
	}
	if (i < 2) flag = 1;
	if (i < 3) tick = 0;
	
	if( flag == 0 )
		clif->sc_end(&sd->bl,sd->bl.id,AREA,type);
	else
		clif->status_change(&sd->bl, type, flag, tick, val1, val2, val3);
	
	return true;
}

/*==========================================
 * @stpoint (Rewritten by [Yor])
 *------------------------------------------*/
ACMD(statuspoint)
{
	int point;
	unsigned int new_status_point;
	
	if (!message || !*message || (point = atoi(message)) == 0) {
		clif->message(fd, msg_txt(1010)); // Please enter a number (usage: @stpoint <number of points>).
		return false;
	}
	
	if(point < 0)
	{
		if(sd->status.status_point < (unsigned int)(-point))
		{
			new_status_point = 0;
		}
		else
		{
			new_status_point = sd->status.status_point + point;
		}
	}
	else if(UINT_MAX - sd->status.status_point < (unsigned int)point)
	{
		new_status_point = UINT_MAX;
	}
	else
	{
		new_status_point = sd->status.status_point + point;
	}
	
	if (new_status_point != sd->status.status_point) {
		sd->status.status_point = new_status_point;
		clif->updatestatus(sd, SP_STATUSPOINT);
		clif->message(fd, msg_txt(174)); // Number of status points changed.
	} else {
		if (point < 0)
			clif->message(fd, msg_txt(41)); // Unable to decrease the number/value.
		else
			clif->message(fd, msg_txt(149)); // Unable to increase the number/value.
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
	unsigned int new_skill_point;
	
	if (!message || !*message || (point = atoi(message)) == 0) {
		clif->message(fd, msg_txt(1011)); // Please enter a number (usage: @skpoint <number of points>).
		return false;
	}
	
	if(point < 0)
	{
		if(sd->status.skill_point < (unsigned int)(-point))
		{
			new_skill_point = 0;
		}
		else
		{
			new_skill_point = sd->status.skill_point + point;
		}
	}
	else if(UINT_MAX - sd->status.skill_point < (unsigned int)point)
	{
		new_skill_point = UINT_MAX;
	}
	else
	{
		new_skill_point = sd->status.skill_point + point;
	}
	
	if (new_skill_point != sd->status.skill_point) {
		sd->status.skill_point = new_skill_point;
		clif->updatestatus(sd, SP_SKILLPOINT);
		clif->message(fd, msg_txt(175)); // Number of skill points changed.
	} else {
		if (point < 0)
			clif->message(fd, msg_txt(41)); // Unable to decrease the number/value.
		else
			clif->message(fd, msg_txt(149)); // Unable to increase the number/value.
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
	
	if (!message || !*message || (zeny = atoi(message)) == 0) {
		clif->message(fd, msg_txt(1012)); // Please enter an amount (usage: @zeny <amount>).
		return false;
	}
	
	if(zeny > 0){
	    if((ret=pc->getzeny(sd,zeny,LOG_TYPE_COMMAND,NULL)) == 1)
			clif->message(fd, msg_txt(149)); // Unable to increase the number/value.
	}
	else {
	    if( sd->status.zeny < -zeny ) zeny = -sd->status.zeny;
	    if((ret=pc->payzeny(sd,-zeny,LOG_TYPE_COMMAND,NULL)) == 1)
			clif->message(fd, msg_txt(41)); // Unable to decrease the number/value.
	}

	if( ret ) //ret != 0 means cmd failure
		return false;

	clif->message(fd, msg_txt(176));
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(param) {
	int i, value = 0, new_value, max;
	const char* param[] = { "str", "agi", "vit", "int", "dex", "luk" };
	short* stats[6];
 	//we don't use direct initialization because it isn't part of the c standard.
	
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	
	if (!message || !*message || sscanf(message, "%d", &value) < 1 || value == 0) {
		clif->message(fd, msg_txt(1013)); // Please enter a valid value (usage: @str/@agi/@vit/@int/@dex/@luk <+/-adjustment>).
		return false;
	}
	
	ARR_FIND( 0, ARRAYLENGTH(param), i, strcmpi(info->command, param[i]) == 0 );
	
	if( i == ARRAYLENGTH(param) || i > MAX_STATUS_TYPE) { // normally impossible...
		clif->message(fd, msg_txt(1013)); // Please enter a valid value (usage: @str/@agi/@vit/@int/@dex/@luk <+/-adjustment>).
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
		max = pc_maxparameter(sd);
	
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
		clif->message(fd, msg_txt(42)); // Stat changed.
	} else {
		if (value < 0)
			clif->message(fd, msg_txt(41)); // Unable to decrease the number/value.
		else
			clif->message(fd, msg_txt(149)); // Unable to increase the number/value.
		return false;
	}
	
	return true;
}

/*==========================================
 * Stat all by fritz (rewritten by [Yor])
 *------------------------------------------*/
ACMD(stat_all) {
	int index, count, value, max, new_value;
	short* stats[6];
 	//we don't use direct initialization because it isn't part of the c standard.
	
	stats[0] = &sd->status.str;
	stats[1] = &sd->status.agi;
	stats[2] = &sd->status.vit;
	stats[3] = &sd->status.int_;
	stats[4] = &sd->status.dex;
	stats[5] = &sd->status.luk;
	
	if (!message || !*message || sscanf(message, "%d", &value) < 1 || value == 0) {
		value = pc_maxparameter(sd);
		max = pc_maxparameter(sd);
	} else {
		if( battle_config.atcommand_max_stat_bypass )
			max = SHRT_MAX;
		else
			max = pc_maxparameter(sd);
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
		clif->message(fd, msg_txt(84)); // All stats changed!
	} else {
		if (value < 0)
			clif->message(fd, msg_txt(177)); // You cannot decrease that stat anymore.
		else
			clif->message(fd, msg_txt(178)); // You cannot increase that stat anymore.
		return false;
	}
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(guildlevelup) {
	int level = 0;
	int16 added_level;
	struct guild *guild_info;
	
	if (!message || !*message || sscanf(message, "%d", &level) < 1 || level == 0) {
		clif->message(fd, msg_txt(1014)); // Please enter a valid level (usage: @guildlvup/@guildlvlup <# of levels>).
		return false;
	}
	
	if (sd->status.guild_id <= 0 || (guild_info = sd->guild) == NULL) {
		clif->message(fd, msg_txt(43)); // You're not in a guild.
		return false;
	}
#if 0 // By enabling this, only the guild leader can use this command
	if (strcmp(sd->status.name, guild_info->master) != 0) {
		clif->message(fd, msg_txt(44)); // You're not the master of your guild.
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
		clif->message(fd, msg_txt(179)); // Guild level changed.
	} else {
		clif->message(fd, msg_txt(45)); // Guild level change failed.
		return false;
	}
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(makeegg)
{
	struct item_data *item_data;
	int id, pet_id;
	
	if (!message || !*message) {
		clif->message(fd, msg_txt(1015)); // Please enter a monster/egg name/ID (usage: @makeegg <pet>).
		return false;
	}
	
	if ((item_data = itemdb->search_name(message)) != NULL) // for egg name
		id = item_data->nameid;
	else
		if ((id = mob->db_searchname(message)) != 0) // for monster name
			;
		else
			id = atoi(message);
	
	pet_id = pet->search_petDB_index(id, PET_CLASS);
	if (pet_id < 0)
		pet_id = pet->search_petDB_index(id, PET_EGG);
	if (pet_id >= 0) {
		sd->catch_target_class = pet->db[pet_id].class_;
		intif->create_pet(
						 sd->status.account_id, sd->status.char_id,
						 (short)pet->db[pet_id].class_, (short)mob->db(pet->db[pet_id].class_)->lv,
						 (short)pet->db[pet_id].EggID, 0, (short)pet->db[pet_id].intimate,
						 100, 0, 1, pet->db[pet_id].jname);
	} else {
		clif->message(fd, msg_txt(180)); // The monster/egg name/id doesn't exist.
		return false;
	}
	
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
		clif->message(fd, msg_txt(181)); // You already have a pet.
		return false;
	}
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(petfriendly)
{
	int friendly;
	struct pet_data *pd;
	
	if (!message || !*message || (friendly = atoi(message)) < 0) {
		clif->message(fd, msg_txt(1016)); // Please enter a valid value (usage: @petfriendly <0-1000>).
		return false;
	}
	
	pd = sd->pd;
	if (!pd) {
		clif->message(fd, msg_txt(184)); // Sorry, but you have no pet.
		return false;
	}
	
	if (friendly < 0 || friendly > 1000)
	{
		clif->message(fd, msg_txt(37)); // An invalid number was specified.
		return false;
	}
	
	if (friendly == pd->pet.intimate) {
		clif->message(fd, msg_txt(183)); // Pet intimacy is already at maximum.
		return false;
	}
	
	pet->set_intimate(pd, friendly);
	clif->send_petstatus(sd);
	clif->message(fd, msg_txt(182)); // Pet intimacy changed.
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(pethungry)
{
	int hungry;
	struct pet_data *pd;
	
	if (!message || !*message || (hungry = atoi(message)) < 0) {
		clif->message(fd, msg_txt(1017)); // Please enter a valid number (usage: @pethungry <0-100>).
		return false;
	}
	
	pd = sd->pd;
	if (!sd->status.pet_id || !pd) {
		clif->message(fd, msg_txt(184)); // Sorry, but you have no pet.
		return false;
	}
	if (hungry < 0 || hungry > 100) {
		clif->message(fd, msg_txt(37)); // An invalid number was specified.
		return false;
	}
	if (hungry == pd->pet.hungry) {
		clif->message(fd, msg_txt(186)); // Pet hunger is already at maximum.
		return false;
	}
	
	pd->pet.hungry = hungry;
	clif->send_petstatus(sd);
	clif->message(fd, msg_txt(185)); // Pet hunger changed.
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(petrename)
{
	struct pet_data *pd;
	if (!sd->status.pet_id || !sd->pd) {
		clif->message(fd, msg_txt(184)); // Sorry, but you have no pet.
		return false;
	}
	pd = sd->pd;
	if (!pd->pet.rename_flag) {
		clif->message(fd, msg_txt(188)); // You can already rename your pet.
		return false;
	}
	
	pd->pet.rename_flag = 0;
	intif->save_petdata(sd->status.account_id, &pd->pet);
	clif->send_petstatus(sd);
	clif->message(fd, msg_txt(187)); // You can now rename your pet.
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(recall) {
	struct map_session_data *pl_sd = NULL;
	
	
	if (!message || !*message) {
		clif->message(fd, msg_txt(1018)); // Please enter a player name (usage: @recall <char name/ID>).
		return false;
	}
	
	if((pl_sd=map->nick2sd((char *)message)) == NULL && (pl_sd=map->charid2sd(atoi(message))) == NULL) {
		clif->message(fd, msg_txt(3)); // Character not found.
		return false;
	}
	
	if ( pc_get_group_level(sd) < pc_get_group_level(pl_sd) )
	{
		clif->message(fd, msg_txt(81)); // Your GM level doesn't authorize you to preform this action on the specified player.
		return false;
	}
	
	if (sd->bl.m >= 0 && map->list[sd->bl.m].flag.nowarpto && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_txt(1019)); // You are not authorized to warp someone to this map.
		return false;
	}
	if (pl_sd->bl.m >= 0 && map->list[pl_sd->bl.m].flag.nowarp && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_txt(1020)); // You are not authorized to warp this player from their map.
		return false;
	}
	if (pl_sd->bl.m == sd->bl.m && pl_sd->bl.x == sd->bl.x && pl_sd->bl.y == sd->bl.y) {
		return false;
	}
	pc->setpos(pl_sd, sd->mapindex, sd->bl.x, sd->bl.y, CLR_RESPAWN);
	sprintf(atcmd_output, msg_txt(46), pl_sd->status.name); // %s recalled!
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
	
	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif->message(fd, msg_txt(1021)); // Please enter a player name (usage: @block <char name>).
		return false;
	}
	
	chrif->char_ask_name(sd->status.account_id, atcmd_player_name, 1, 0, 0, 0, 0, 0, 0); // type: 1 - block
	clif->message(fd, msg_txt(88)); // Character name sent to char-server to ask it.
	
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
 *           this example adds 1 month and 1 second, and substracts 2 minutes and 6 years at the same time.
 *------------------------------------------*/
ACMD(char_ban)
{
	char * modif_p;
	int year, month, day, hour, minute, second, value;
	time_t timestamp;
	struct tm *tmtime;
	
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));
	
	if (!message || !*message || sscanf(message, "%255s %23[^\n]", atcmd_output, atcmd_player_name) < 2) {
		clif->message(fd, msg_txt(1022)); // Please enter ban time and a player name (usage: @ban <time> <char name>).
		return false;
	}
	
	atcmd_output[sizeof(atcmd_output)-1] = '\0';
	
	modif_p = atcmd_output;
	year = month = day = hour = minute = second = 0;
	while (modif_p[0] != '\0') {
		value = atoi(modif_p);
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
		clif->message(fd, msg_txt(85)); // Invalid time for ban command.
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
		clif->message(fd,msg_txt(1023)); // You are not allowed to reduce the length of a ban.
		return false;
	}
	
	chrif->char_ask_name(sd->status.account_id, atcmd_player_name, !strcmpi(info->command,"charban") ? 6 : 2, year, month, day, hour, minute, second); // type: 2 - ban; 6 - charban
	clif->message(fd, msg_txt(88)); // Character name sent to char-server to ask it.
	
	return true;
}

/*==========================================
 * charunblock command (usage: charunblock <player_name>)
 *------------------------------------------*/
ACMD(char_unblock)
{
	
	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));
	
	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif->message(fd, msg_txt(1024)); // Please enter a player name (usage: @unblock <char name>).
		return false;
	}
	
	// send answer to login server via char-server
	chrif->char_ask_name(sd->status.account_id, atcmd_player_name, 3, 0, 0, 0, 0, 0, 0); // type: 3 - unblock
	clif->message(fd, msg_txt(88)); // Character name sent to char-server to ask it.
	
	return true;
}

/*==========================================
 * charunban command (usage: charunban <player_name>)
 *------------------------------------------*/
ACMD(char_unban)
{
	
	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));
	
	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif->message(fd, msg_txt(1025)); // Please enter a player name (usage: @unban <char name>).
		return false;
	}
	
	// send answer to login server via char-server
	chrif->char_ask_name(sd->status.account_id, atcmd_player_name, !strcmpi(info->command,"charunban") ? 7 : 4, 0, 0, 0, 0, 0, 0); // type: 4 - unban account; type 7 - unban character
	clif->message(fd, msg_txt(88)); // Character name sent to char-server to ask it.
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(night) {
	
	if (map->night_flag != 1) {
		pc->map_night_timer(pc->night_timer_tid, 0, 0, 1);
	} else {
		clif->message(fd, msg_txt(89)); // Night mode is already enabled.
		return false;
	}
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(day) {
	
	if (map->night_flag != 0) {
		pc->map_day_timer(pc->day_timer_tid, 0, 0, 1);
	} else {
		clif->message(fd, msg_txt(90)); // Day mode is already enabled.
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
	for( pl_sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); pl_sd = (TBL_PC*)mapit->next(iter) )
	{
		if (pl_sd->fd != fd && pc_get_group_level(sd) >= pc_get_group_level(pl_sd))
		{
			status_kill(&pl_sd->bl);
			clif->specialeffect(&pl_sd->bl,450,AREA);
			clif->message(pl_sd->fd, msg_txt(61)); // The holy messenger has given judgement.
		}
	}
	mapit->free(iter);
	
	clif->message(fd, msg_txt(62)); // Judgement was made.
	
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
	for( pl_sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); pl_sd = (TBL_PC*)mapit->next(iter) )
	{
		if (pl_sd->fd != fd && sd->bl.m == pl_sd->bl.m && pc_get_group_level(sd) >= pc_get_group_level(pl_sd))
		{
			status_kill(&pl_sd->bl);
			clif->specialeffect(&pl_sd->bl,450,AREA);
			clif->message(pl_sd->fd, msg_txt(61)); // The holy messenger has given judgement.
		}
	}
	mapit->free(iter);
	
	clif->message(fd, msg_txt(62)); // Judgement was made.
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
void atcommand_raise_sub(struct map_session_data* sd) {
	
	status->revive(&sd->bl, 100, 100);
	
	clif->skill_nodamage(&sd->bl,&sd->bl,ALL_RESURRECTION,4,1);
	clif->message(sd->fd, msg_txt(63)); // Mercy has been shown.
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(raise)
{
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;
		
	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); pl_sd = (TBL_PC*)mapit->next(iter) )
		if( pc_isdead(pl_sd) )
			atcommand->raise_sub(pl_sd);
	mapit->free(iter);
	
	clif->message(fd, msg_txt(64)); // Mercy has been granted.
	
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
	for( pl_sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); pl_sd = (TBL_PC*)mapit->next(iter) )
		if (sd->bl.m == pl_sd->bl.m && pc_isdead(pl_sd) )
			atcommand->raise_sub(pl_sd);
	mapit->free(iter);
	
	clif->message(fd, msg_txt(64)); // Mercy has been granted.
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(kick)
{
	struct map_session_data *pl_sd;
	
	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));
	
	if (!message || !*message) {
		clif->message(fd, msg_txt(1026)); // Please enter a player name (usage: @kick <char name/ID>).
		return false;
	}
	
	if((pl_sd=map->nick2sd((char *)message)) == NULL && (pl_sd=map->charid2sd(atoi(message))) == NULL) {
		clif->message(fd, msg_txt(3)); // Character not found.
		return false;
	}
	
	if ( pc_get_group_level(sd) < pc_get_group_level(pl_sd) )
	{
		clif->message(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
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
	for( pl_sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); pl_sd = (TBL_PC*)mapit->next(iter) )
	{
		if (pc_get_group_level(sd) >= pc_get_group_level(pl_sd)) { // you can kick only lower or same gm level
			if (sd->status.account_id != pl_sd->status.account_id)
				clif->GM_kick(NULL, pl_sd);
		}
	}
	mapit->free(iter);
	
	clif->message(fd, msg_txt(195)); // All players have been kicked!
	
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
	clif->message(fd, msg_txt(76)); // All skills have been added to your skill tree.
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(questskill)
{
	uint16 skill_id, index;
	
	if (!message || !*message || (skill_id = atoi(message)) <= 0)
	{// also send a list of skills applicable to this command
		const char* text;
		
		// attempt to find the text corresponding to this command
		text = atcommand_help_string( info );
		
		// send the error message as always
		clif->message(fd, msg_txt(1027)); // Please enter a quest skill number.
		
		if( text ) {// send the skill ID list associated with this command
			clif->messageln( fd, text );
		}
		
		return false;
	}
	if( !(index = skill->get_index(skill_id)) ) {
		clif->message(fd, msg_txt(198)); // This skill number doesn't exist.
		return false;
	}
	if (!(skill->get_inf2(skill_id) & INF2_QUEST_SKILL)) {
		clif->message(fd, msg_txt(197)); // This skill number doesn't exist or isn't a quest skill.
		return false;
	}
	if (pc->checkskill2(sd, index) > 0) {
		clif->message(fd, msg_txt(196)); // You already have this quest skill.
		return false;
	}
	
	pc->skill(sd, skill_id, 1, 0);
	clif->message(fd, msg_txt(70)); // You have learned the skill.
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(lostskill)
{
	uint16 skill_id, index;
	
	if (!message || !*message || (skill_id = atoi(message)) <= 0)
	{// also send a list of skills applicable to this command
		const char* text;
		
		// attempt to find the text corresponding to this command
		text = atcommand_help_string( info );
		
		// send the error message as always
		clif->message(fd, msg_txt(1027)); // Please enter a quest skill number.
		
		if( text ) {// send the skill ID list associated with this command
			clif->messageln( fd, text );
		}
		
		return false;
	}
	if ( !( index = skill->get_index(skill_id) ) ) {
		clif->message(fd, msg_txt(198)); // This skill number doesn't exist.
		return false;
	}
	if (!(skill->get_inf2(skill_id) & INF2_QUEST_SKILL)) {
		clif->message(fd, msg_txt(197)); // This skill number doesn't exist or isn't a quest skill.
		return false;
	}
	if (pc->checkskill2(sd, index) == 0) {
		clif->message(fd, msg_txt(201)); // You don't have this quest skill.
		return false;
	}
	
	sd->status.skill[index].lv = 0;
	sd->status.skill[index].flag = 0;
	clif->deleteskill(sd,skill_id);
	clif->message(fd, msg_txt(71)); // You have forgotten the skill.
	
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
	
	if( !message || !*message || (number = atoi(message)) < 0 || number > max_spiritballs )
	{
		char msg[CHAT_SIZE_MAX];
		safesnprintf(msg, sizeof(msg), msg_txt(1028), max_spiritballs); // Please enter an amount (usage: @spiritball <number: 0-%d>).
		clif->message(fd, msg);
		return false;
	}
	
	if( sd->spiritball > 0 )
		pc->delspiritball(sd, sd->spiritball, 1);
	sd->spiritball = number;
	clif->spiritball(&sd->bl);
	// no message, player can look the difference
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(party)
{
	char party_name[NAME_LENGTH];
	
	memset(party_name, '\0', sizeof(party_name));
	
	if (!message || !*message || sscanf(message, "%23[^\n]", party_name) < 1) {
		clif->message(fd, msg_txt(1029)); // Please enter a party name (usage: @party <party_name>).
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
	
	if (!message || !*message || sscanf(message, "%23[^\n]", guild_name) < 1) {
		clif->message(fd, msg_txt(1030)); // Please enter a guild name (usage: @guild <guild_name>).
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
		struct guild *g;
		g = sd->guild; // Search the guild
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
				clif->message(fd, msg_txt(1181)); // You need to be a Guild Master to use this command.
				return false;
			}
		} else { // Guild was not found. HOW?
			clif->message(fd, msg_txt(252)); // You are not in a guild.
			return false;
		}
	} else { // Player does not have a guild
		clif->message(fd, msg_txt(252)); // You are not in a guild.
		return false;
	}
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(agitstart) {
	if (map->agit_flag == 1) {
		clif->message(fd, msg_txt(73)); // War of Emperium is currently in progress.
		return false;
	}
	
	map->agit_flag = 1;
	guild->agit_start();
	clif->message(fd, msg_txt(72)); // War of Emperium has been initiated.
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(agitstart2) {
	if (map->agit2_flag == 1) {
		clif->message(fd, msg_txt(404)); // "War of Emperium SE is currently in progress."
		return false;
	}
	
	map->agit2_flag = 1;
	guild->agit2_start();
	clif->message(fd, msg_txt(403)); // "War of Emperium SE has been initiated."
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(agitend) {
	if (map->agit_flag == 0) {
		clif->message(fd, msg_txt(75)); // War of Emperium is currently not in progress.
		return false;
	}
	
	map->agit_flag = 0;
	guild->agit_end();
	clif->message(fd, msg_txt(74)); // War of Emperium has been ended.
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(agitend2) {
	if (map->agit2_flag == 0) {
		clif->message(fd, msg_txt(406)); // "War of Emperium SE is currently not in progress."
		return false;
	}
	
	map->agit2_flag = 0;
	guild->agit2_end();
	clif->message(fd, msg_txt(405)); // "War of Emperium SE has been ended."
	
	return true;
}

/*==========================================
 * @mapexit - shuts down the map server
 *------------------------------------------*/
ACMD(mapexit) {
	map->do_shutdown();
	return true;
}

/*==========================================
 * idsearch <part_of_name>: revrited by [Yor]
 *------------------------------------------*/
ACMD(idsearch)
{
	char item_name[100];
	unsigned int i, match;
	struct item_data *item_array[MAX_SEARCH];
	
	memset(item_name, '\0', sizeof(item_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	
	if (!message || !*message || sscanf(message, "%99s", item_name) < 0) {
		clif->message(fd, msg_txt(1031)); // Please enter part of an item name (usage: @idsearch <part_of_item_name>).
		return false;
	}
	
	sprintf(atcmd_output, msg_txt(77), item_name); // Search results for '%s' (name: id):
	clif->message(fd, atcmd_output);
	match = itemdb->search_name_array(item_array, MAX_SEARCH, item_name, 0);
	if (match > MAX_SEARCH) {
		sprintf(atcmd_output, msg_txt(269), MAX_SEARCH, match);
		clif->message(fd, atcmd_output);
		match = MAX_SEARCH;
	}
	for(i = 0; i < match; i++) {
		sprintf(atcmd_output, msg_txt(78), item_array[i]->jname, item_array[i]->nameid); // %s: %d
		clif->message(fd, atcmd_output);
	}
	sprintf(atcmd_output, msg_txt(79), match); // %d results found.
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
		clif->message(fd, msg_txt(1032)); // You are not authorized to warp somenone to your current map.
		return false;
	}
	
	count = 0;
	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); pl_sd = (TBL_PC*)mapit->next(iter) ) {
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
	
	clif->message(fd, msg_txt(92)); // All characters recalled!
	if (count) {
		sprintf(atcmd_output, msg_txt(1033), count); // Because you are not authorized to warp from some maps, %d player(s) have not been recalled.
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
	
	if (!message || !*message || sscanf(message, "%23[^\n]", guild_name) < 1) {
		clif->message(fd, msg_txt(1034)); // Please enter a guild name/ID (usage: @guildrecall <guild_name/ID>).
		return false;
	}
	
	if (sd->bl.m >= 0 && map->list[sd->bl.m].flag.nowarpto && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_txt(1032)); // You are not authorized to warp somenone to your current map.
		return false;
	}
	
	if ((g = guild->searchname(guild_name)) == NULL && // name first to avoid error when name begin with a number
	    (g = guild->search(atoi(message))) == NULL)
	{
		clif->message(fd, msg_txt(94)); // Incorrect name/ID, or no one from the guild is online.
		return false;
	}
	
	count = 0;
	
	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); pl_sd = (TBL_PC*)mapit->next(iter) )
	{
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
	
	sprintf(atcmd_output, msg_txt(93), g->name); // All online characters of the %s guild have been recalled to your position.
	clif->message(fd, atcmd_output);
	if (count) {
		sprintf(atcmd_output, msg_txt(1033), count); // Because you are not authorized to warp from some maps, %d player(s) have not been recalled.
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
	
	if (!message || !*message || sscanf(message, "%23[^\n]", party_name) < 1) {
		clif->message(fd, msg_txt(1035)); // Please enter a party name/ID (usage: @partyrecall <party_name/ID>).
		return false;
	}
	
	if (sd->bl.m >= 0 && map->list[sd->bl.m].flag.nowarpto && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE)) {
		clif->message(fd, msg_txt(1032)); // You are not authorized to warp somenone to your current map.
		return false;
	}
	
	if ((p = party->searchname(party_name)) == NULL && // name first to avoid error when name begin with a number
	    (p = party->search(atoi(message))) == NULL)
	{
		clif->message(fd, msg_txt(96)); // Incorrect name or ID, or no one from the party is online.
		return false;
	}
	
	count = 0;
	
	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); pl_sd = (TBL_PC*)mapit->next(iter) ) {
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
	
	sprintf(atcmd_output, msg_txt(95), p->party.name); // All online characters of the %s party have been recalled to your position.
	clif->message(fd, atcmd_output);
	if (count) {
		sprintf(atcmd_output, msg_txt(1033), count); // Because you are not authorized to warp from some maps, %d player(s) have not been recalled.
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
	clif->message(fd, msg_txt(97)); // Item database has been reloaded.
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(reloadmobdb) {
	mob->reload();
	pet->read_db();
	homun->reload();
	mercenary->read_db();
	mercenary->read_skilldb();
	elemental->reload_db();
	clif->message(fd, msg_txt(98)); // Monster database has been reloaded.
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(reloadskilldb)
{
	skill->reload();
	homun->reload_skill();
	elemental->reload_skilldb();
	mercenary->read_skilldb();
	clif->message(fd, msg_txt(99)); // Skill database has been reloaded.
	
	return true;
}

/*==========================================
 * @reloadatcommand - reloads conf/atcommand.conf conf/groups.conf
 *------------------------------------------*/
ACMD(reloadatcommand) {
	config_t run_test;
	
	if (libconfig->read_file(&run_test, "conf/groups.conf")) {
		clif->message(fd, msg_txt(1036)); // Error reading groups.conf, reload failed.
		return false;
	}
	
	libconfig->destroy(&run_test);
	
	if (libconfig->read_file(&run_test, map->ATCOMMAND_CONF_FILENAME)) {
		clif->message(fd, msg_txt(1037)); // Error reading atcommand.conf, reload failed.
		return false;
	}
	
	libconfig->destroy(&run_test);
	
	atcommand->doload();
	pcg->reload();
	clif->message(fd, msg_txt(254));
	return true;
}
/*==========================================
 * @reloadbattleconf - reloads /conf/battle.conf
 *------------------------------------------*/
ACMD(reloadbattleconf)
{
	struct Battle_Config prev_config;
	memcpy(&prev_config, &battle_config, sizeof(prev_config));
	
	battle->config_read(map->BATTLE_CONF_FILENAME);
	
	if( prev_config.item_rate_mvp          != battle_config.item_rate_mvp
	   ||  prev_config.item_rate_common       != battle_config.item_rate_common
	   ||  prev_config.item_rate_common_boss  != battle_config.item_rate_common_boss
	   ||  prev_config.item_rate_card         != battle_config.item_rate_card
	   ||  prev_config.item_rate_card_boss    != battle_config.item_rate_card_boss
	   ||  prev_config.item_rate_equip        != battle_config.item_rate_equip
	   ||  prev_config.item_rate_equip_boss   != battle_config.item_rate_equip_boss
	   ||  prev_config.item_rate_heal         != battle_config.item_rate_heal
	   ||  prev_config.item_rate_heal_boss    != battle_config.item_rate_heal_boss
	   ||  prev_config.item_rate_use          != battle_config.item_rate_use
	   ||  prev_config.item_rate_use_boss     != battle_config.item_rate_use_boss
	   ||  prev_config.item_rate_treasure     != battle_config.item_rate_treasure
	   ||  prev_config.item_rate_adddrop      != battle_config.item_rate_adddrop
	   ||  prev_config.logarithmic_drops      != battle_config.logarithmic_drops
	   ||  prev_config.item_drop_common_min   != battle_config.item_drop_common_min
	   ||  prev_config.item_drop_common_max   != battle_config.item_drop_common_max
	   ||  prev_config.item_drop_card_min     != battle_config.item_drop_card_min
	   ||  prev_config.item_drop_card_max     != battle_config.item_drop_card_max
	   ||  prev_config.item_drop_equip_min    != battle_config.item_drop_equip_min
	   ||  prev_config.item_drop_equip_max    != battle_config.item_drop_equip_max
	   ||  prev_config.item_drop_mvp_min      != battle_config.item_drop_mvp_min
	   ||  prev_config.item_drop_mvp_max      != battle_config.item_drop_mvp_max
	   ||  prev_config.item_drop_heal_min     != battle_config.item_drop_heal_min
	   ||  prev_config.item_drop_heal_max     != battle_config.item_drop_heal_max
	   ||  prev_config.item_drop_use_min      != battle_config.item_drop_use_min
	   ||  prev_config.item_drop_use_max      != battle_config.item_drop_use_max
	   ||  prev_config.item_drop_treasure_min != battle_config.item_drop_treasure_min
	   ||  prev_config.item_drop_treasure_max != battle_config.item_drop_treasure_max
	   ||  prev_config.base_exp_rate          != battle_config.base_exp_rate
	   ||  prev_config.job_exp_rate           != battle_config.job_exp_rate
	   )
  	{	// Exp or Drop rates changed.
		mob->reload(); //Needed as well so rate changes take effect.
		chrif->ragsrvinfo(battle_config.base_exp_rate, battle_config.job_exp_rate, battle_config.item_rate_common);
	}
	clif->message(fd, msg_txt(255));
	return true;
}
/*==========================================
 * @reloadstatusdb - reloads job_db1.txt job_db2.txt job_db2-2.txt refine_db.txt size_fix.txt
 *------------------------------------------*/
ACMD(reloadstatusdb) {
	status->readdb();
	clif->message(fd, msg_txt(256));
	return true;
}
/*==========================================
 * @reloadpcdb - reloads exp.txt skill_tree.txt attr_fix.txt statpoint.txt
 *------------------------------------------*/
ACMD(reloadpcdb)
{
	pc->readdb();
	clif->message(fd, msg_txt(257));
	return true;
}

/*==========================================
 * @reloadscript - reloads all scripts (npcs, warps, mob spawns, ...)
 *------------------------------------------*/
ACMD(reloadscript) {
	struct s_mapiterator* iter;
	struct map_session_data* pl_sd;

	//atcommand_broadcast( fd, sd, "@broadcast", "Server is reloading scripts..." );
	//atcommand_broadcast( fd, sd, "@broadcast", "You will feel a bit of lag at this point !" );
	
	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); pl_sd = (TBL_PC*)mapit->next(iter) ) {
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
	
	flush_fifos();
	map->reloadnpc(true, NULL, 0); // reload config files seeking for npcs
	script->reload();
	npc->reload();
	
	clif->message(fd, msg_txt(100)); // Scripts have been reloaded.
	
	return true;
}

/*==========================================
 * @mapinfo [0-3] <map name> by MC_Cameri
 * => Shows information about the map [map name]
 * 0 = no additional information
 * 1 = Show users in that map and their location
 * 2 = Shows NPCs in that map
 * 3 = Shows the chats in that map
 TODO# add the missing mapflags e.g. adjust_skill_damage to display
 *------------------------------------------*/
ACMD(mapinfo) {
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;
	struct npc_data *nd = NULL;
	struct chat_data *cd = NULL;
	char direction[12];
	int i, m_id, chat_num = 0, list = 0, vend_num = 0;
	unsigned short m_index;
	char mapname[24];
		
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(mapname, '\0', sizeof(mapname));
	memset(direction, '\0', sizeof(direction));
	
	sscanf(message, "%d %23[^\n]", &list, mapname);
	
	if (list < 0 || list > 3) {
		clif->message(fd, msg_txt(1038)); // Please enter at least one valid list number (usage: @mapinfo <0-3> <map>).
		return false;
	}
	
	if (mapname[0] == '\0') {
		safestrncpy(mapname, mapindex_id2name(sd->mapindex), MAP_NAME_LENGTH);
		m_id =  map->mapindex2mapid(sd->mapindex);
	} else {
		m_id = map->mapname2mapid(mapname);
	}
	
	if (m_id < 0) {
		clif->message(fd, msg_txt(1)); // Map not found.
		return false;
	}
	m_index = mapindex->name2id(mapname); //This one shouldn't fail since the previous seek did not.
	
	clif->message(fd, msg_txt(1039)); // ------ Map Info ------
	
	// count chats (for initial message)
	chat_num = 0;
	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); pl_sd = (TBL_PC*)mapit->next(iter) ) {
		if( pl_sd->mapindex == m_index ) {
			if( pl_sd->state.vending )
				vend_num++;
			else if( (cd = (struct chat_data*)map->id2bl(pl_sd->chatID)) != NULL && cd->usersd[0] == pl_sd )
				chat_num++;
		}
	}
	mapit->free(iter);
	
	sprintf(atcmd_output, msg_txt(1040), mapname, map->list[m_id].zone->name, map->list[m_id].users, map->list[m_id].npc_num, chat_num, vend_num); // Map: %s (Zone:%s) | Players: %d | NPCs: %d | Chats: %d | Vendings: %d
	clif->message(fd, atcmd_output);
	clif->message(fd, msg_txt(1041)); // ------ Map Flags ------
	if (map->list[m_id].flag.town)
		clif->message(fd, msg_txt(1042)); // Town Map
	
	if (battle_config.autotrade_mapflag == map->list[m_id].flag.autotrade)
		clif->message(fd, msg_txt(1043)); // Autotrade Enabled
	else
		clif->message(fd, msg_txt(1044)); // Autotrade Disabled
	
	if (map->list[m_id].flag.battleground)
		clif->message(fd, msg_txt(1045)); // Battlegrounds ON
	
	strcpy(atcmd_output,msg_txt(1046)); // PvP Flags:
	if (map->list[m_id].flag.pvp)
		strcat(atcmd_output, msg_txt(1047)); // Pvp ON |
	if (map->list[m_id].flag.pvp_noguild)
		strcat(atcmd_output, msg_txt(1048)); // NoGuild |
	if (map->list[m_id].flag.pvp_noparty)
		strcat(atcmd_output, msg_txt(1049)); // NoParty |
	if (map->list[m_id].flag.pvp_nightmaredrop)
		strcat(atcmd_output, msg_txt(1050)); // NightmareDrop |
	if (map->list[m_id].flag.pvp_nocalcrank)
		strcat(atcmd_output, msg_txt(1051)); // NoCalcRank |
	clif->message(fd, atcmd_output);
	
	strcpy(atcmd_output,msg_txt(1052)); // GvG Flags:
	if (map->list[m_id].flag.gvg)
		strcat(atcmd_output, msg_txt(1053)); // GvG ON |
	if (map->list[m_id].flag.gvg_dungeon)
		strcat(atcmd_output, msg_txt(1054)); // GvG Dungeon |
	if (map->list[m_id].flag.gvg_castle)
		strcat(atcmd_output, msg_txt(1055)); // GvG Castle |
	if (map->list[m_id].flag.gvg_noparty)
		strcat(atcmd_output, msg_txt(1056)); // NoParty |
	clif->message(fd, atcmd_output);
	
	strcpy(atcmd_output,msg_txt(1057)); // Teleport Flags:
	if (map->list[m_id].flag.noteleport)
		strcat(atcmd_output, msg_txt(1058)); // NoTeleport |
	if (map->list[m_id].flag.monster_noteleport)
		strcat(atcmd_output, msg_txt(1059)); // Monster NoTeleport |
	if (map->list[m_id].flag.nowarp)
		strcat(atcmd_output, msg_txt(1060)); // NoWarp |
	if (map->list[m_id].flag.nowarpto)
		strcat(atcmd_output, msg_txt(1061)); // NoWarpTo |
	if (map->list[m_id].flag.noreturn)
		strcat(atcmd_output, msg_txt(1062)); // NoReturn |
	if (map->list[m_id].flag.nomemo)
		strcat(atcmd_output, msg_txt(1064)); // NoMemo |
	clif->message(fd, atcmd_output);
	
	sprintf(atcmd_output, msg_txt(1065),  // No Exp Penalty: %s | No Zeny Penalty: %s
			(map->list[m_id].flag.noexppenalty) ? msg_txt(1066) : msg_txt(1067),
			(map->list[m_id].flag.nozenypenalty) ? msg_txt(1066) : msg_txt(1067)); // On / Off
	clif->message(fd, atcmd_output);
	
	if (map->list[m_id].flag.nosave) {
		if (!map->list[m_id].save.map)
			clif->message(fd, msg_txt(1068)); // No Save (Return to last Save Point)
		else if (map->list[m_id].save.x == -1 || map->list[m_id].save.y == -1 ) {
			sprintf(atcmd_output, msg_txt(1069), mapindex_id2name(map->list[m_id].save.map)); // No Save, Save Point: %s,Random
			clif->message(fd, atcmd_output);
		} else {
			sprintf(atcmd_output, msg_txt(1070), // No Save, Save Point: %s,%d,%d
					mapindex_id2name(map->list[m_id].save.map),map->list[m_id].save.x,map->list[m_id].save.y);
			clif->message(fd, atcmd_output);
		}
	}
	
	strcpy(atcmd_output,msg_txt(1071)); // Weather Flags:
	if (map->list[m_id].flag.snow)
		strcat(atcmd_output, msg_txt(1072)); // Snow |
	if (map->list[m_id].flag.fog)
		strcat(atcmd_output, msg_txt(1073)); // Fog |
	if (map->list[m_id].flag.sakura)
		strcat(atcmd_output, msg_txt(1074)); // Sakura |
	if (map->list[m_id].flag.clouds)
		strcat(atcmd_output, msg_txt(1075)); // Clouds |
	if (map->list[m_id].flag.clouds2)
		strcat(atcmd_output, msg_txt(1076)); // Clouds2 |
	if (map->list[m_id].flag.fireworks)
		strcat(atcmd_output, msg_txt(1077)); // Fireworks |
	if (map->list[m_id].flag.leaves)
		strcat(atcmd_output, msg_txt(1078)); // Leaves |
	if (map->list[m_id].flag.nightenabled)
		strcat(atcmd_output, msg_txt(1080)); // Displays Night |
	clif->message(fd, atcmd_output);
	
	strcpy(atcmd_output,msg_txt(1081)); // Other Flags:
	if (map->list[m_id].flag.nobranch)
		strcat(atcmd_output, msg_txt(1082)); // NoBranch |
	if (map->list[m_id].flag.notrade)
		strcat(atcmd_output, msg_txt(1083)); // NoTrade |
	if (map->list[m_id].flag.novending)
		strcat(atcmd_output, msg_txt(1084)); // NoVending |
	if (map->list[m_id].flag.nodrop)
		strcat(atcmd_output, msg_txt(1085)); // NoDrop |
	if (map->list[m_id].flag.noskill)
		strcat(atcmd_output, msg_txt(1086)); // NoSkill |
	if (map->list[m_id].flag.noicewall)
		strcat(atcmd_output, msg_txt(1087)); // NoIcewall |
	if (map->list[m_id].flag.allowks)
		strcat(atcmd_output, msg_txt(1088)); // AllowKS |
	if (map->list[m_id].flag.reset)
		strcat(atcmd_output, msg_txt(1089)); // Reset |
	clif->message(fd, atcmd_output);
	
	strcpy(atcmd_output,msg_txt(1090)); // Other Flags:
	if (map->list[m_id].nocommand)
		strcat(atcmd_output, msg_txt(1091)); // NoCommand |
	if (map->list[m_id].flag.nobaseexp)
		strcat(atcmd_output, msg_txt(1092)); // NoBaseEXP |
	if (map->list[m_id].flag.nojobexp)
		strcat(atcmd_output, msg_txt(1093)); // NoJobEXP |
	if (map->list[m_id].flag.nomobloot)
		strcat(atcmd_output, msg_txt(1094)); // NoMobLoot |
	if (map->list[m_id].flag.nomvploot)
		strcat(atcmd_output, msg_txt(1095)); // NoMVPLoot |
	if (map->list[m_id].flag.partylock)
		strcat(atcmd_output, msg_txt(1096)); // PartyLock |
	if (map->list[m_id].flag.guildlock)
		strcat(atcmd_output, msg_txt(1097)); // GuildLock |
	clif->message(fd, atcmd_output);
	
	switch (list) {
		case 0:
			// Do nothing. It's list 0, no additional display.
			break;
		case 1:
			clif->message(fd, msg_txt(1098)); // ----- Players in Map -----
			iter = mapit_getallusers();
			for( pl_sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); pl_sd = (TBL_PC*)mapit->next(iter) )
			{
				if (pl_sd->mapindex == m_index) {
					sprintf(atcmd_output, msg_txt(1099), // Player '%s' (session #%d) | Location: %d,%d
							pl_sd->status.name, pl_sd->fd, pl_sd->bl.x, pl_sd->bl.y);
					clif->message(fd, atcmd_output);
				}
			}
			mapit->free(iter);
			break;
		case 2:
			clif->message(fd, msg_txt(1100)); // ----- NPCs in Map -----
			for (i = 0; i < map->list[m_id].npc_num;) {
				nd = map->list[m_id].npc[i];
				switch(nd->dir) {
					case 0:  strcpy(direction, msg_txt(1101)); break; // North
					case 1:  strcpy(direction, msg_txt(1102)); break; // North West
					case 2:  strcpy(direction, msg_txt(1103)); break; // West
					case 3:  strcpy(direction, msg_txt(1104)); break; // South West
					case 4:  strcpy(direction, msg_txt(1105)); break; // South
					case 5:  strcpy(direction, msg_txt(1106)); break; // South East
					case 6:  strcpy(direction, msg_txt(1107)); break; // East
					case 7:  strcpy(direction, msg_txt(1108)); break; // North East
					case 9:  strcpy(direction, msg_txt(1109)); break; // North
					default: strcpy(direction, msg_txt(1110)); break; // Unknown
				}
				if(strcmp(nd->name,nd->exname) == 0)
					sprintf(atcmd_output, msg_txt(1111), // NPC %d: %s | Direction: %s | Sprite: %d | Location: %d %d
							++i, nd->name, direction, nd->class_, nd->bl.x, nd->bl.y);
				else
					sprintf(atcmd_output, msg_txt(1112), // NPC %d: %s::%s | Direction: %s | Sprite: %d | Location: %d %d
							++i, nd->name, nd->exname, direction, nd->class_, nd->bl.x, nd->bl.y);
				clif->message(fd, atcmd_output);
			}
			break;
		case 3:
			clif->message(fd, msg_txt(1113)); // ----- Chats in Map -----
			iter = mapit_getallusers();
			for( pl_sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); pl_sd = (TBL_PC*)mapit->next(iter) )
			{
				if ((cd = (struct chat_data*)map->id2bl(pl_sd->chatID)) != NULL &&
					pl_sd->mapindex == m_index &&
					cd->usersd[0] == pl_sd)
				{
					sprintf(atcmd_output, msg_txt(1114), // Chat: %s | Player: %s | Location: %d %d
							cd->title, pl_sd->status.name, cd->bl.x, cd->bl.y);
					clif->message(fd, atcmd_output);
					sprintf(atcmd_output, msg_txt(1115), //    Users: %d/%d | Password: %s | Public: %s
							cd->users, cd->limit, cd->pass, (cd->pub) ? msg_txt(1116) : msg_txt(1117)); // Yes / No
					clif->message(fd, atcmd_output);
				}
			}
			mapit->free(iter);
			break;
		default: // normally impossible to arrive here
			clif->message(fd, msg_txt(1118)); // Please enter at least one valid list number (usage: @mapinfo <0-3> <map>).
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
		clif->message(fd, msg_txt(212)); // Cannot mount while in disguise.
		return false;
	}
	
	if( sd->sc.data[SC_ALL_RIDING] ) {
		clif->message(fd, msg_txt(1476)); // You are already mounting something else
		return false;
	}
	
	if( (sd->class_&MAPID_THIRDMASK) == MAPID_RUNE_KNIGHT && pc->checkskill(sd,RK_DRAGONTRAINING) > 0 ) {
		if( !(sd->sc.option&OPTION_DRAGON1) ) {
			clif->message(sd->fd,msg_txt(1119)); // You have mounted your Dragon.
			pc->setoption(sd, sd->sc.option|OPTION_DRAGON1);
		} else {
			clif->message(sd->fd,msg_txt(1120)); // You have released your Dragon.
			pc->setoption(sd, sd->sc.option&~OPTION_DRAGON1);
		}
		return true;
	}
	if( (sd->class_&MAPID_THIRDMASK) == MAPID_RANGER && pc->checkskill(sd,RA_WUGRIDER) > 0 ) {
		if( !pc_isridingwug(sd) ) {
			clif->message(sd->fd,msg_txt(1121)); // You have mounted your Warg.
			pc->setoption(sd, sd->sc.option|OPTION_WUGRIDER);
		} else {
			clif->message(sd->fd,msg_txt(1122)); // You have released your Warg.
			pc->setoption(sd, sd->sc.option&~OPTION_WUGRIDER);
		}
		return true;
	}
	if( (sd->class_&MAPID_THIRDMASK) == MAPID_MECHANIC ) {
		if( !pc_ismadogear(sd) ) {
			clif->message(sd->fd,msg_txt(1123)); // You have mounted your Mado Gear.
			pc->setoption(sd, sd->sc.option|OPTION_MADOGEAR);
		} else {
			clif->message(sd->fd,msg_txt(1124)); // You have released your Mado Gear.
			pc->setoption(sd, sd->sc.option&~OPTION_MADOGEAR);
		}
		return true;
	}
	if (!pc_isriding(sd)) { // if actually no peco
		
		if (!pc->checkskill(sd, KN_RIDING)) {
			clif->message(fd, msg_txt(213)); // You can not mount a Peco Peco with your current job.
			return false;
		}
		
		pc->setoption(sd, sd->sc.option | OPTION_RIDING);
		clif->message(fd, msg_txt(102)); // You have mounted a Peco Peco.
	} else {//Dismount
		pc->setoption(sd, sd->sc.option & ~OPTION_RIDING);
		clif->message(fd, msg_txt(214)); // You have released your Peco Peco.
	}
	
	return true;
}

/*==========================================
 *Spy Commands by Syrus22
 *------------------------------------------*/
ACMD(guildspy) {
	char guild_name[NAME_LENGTH];
	struct guild *g;
	
	memset(guild_name, '\0', sizeof(guild_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	
	if (!map->enable_spy)
	{
		clif->message(fd, msg_txt(1125)); // The mapserver has spy command support disabled.
		return false;
	}
	if (!message || !*message || sscanf(message, "%23[^\n]", guild_name) < 1) {
		clif->message(fd, msg_txt(1126)); // Please enter a guild name/ID (usage: @guildspy <guild_name/ID>).
		return false;
	}
	
	if ((g = guild->searchname(guild_name)) != NULL || // name first to avoid error when name begin with a number
	    (g = guild->search(atoi(message))) != NULL) {
		if (sd->guildspy == g->guild_id) {
			sd->guildspy = 0;
			sprintf(atcmd_output, msg_txt(103), g->name); // No longer spying on the %s guild.
			clif->message(fd, atcmd_output);
		} else {
			sd->guildspy = g->guild_id;
			sprintf(atcmd_output, msg_txt(104), g->name); // Spying on the %s guild.
			clif->message(fd, atcmd_output);
		}
	} else {
		clif->message(fd, msg_txt(94)); // Incorrect name/ID, or no one from the specified guild is online.
		return false;
	}
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(partyspy) {
	char party_name[NAME_LENGTH];
	struct party_data *p;
	
	memset(party_name, '\0', sizeof(party_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	
	if (!map->enable_spy)
	{
		clif->message(fd, msg_txt(1125)); // The mapserver has spy command support disabled.
		return false;
	}
	
	if (!message || !*message || sscanf(message, "%23[^\n]", party_name) < 1) {
		clif->message(fd, msg_txt(1127)); // Please enter a party name/ID (usage: @partyspy <party_name/ID>).
		return false;
	}
	
	if ((p = party->searchname(party_name)) != NULL || // name first to avoid error when name begin with a number
	    (p = party->search(atoi(message))) != NULL) {
		if (sd->partyspy == p->party.party_id) {
			sd->partyspy = 0;
			sprintf(atcmd_output, msg_txt(105), p->party.name); // No longer spying on the %s party.
			clif->message(fd, atcmd_output);
		} else {
			sd->partyspy = p->party.party_id;
			sprintf(atcmd_output, msg_txt(106), p->party.name); // Spying on the %s party.
			clif->message(fd, atcmd_output);
		}
	} else {
		clif->message(fd, msg_txt(96)); // Incorrect name/ID, or no one from the specified party is online.
		return false;
	}
	
	return true;
}

/*==========================================
 * @repairall [Valaris]
 *------------------------------------------*/
ACMD(repairall)
{
	int count, i;
	
	count = 0;
	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].nameid && sd->status.inventory[i].attribute == 1) {
			sd->status.inventory[i].attribute = 0;
			clif->produce_effect(sd, 0, sd->status.inventory[i].nameid);
			count++;
		}
	}
	
	if (count > 0) {
		clif->misceffect(&sd->bl, 3);
		clif->equiplist(sd);
		clif->message(fd, msg_txt(107)); // All items have been repaired.
	} else {
		clif->message(fd, msg_txt(108)); // No item need to be repaired.
		return false;
	}
	
	return true;
}

/*==========================================
 * @nuke [Valaris]
 *------------------------------------------*/
ACMD(nuke) {
	struct map_session_data *pl_sd;
	
	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));
	
	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif->message(fd, msg_txt(1128)); // Please enter a player name (usage: @nuke <char name>).
		return false;
	}
	
	if ((pl_sd = map->nick2sd(atcmd_player_name)) != NULL) {
		if (pc_get_group_level(sd) >= pc_get_group_level(pl_sd)) { // you can kill only lower or same GM level
			skill->castend_nodamage_id(&pl_sd->bl, &pl_sd->bl, NPC_SELFDESTRUCTION, 99, timer->gettick(), 0);
			clif->message(fd, msg_txt(109)); // Player has been nuked!
		} else {
			clif->message(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	} else {
		clif->message(fd, msg_txt(3)); // Character not found.
		return false;
	}
	
	return true;
}

/*==========================================
 * @tonpc
 *------------------------------------------*/
ACMD(tonpc) {
	char npcname[NAME_LENGTH+1];
	struct npc_data *nd;
		
	memset(npcname, 0, sizeof(npcname));
	
	if (!message || !*message || sscanf(message, "%23[^\n]", npcname) < 1) {
		clif->message(fd, msg_txt(1129)); // Please enter a NPC name (usage: @tonpc <NPC_name>).
		return false;
	}
	
	if ((nd = npc->name2id(npcname)) != NULL) {
		if (nd->bl.m != -1 && pc->setpos(sd, map_id2index(nd->bl.m), nd->bl.x, nd->bl.y, CLR_TELEPORT) == 0)
			clif->message(fd, msg_txt(0)); // Warped.
		else
			return false;
	} else {
		clif->message(fd, msg_txt(111)); // This NPC doesn't exist.
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
	
	if (!message || !*message || sscanf(message, "%23[^\n]", NPCname) < 1) {
		clif->message(fd, msg_txt(1130)); // Please enter a NPC name (usage: @enablenpc <NPC_name>).
		return false;
	}
	
	if (npc->name2id(NPCname) != NULL) {
		npc->enable(NPCname, 1);
		clif->message(fd, msg_txt(110)); // Npc Enabled.
	} else {
		clif->message(fd, msg_txt(111)); // This NPC doesn't exist.
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
	
	if (!message || !*message || sscanf(message, "%23[^\n]", NPCname) < 1) {
		clif->message(fd, msg_txt(1131)); // Please enter a NPC name (usage: @hidenpc <NPC_name>).
		return false;
	}
	
	if (npc->name2id(NPCname) == NULL) {
		clif->message(fd, msg_txt(111)); // This NPC doesn't exist.
		return false;
	}
	
	npc->enable(NPCname, 0);
	clif->message(fd, msg_txt(112)); // Npc Disabled.
	return true;
}

ACMD(loadnpc)
{
	FILE *fp;
	
	if (!message || !*message) {
		clif->message(fd, msg_txt(1132)); // Please enter a script file name (usage: @loadnpc <file name>).
		return false;
	}
	
	// check if script file exists
	if ((fp = fopen(message, "r")) == NULL) {
		clif->message(fd, msg_txt(261));
		return false;
	}
	fclose(fp);
	
	// add to list of script sources and run it
	npc->addsrcfile(message);
	npc->parsesrcfile(message,true);
	npc->read_event_script();
	
	clif->message(fd, msg_txt(262));
	
	return true;
}

ACMD(unloadnpc)
{
	struct npc_data *nd;
	char NPCname[NAME_LENGTH+1];
	
	memset(NPCname, '\0', sizeof(NPCname));
	
	if (!message || !*message || sscanf(message, "%24[^\n]", NPCname) < 1) {
		clif->message(fd, msg_txt(1133)); // Please enter a NPC name (usage: @npcoff <NPC_name>).
		return false;
	}
	
	if ((nd = npc->name2id(NPCname)) == NULL) {
		clif->message(fd, msg_txt(111)); // This NPC doesn't exist.
		return false;
	}
	
	npc->unload_duplicates(nd);
	npc->unload(nd,true);
	npc->read_event_script();
	clif->message(fd, msg_txt(112)); // Npc Disabled.
	return true;
}

/*==========================================
 * time in txt for time command (by [Yor])
 *------------------------------------------*/
char* txt_time(unsigned int duration)
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
		tlen += sprintf(tlen + temp1, msg_txt(219), days); // %d day
	else if (days > 1)
		tlen += sprintf(tlen + temp1, msg_txt(220), days); // %d days
	if (hours == 1)
		tlen += sprintf(tlen + temp1, msg_txt(221), hours); // %d hour
	else if (hours > 1)
		tlen += sprintf(tlen + temp1, msg_txt(222), hours); // %d hours
	if (minutes < 2)
		tlen += sprintf(tlen + temp1, msg_txt(223), minutes); // %d minute
	else
		tlen += sprintf(tlen + temp1, msg_txt(224), minutes); // %d minutes
	if (seconds == 1)
		sprintf(tlen + temp1, msg_txt(225), seconds); // and %d second
	else if (seconds > 1)
		sprintf(tlen + temp1, msg_txt(226), seconds); // and %d seconds
		
	return temp1;
}

/*==========================================
 * @time/@date/@serverdate/@servertime: Display the date/time of the server (by [Yor]
 * Calculation management of GM modification (@day/@night GM commands) is done
 *------------------------------------------*/
ACMD(servertime) {
	time_t time_server;  // variable for number of seconds (used with time() function)
	struct tm *datetime; // variable for time in structure ->tm_mday, ->tm_sec, ...
	char temp[CHAT_SIZE_MAX];
	
	memset(temp, '\0', sizeof(temp));
	
	time(&time_server);  // get time in seconds since 1/1/1970
	datetime = localtime(&time_server); // convert seconds in structure
	// like sprintf, but only for date/time (Sunday, November 02 2003 15:12:52)
	strftime(temp, sizeof(temp)-1, msg_txt(230), datetime); // Server time (normal time): %A, %B %d %Y %X.
	clif->message(fd, temp);
	
	if (pc->day_timer_tid != INVALID_TIMER && pc->night_timer_tid != INVALID_TIMER) {
		const struct TimerData * timer_data = timer->get(pc->night_timer_tid);
		const struct TimerData * timer_data2 = timer->get(pc->day_timer_tid);
		
		if (map->night_flag == 0) {
			sprintf(temp, msg_txt(235), // Game time: The game is actualy in daylight for %s.
			        txt_time((unsigned int)(DIFF_TICK(timer_data->tick,timer->gettick())/1000)));
			clif->message(fd, temp);
			if (DIFF_TICK(timer_data->tick, timer_data2->tick) > 0)
				sprintf(temp, msg_txt(237), // Game time: After, the game will be in night for %s.
				        txt_time((unsigned int)(DIFF_TICK(timer_data->interval,DIFF_TICK(timer_data->tick,timer_data2->tick)) / 1000)));
			else
				sprintf(temp, msg_txt(237), // Game time: After, the game will be in night for %s.
				        txt_time((unsigned int)(DIFF_TICK(timer_data2->tick,timer_data->tick)/1000)));
			clif->message(fd, temp);
		} else {
			sprintf(temp, msg_txt(233), // Game time: The game is actualy in night for %s.
			        txt_time((unsigned int)(DIFF_TICK(timer_data2->tick,timer->gettick()) / 1000)));
			clif->message(fd, temp);
			if (DIFF_TICK(timer_data2->tick,timer_data->tick) > 0)
				sprintf(temp, msg_txt(239), // Game time: After, the game will be in daylight for %s.
				        txt_time((unsigned int)((timer_data2->interval - DIFF_TICK(timer_data2->tick, timer_data->tick)) / 1000)));
			else
				sprintf(temp, msg_txt(239), // Game time: After, the game will be in daylight for %s.
				        txt_time((unsigned int)(DIFF_TICK(timer_data->tick, timer_data2->tick) / 1000)));
			clif->message(fd, temp);
		}
		sprintf(temp, msg_txt(238), txt_time(timer_data2->interval / 1000)); // Game time: A day cycle has a normal duration of %s.
		clif->message(fd, temp);
	} else {
		if (map->night_flag == 0)
			clif->message(fd, msg_txt(231)); // Game time: The game is in permanent daylight.
		else
			clif->message(fd, msg_txt(232)); // Game time: The game is in permanent night.
	}
	
	return true;
}

//Added by Coltaro
//We're using this function here instead of using time_t so that it only counts player's jail time when he/she's online (and since the idea is to reduce the amount of minutes one by one in status->change_timer...).
//Well, using time_t could still work but for some reason that looks like more coding x_x
void get_jail_time(int jailtime, int* year, int* month, int* day, int* hour, int* minute)
{
	const int factor_year = 518400; //12*30*24*60 = 518400
	const int factor_month = 43200; //30*24*60 = 43200
	const int factor_day = 1440; //24*60 = 1440
	const int factor_hour = 60;
	
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
ACMD(jail) {
	struct map_session_data *pl_sd;
	int x, y;
	unsigned short m_index;
	
	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));
	
	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif->message(fd, msg_txt(1134)); // Please enter a player name (usage: @jail <char_name>).
		return false;
	}
	
	if ((pl_sd = map->nick2sd(atcmd_player_name)) == NULL) {
		clif->message(fd, msg_txt(3)); // Character not found.
		return false;
	}
	
	if (pc_get_group_level(sd) < pc_get_group_level(pl_sd))
  	{ // you can jail only lower or same GM
		clif->message(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return false;
	}
	
	if (pl_sd->sc.data[SC_JAILED])
	{
		clif->message(fd, msg_txt(118)); // Player warped in jails.
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
	sc_start4(NULL,&pl_sd->bl,SC_JAILED,100,INT_MAX,m_index,x,y,1000);
	clif->message(pl_sd->fd, msg_txt(117)); // You have been jailed by a GM.
	clif->message(fd, msg_txt(118)); // Player warped in jails.
	return true;
}

/*==========================================
 * @unjail/@discharge <char_name> by [Yor]
 * Special warp! No check with nowarp and nowarpto flag
 *------------------------------------------*/
ACMD(unjail) {
	struct map_session_data *pl_sd;
	
	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));
	
	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif->message(fd, msg_txt(1135)); // Please enter a player name (usage: @unjail/@discharge <char_name>).
		return false;
	}
	
	if ((pl_sd = map->nick2sd(atcmd_player_name)) == NULL) {
		clif->message(fd, msg_txt(3)); // Character not found.
		return false;
	}
	
	if (pc_get_group_level(sd) < pc_get_group_level(pl_sd)) { // you can jail only lower or same GM
		
		clif->message(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return false;
	}
	
	if (!pl_sd->sc.data[SC_JAILED])
	{
		clif->message(fd, msg_txt(119)); // This player is not in jails.
		return false;
	}
	
	//Reset jail time to 1 sec.
	sc_start(NULL,&pl_sd->bl,SC_JAILED,100,1,1000);
	clif->message(pl_sd->fd, msg_txt(120)); // A GM has discharged you from jail.
	clif->message(fd, msg_txt(121)); // Player unjailed.
	return true;
}

ACMD(jailfor) {
	struct map_session_data *pl_sd = NULL;
	int year, month, day, hour, minute, value;
	char * modif_p;
	int jailtime = 0,x,y;
	short m_index = 0;
	
	if (!message || !*message || sscanf(message, "%255s %23[^\n]",atcmd_output,atcmd_player_name) < 2) {
		clif->message(fd, msg_txt(400));	//Usage: @jailfor <time> <character name>
		return false;
	}
	
	atcmd_output[sizeof(atcmd_output)-1] = '\0';
	
	modif_p = atcmd_output;
	year = month = day = hour = minute = 0;
	while (modif_p[0] != '\0') {
		value = atoi(modif_p);
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
		clif->message(fd, msg_txt(1136)); // Invalid time for jail command.
		return false;
	}
	
	if ((pl_sd = map->nick2sd(atcmd_player_name)) == NULL) {
		clif->message(fd, msg_txt(3)); // Character not found.
		return false;
	}
	
	if (pc_get_group_level(pl_sd) > pc_get_group_level(sd)) {
		clif->message(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return false;
	}
	
	jailtime = year*12*30*24*60 + month*30*24*60 + day*24*60 + hour*60 + minute;	//In minutes
	
	if(jailtime==0) {
		clif->message(fd, msg_txt(1136)); // Invalid time for jail command.
		return false;
	}
	
	//Added by Coltaro
	if(pl_sd->sc.data[SC_JAILED] &&
	   pl_sd->sc.data[SC_JAILED]->val1 != INT_MAX)
  	{	//Update the player's jail time
		jailtime += pl_sd->sc.data[SC_JAILED]->val1;
		if (jailtime <= 0) {
			jailtime = 0;
			clif->message(pl_sd->fd, msg_txt(120)); // GM has discharge you.
			clif->message(fd, msg_txt(121)); // Player unjailed
		} else {
			atcommand->get_jail_time(jailtime,&year,&month,&day,&hour,&minute);
			sprintf(atcmd_output,msg_txt(402),msg_txt(1137),year,month,day,hour,minute); //%s in jail for %d years, %d months, %d days, %d hours and %d minutes
	 		clif->message(pl_sd->fd, atcmd_output);
			sprintf(atcmd_output,msg_txt(402),msg_txt(1138),year,month,day,hour,minute); //This player is now in jail for %d years, %d months, %d days, %d hours and %d minutes
	 		clif->message(fd, atcmd_output);
		}
	} else if (jailtime < 0) {
		clif->message(fd, msg_txt(1136));
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
	
	sc_start4(NULL,&pl_sd->bl,SC_JAILED,100,jailtime,m_index,x,y,jailtime?60000:1000); //jailtime = 0: Time was reset to 0. Wait 1 second to warp player out (since it's done in status->change_timer).
	return true;
}


//By Coltaro
ACMD(jailtime)
{
	int year, month, day, hour, minute;
	
	if (!sd->sc.data[SC_JAILED]) {
		clif->message(fd, msg_txt(1139)); // You are not in jail.
		return false;
	}
	
	if (sd->sc.data[SC_JAILED]->val1 == INT_MAX) {
		clif->message(fd, msg_txt(1140)); // You have been jailed indefinitely.
		return true;
	}
	
	if (sd->sc.data[SC_JAILED]->val1 <= 0) { // Was not jailed with @jailfor (maybe @jail? or warped there? or got recalled?)
		clif->message(fd, msg_txt(1141)); // You have been jailed for an unknown amount of time.
		return false;
	}
	
	//Get remaining jail time
	atcommand->get_jail_time(sd->sc.data[SC_JAILED]->val1,&year,&month,&day,&hour,&minute);
	sprintf(atcmd_output,msg_txt(402),msg_txt(1142),year,month,day,hour,minute); // You will remain in jail for %d years, %d months, %d days, %d hours and %d minutes
	
	clif->message(fd, atcmd_output);
	
	return true;
}

/*==========================================
 * @disguise <mob_id> by [Valaris] (simplified by [Yor])
 *------------------------------------------*/
ACMD(disguise)
{
	int id = 0;
	
	if (!message || !*message) {
		clif->message(fd, msg_txt(1143)); // Please enter a Monster/NPC name/ID (usage: @disguise <name/ID>).
		return false;
	}
	
	if ((id = atoi(message)) > 0)
	{	//Acquired an ID
		if (!mob->db_checkid(id) && !npcdb_checkid(id))
			id = 0; //Invalid id for either mobs or npcs.
	}	else	{ //Acquired a Name
		if ((id = mob->db_searchname(message)) == 0)
		{
			struct npc_data* nd = npc->name2id(message);
			if (nd != NULL)
				id = nd->class_;
		}
	}
	
	if (id == 0)
	{
		clif->message(fd, msg_txt(123));	// Invalid Monster/NPC name/ID specified.
		return false;
	}
	
	if(pc_isriding(sd))
	{
		clif->message(fd, msg_txt(1144)); // Character cannot be disguised while mounted.
		return false;
	}
		
	if(sd->sc.data[SC_MONSTER_TRANSFORM])
	{
		clif->message(fd, msg_txt(1487)); // Character cannot be disguised while in monster form.
		return false;
	}

	pc->disguise(sd, id);
	clif->message(fd, msg_txt(122)); // Disguise applied.
	
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
	
	if (!message || !*message) {
		clif->message(fd, msg_txt(1145)); // Please enter a Monster/NPC name/ID (usage: @disguiseall <name/ID>).
		return false;
	}
	
	if ((mob_id = mob->db_searchname(message)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = atoi(message);
	
	if (!mob->db_checkid(mob_id) && !npcdb_checkid(mob_id)) { //if mob or npc...
		clif->message(fd, msg_txt(123)); // Monster/NPC name/id not found.
		return false;
	}
	
	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); pl_sd = (TBL_PC*)mapit->next(iter) )
		pc->disguise(pl_sd, mob_id);
	mapit->free(iter);
	
	clif->message(fd, msg_txt(122)); // Disguise applied.
	return true;
}

/*==========================================
 * DisguiseGuild
 *------------------------------------------*/
ACMD(disguiseguild)
{
	int id = 0, i;
	char monster[NAME_LENGTH], guild_name[NAME_LENGTH];
	struct map_session_data *pl_sd;
	struct guild *g;
	
	memset(monster, '\0', sizeof(monster));
	memset(guild_name, '\0', sizeof(guild_name));
	
	if( !message || !*message || sscanf(message, "%23[^,], %23[^\r\n]", monster, guild_name) < 2 ) {
		clif->message(fd, msg_txt(1146)); // Please enter a mob name/ID and guild name/ID (usage: @disguiseguild <mob name/ID>, <guild name/ID>).
		return false;
	}
	
	if( (id = atoi(monster)) > 0 ) {
		if( !mob->db_checkid(id) && !npcdb_checkid(id) )
			id = 0;
	} else {
		if( (id = mob->db_searchname(monster)) == 0 ) {
			struct npc_data* nd = npc->name2id(monster);
			if( nd != NULL )
				id = nd->class_;
		}
	}
	
	if( id == 0 ) {
		clif->message(fd, msg_txt(123));	// Monster/NPC name/id hasn't been found.
		return false;
	}
	
	if( (g = guild->searchname(guild_name)) == NULL && (g = guild->search(atoi(guild_name))) == NULL ) {
		clif->message(fd, msg_txt(94)); // Incorrect name/ID, or no one from the guild is online.
		return false;
	}
	
	for( i = 0; i < g->max_member; i++ )
		if( (pl_sd = g->member[i].sd) && !pc_isriding(pl_sd) )
			pc->disguise(pl_sd, id);
	
	clif->message(fd, msg_txt(122)); // Disguise applied.
	return true;
}


/*==========================================
 * @undisguise by [Yor]
 *------------------------------------------*/
ACMD(undisguise)
{
	if (sd->disguise != -1) {
		pc->disguise(sd, -1);
		clif->message(fd, msg_txt(124)); // Disguise removed.
	} else {
		clif->message(fd, msg_txt(125)); // You're not disguised.
		return false;
	}
	
	return true;
}

/*==========================================
 * UndisguiseAll
 *------------------------------------------*/
ACMD(undisguiseall) {
	struct map_session_data *pl_sd;
	struct s_mapiterator* iter;
	
	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); pl_sd = (TBL_PC*)mapit->next(iter) )
		if( pl_sd->disguise != -1 )
			pc->disguise(pl_sd, -1);
	mapit->free(iter);
	
	clif->message(fd, msg_txt(124)); // Disguise removed.
	
	return true;
}

/*==========================================
 * UndisguiseGuild
 *------------------------------------------*/
ACMD(undisguiseguild)
{
	char guild_name[NAME_LENGTH];
	struct map_session_data *pl_sd;
	struct guild *g;
	int i;
	
	memset(guild_name, '\0', sizeof(guild_name));
	
	if(!message || !*message || sscanf(message, "%23[^\n]", guild_name) < 1) {
		clif->message(fd, msg_txt(1147)); // Please enter guild name/ID (usage: @undisguiseguild <guild name/ID>).
		return false;
	}
	
	if( (g = guild->searchname(guild_name)) == NULL && (g = guild->search(atoi(message))) == NULL ) {
		clif->message(fd, msg_txt(94)); // Incorrect name/ID, or no one from the guild is online.
		return false;
	}
	
	for(i = 0; i < g->max_member; i++)
		if( (pl_sd = g->member[i].sd) && pl_sd->disguise != -1 )
			pc->disguise(pl_sd, -1);
	
	clif->message(fd, msg_txt(124)); // Disguise removed.
	
	return true;
}

/*==========================================
 * @exp by [Skotlex]
 *------------------------------------------*/
ACMD(exp)
{
	char output[CHAT_SIZE_MAX];
	double nextb, nextj;

	memset(output, '\0', sizeof(output));
	
	nextb = pc->nextbaseexp(sd);
	if (nextb)
		nextb = sd->status.base_exp*100.0/nextb;
	
	nextj = pc->nextjobexp(sd);
	if (nextj)
		nextj = sd->status.job_exp*100.0/nextj;
	
	sprintf(output, msg_txt(1148), sd->status.base_level, nextb, sd->status.job_level, nextj); // Base Level: %d (%.3f%%) | Job Level: %d (%.3f%%)
	clif->message(fd, output);
	return true;
}


/*==========================================
 * @broadcast by [Valaris]
 *------------------------------------------*/
ACMD(broadcast)
{
	
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	
	if (!message || !*message) {
		clif->message(fd, msg_txt(1149)); // Please enter a message (usage: @broadcast <message>).
		return false;
	}
	
	sprintf(atcmd_output, "%s: %s", sd->status.name, message);
	intif->broadcast(atcmd_output, strlen(atcmd_output) + 1, BC_DEFAULT);
	
	return true;
}

/*==========================================
 * @localbroadcast by [Valaris]
 *------------------------------------------*/
ACMD(localbroadcast)
{
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	
	if (!message || !*message) {
		clif->message(fd, msg_txt(1150)); // Please enter a message (usage: @localbroadcast <message>).
		return false;
	}
	
	sprintf(atcmd_output, "%s: %s", sd->status.name, message);
	
	clif->broadcast(&sd->bl, atcmd_output, strlen(atcmd_output) + 1, BC_DEFAULT, ALL_SAMEMAP);
	
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
	
	if (!message || !*message || sscanf(message, "%99s %99s", actual_email, new_email) < 2) {
		clif->message(fd, msg_txt(1151)); // Please enter two e-mail addresses (usage: @email <current@email> <new@email>).
		return false;
	}
	
	if (e_mail_check(actual_email) == 0) {
		clif->message(fd, msg_txt(144)); // Invalid e-mail. If your email hasn't been set, use a@a.com.
		return false;
	} else if (e_mail_check(new_email) == 0) {
		clif->message(fd, msg_txt(145)); // Invalid new email. Please enter a real e-mail address.
		return false;
	} else if (strcmpi(new_email, "a@a.com") == 0) {
		clif->message(fd, msg_txt(146)); // New email must be a real e-mail address.
		return false;
	} else if (strcmpi(actual_email, new_email) == 0) {
		clif->message(fd, msg_txt(147)); // New e-mail must be different from the current e-mail address.
		return false;
	}
	
	chrif->changeemail(sd->status.account_id, actual_email, new_email);
	clif->message(fd, msg_txt(148)); // Information sended to login-server via char-server.
	return true;
}

/*==========================================
 *@effect
 *------------------------------------------*/
ACMD(effect)
{
	int type = 0, flag = 0;
	
	if (!message || !*message || sscanf(message, "%d", &type) < 1) {
		clif->message(fd, msg_txt(1152)); // Please enter an effect number (usage: @effect <effect number>).
		return false;
	}
	
	clif->specialeffect(&sd->bl, type, (send_target)flag);
	clif->message(fd, msg_txt(229)); // Your effect has changed.
	return true;
}

/*==========================================
 * @killer by MouseJstr
 * enable killing players even when not in pvp
 *------------------------------------------*/
ACMD(killer)
{
	sd->state.killer = !sd->state.killer;
	
	if(sd->state.killer)
		clif->message(fd, msg_txt(241));
	else {
		clif->message(fd, msg_txt(292));
		pc_stop_attack(sd);
	}
	return true;
}

/*==========================================
 * @killable by MouseJstr
 * enable other people killing you
 *------------------------------------------*/
ACMD(killable) {
	sd->state.killable = !sd->state.killable;
	
	if(sd->state.killable)
		clif->message(fd, msg_txt(242));
	else {
		clif->message(fd, msg_txt(288));
		map->foreachinrange(atcommand->stopattack,&sd->bl, AREA_SIZE, BL_CHAR, sd->bl.id);
	}
	return true;
}

/*==========================================
 * @skillon by MouseJstr
 * turn skills on for the map
 *------------------------------------------*/
ACMD(skillon) {
	map->list[sd->bl.m].flag.noskill = 0;
	clif->message(fd, msg_txt(244));
	return true;
}

/*==========================================
 * @skilloff by MouseJstr
 * Turn skills off on the map
 *------------------------------------------*/
ACMD(skilloff) {
	map->list[sd->bl.m].flag.noskill = 1;
	clif->message(fd, msg_txt(243));
	return true;
}

/*==========================================
 * @npcmove by MouseJstr
 * move a npc
 *------------------------------------------*/
ACMD(npcmove) {
	int x = 0, y = 0, m;
	struct npc_data *nd = 0;

	memset(atcmd_player_name, '\0', sizeof atcmd_player_name);
	
	if (!message || !*message || sscanf(message, "%d %d %23[^\n]", &x, &y, atcmd_player_name) < 3) {
		clif->message(fd, msg_txt(1153)); // Usage: @npcmove <X> <Y> <npc_name>
		return false;
	}
	
	if ((nd = npc->name2id(atcmd_player_name)) == NULL) {
		clif->message(fd, msg_txt(111)); // This NPC doesn't exist.
		return false;
	}
	
	if ((m=nd->bl.m) < 0 || nd->bl.prev == NULL) {
		clif->message(fd, msg_txt(1154)); // NPC is not in this map.
		return false;	//Not on a map.
	}
	
	x = cap_value(x, 0, map->list[m].xs-1);
	y = cap_value(y, 0, map->list[m].ys-1);
	map->foreachinrange(clif->outsight, &nd->bl, AREA_SIZE, BL_PC, &nd->bl);
	map->moveblock(&nd->bl, x, y, timer->gettick());
	map->foreachinrange(clif->insight, &nd->bl, AREA_SIZE, BL_PC, &nd->bl);
	clif->message(fd, msg_txt(1155)); // NPC moved.
	
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
	
	if (!message || !*message || sscanf(message, "%31s %d %d %23[^\n]", mapname, &x, &y, warpname) < 4) {
		clif->message(fd, msg_txt(1156)); // Usage: @addwarp <mapname> <X> <Y> <npc name>
		return false;
	}
	
	m = mapindex->name2id(mapname);
	if( m == 0 )
	{
		sprintf(atcmd_output, msg_txt(1157), mapname); // Unknown map '%s'.
		clif->message(fd, atcmd_output);
		return false;
	}
	
	nd = npc->add_warp(warpname, sd->bl.m, sd->bl.x, sd->bl.y, 2, 2, m, x, y);
	if( nd == NULL )
		return false;
	
	sprintf(atcmd_output, msg_txt(1158), nd->exname); // New warp NPC '%s' created.
	clif->message(fd, atcmd_output);
	return true;
}

/*==========================================
 * @follow by [MouseJstr]
 * Follow a player .. staying no more then 5 spaces away
 *------------------------------------------*/
ACMD(follow) {
	struct map_session_data *pl_sd = NULL;
	
	if (!message || !*message) {
		if (sd->followtarget == -1)
			return false;
		
		pc->stop_following (sd);
		clif->message(fd, msg_txt(1159)); // Follow mode OFF.
		return true;
	}
	
	if ( (pl_sd = map->nick2sd((char *)message)) == NULL )
	{
		clif->message(fd, msg_txt(3)); // Character not found.
		return false;
	}
	
	if (sd->followtarget == pl_sd->bl.id) {
		pc->stop_following (sd);
		clif->message(fd, msg_txt(1159)); // Follow mode OFF.
	} else {
		pc->follow(sd, pl_sd->bl.id);
		clif->message(fd, msg_txt(1160)); // Follow mode ON.
	}
	
	return true;
}


/*==========================================
 * @dropall by [MouseJstr]
 * Drop all your possession on the ground
 *------------------------------------------*/
ACMD(dropall)
{
	int i;

	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].amount) {
			if(sd->status.inventory[i].equip != 0)
				pc->unequipitem(sd, i, 3);
			pc->dropitem(sd,  i, sd->status.inventory[i].amount);
		}
	}
	return true;
}

/*==========================================
 * @storeall by [MouseJstr]
 * Put everything into storage
 *------------------------------------------*/
ACMD(storeall)
{
	int i;
	
	if (sd->state.storage_flag != 1)
  	{	//Open storage.
		if( storage->open(sd) == 1 ) {
			clif->message(fd, msg_txt(1161)); // You currently cannot open your storage.
			return false;
		}
	}
	
	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].amount) {
			if(sd->status.inventory[i].equip != 0)
				pc->unequipitem(sd, i, 3);
			storage->add(sd,  i, sd->status.inventory[i].amount);
		}
	}
	storage->close(sd);
	
	clif->message(fd, msg_txt(1162)); // All items stored.
	return true;
}

ACMD(clearstorage)
{
	int i, j;
	
	if (sd->state.storage_flag == 1) {
		clif->message(fd, msg_txt(250));
		return false;
	}
	
	j = sd->status.storage.storage_amount;
	for (i = 0; i < j; ++i) {
		storage->delitem(sd, i, sd->status.storage.items[i].amount);
	}
	storage->close(sd);
	
	clif->message(fd, msg_txt(1394)); // Your storage was cleaned.
	return true;
}

ACMD(cleargstorage)
{
	int i, j;
	struct guild *g;
	struct guild_storage *guild_storage;
	
	g = sd->guild;
	
	if (g == NULL) {
		clif->message(fd, msg_txt(43));
		return false;
	}
	
	if (sd->state.storage_flag == 1) {
		clif->message(fd, msg_txt(250));
		return false;
	}
	
	if (sd->state.storage_flag == 2) {
		clif->message(fd, msg_txt(251));
		return false;
	}
	
	guild_storage = gstorage->id2storage2(sd->status.guild_id);
	if (guild_storage == NULL) {// Doesn't have opened @gstorage yet, so we skip the deletion since *shouldn't* have any item there.
		return false;
	}
	
	j = guild_storage->storage_amount;
	guild_storage->lock = 1; // Lock @gstorage: do not allow any item to be retrieved or stored from any guild member
	for (i = 0; i < j; ++i) {
		gstorage->delitem(sd, guild_storage, i, guild_storage->items[i].amount);
	}
	gstorage->close(sd);
	guild_storage->lock = 0; // Cleaning done, release lock
	
	clif->message(fd, msg_txt(1395)); // Your guild storage was cleaned.
	return true;
}

ACMD(clearcart)
{
	int i;
	
	if (pc_iscarton(sd) == 0) {
		clif->message(fd, msg_txt(1396)); // You do not have a cart to be cleaned.
		return false;
	}
	
	if( sd->state.vending == 1 ) {
		clif->message(fd, msg_txt(548)); // You can't clean a cart while vending!
 		return false;
 	}
	
	for( i = 0; i < MAX_CART; i++ )
		if(sd->status.cart[i].nameid > 0)
			pc->cart_delitem(sd, i, sd->status.cart[i].amount, 1, LOG_TYPE_OTHER);
	
	clif->clearcart(fd);
	clif->updatestatus(sd,SP_CARTINFO);
	
	clif->message(fd, msg_txt(1397)); // Your cart was cleaned.
	return true;
}

/*==========================================
 * @skillid by [MouseJstr]
 * lookup a skill by name
 *------------------------------------------*/
#define MAX_SKILLID_PARTIAL_RESULTS 5
#define MAX_SKILLID_PARTIAL_RESULTS_LEN 74 /* "skill " (6) + "%d:" (up to 5) + "%s" (up to 30) + " (%s)" (up to 33) */
ACMD(skillid) {
	int idx, i, found = 0;
	size_t skillen;
	DBIterator* iter;
	DBKey key;
	DBData *data;
	char partials[MAX_SKILLID_PARTIAL_RESULTS][MAX_SKILLID_PARTIAL_RESULTS_LEN];
		
	if (!message || !*message) {
		clif->message(fd, msg_txt(1163)); // Please enter a skill name to look up (usage: @skillid <skill name>).
		return false;
	}
	
	skillen = strlen(message);
	
	iter = db_iterator(skill->name2id_db);
	
	for( data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key) ) {
		idx = skill->get_index(DB->data2i(data));
		if (strnicmp(key.str, message, skillen) == 0 || strnicmp(skill->db[idx].desc, message, skillen) == 0) {
			sprintf(atcmd_output, msg_txt(1164), DB->data2i(data), skill->db[idx].desc, key.str); // skill %d: %s (%s)
			clif->message(fd, atcmd_output);
		} else if ( found < MAX_SKILLID_PARTIAL_RESULTS && ( stristr(key.str,message) || stristr(skill->db[idx].desc,message) ) ) {
			snprintf(partials[found++], MAX_SKILLID_PARTIAL_RESULTS_LEN, msg_txt(1164), DB->data2i(data), skill->db[idx].desc, key.str);
		}
	}
	
	dbi_destroy(iter);
	
	if( found ) {
		sprintf(atcmd_output, msg_txt(1398), found); // -- Displaying first %d partial matches
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
ACMD(useskill) {
	struct map_session_data *pl_sd = NULL;
	struct block_list *bl;
	uint16 skill_id;
	uint16 skill_lv;
	char target[100];
	
	if(!message || !*message || sscanf(message, "%hu %hu %23[^\n]", &skill_id, &skill_lv, target) != 3) {
		clif->message(fd, msg_txt(1165)); // Usage: @useskill <skill ID> <skill level> <target>
		return false;
	}
	
	if(!strcmp(target,"self"))
		pl_sd = sd; //quick keyword
	else if ( (pl_sd = map->nick2sd(target)) == NULL ) {
		clif->message(fd, msg_txt(3)); // Character not found.
		return false;
	}
	
	if ( pc_get_group_level(sd) < pc_get_group_level(pl_sd) )
	{
		clif->message(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return false;
	}
	
	if (skill_id >= HM_SKILLBASE && skill_id < HM_SKILLBASE+MAX_HOMUNSKILL
		&& sd->hd && homun_alive(sd->hd)) // (If used with @useskill, put the homunc as dest)
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
ACMD(displayskill) {
	struct status_data *st;
	int64 tick;
	uint16 skill_id;
	uint16 skill_lv = 1;
	
	if (!message || !*message || sscanf(message, "%hu %hu", &skill_id, &skill_lv) < 1) {
		clif->message(fd, msg_txt(1166)); // Usage: @displayskill <skill ID> {<skill level>}
		return false;
	}
	st = status->get_status_data(&sd->bl);
	tick = timer->gettick();
	clif->skill_damage(&sd->bl,&sd->bl, tick, st->amotion, st->dmotion, 1, 1, skill_id, skill_lv, 5);
	clif->skill_nodamage(&sd->bl, &sd->bl, skill_id, skill_lv, 1);
	clif->skill_poseffect(&sd->bl, skill_id, skill_lv, sd->bl.x, sd->bl.y, tick);
	return true;
}

/*==========================================
 * @skilltree by [MouseJstr]
 * prints the skill tree for a player required to get to a skill
 *------------------------------------------*/
ACMD(skilltree) {
	struct map_session_data *pl_sd = NULL;
	uint16 skill_id;
	int meets, j, c=0;
	char target[NAME_LENGTH];
	struct skill_tree_entry *ent;
	
	if(!message || !*message || sscanf(message, "%hu %23[^\r\n]", &skill_id, target) != 2) {
		clif->message(fd, msg_txt(1167)); // Usage: @skilltree <skill ID> <target>
		return false;
	}
	
	if ( (pl_sd = map->nick2sd(target)) == NULL ) {
		clif->message(fd, msg_txt(3)); // Character not found.
		return false;
	}
	
	c = pc->calc_skilltree_normalize_job(pl_sd);
	c = pc->mapid2jobid(c, pl_sd->status.sex);
	
	sprintf(atcmd_output, msg_txt(1168), pc->job_name(c), pc->checkskill(pl_sd, NV_BASIC)); // Player is using %s skill tree (%d basic points).
	clif->message(fd, atcmd_output);
	
	ARR_FIND( 0, MAX_SKILL_TREE, j, pc->skill_tree[c][j].id == 0 || pc->skill_tree[c][j].id == skill_id );
	if( j == MAX_SKILL_TREE || pc->skill_tree[c][j].id == 0 )
	{
		clif->message(fd, msg_txt(1169)); // The player cannot use that skill.
		return false;
	}
	
	ent = &pc->skill_tree[c][j];
	
	meets = 1;
	for(j=0;j<MAX_PC_SKILL_REQUIRE;j++)
	{
		if( ent->need[j].id && pc->checkskill(sd,ent->need[j].id) < ent->need[j].lv)
		{
			sprintf(atcmd_output, msg_txt(1170), ent->need[j].lv, skill->db[ent->need[j].id].desc); // Player requires level %d of skill %s.
			clif->message(fd, atcmd_output);
			meets = 0;
		}
	}
	if (meets == 1) {
		clif->message(fd, msg_txt(1171)); // The player meets all the requirements for that skill.
	}
	
	return true;
}

// Hand a ring with partners name on it to this char
void getring(struct map_session_data* sd) {
	int flag, item_id;
	struct item item_tmp;
	item_id = (sd->status.sex) ? WEDDING_RING_M : WEDDING_RING_F;
	
	memset(&item_tmp, 0, sizeof(item_tmp));
	item_tmp.nameid = item_id;
	item_tmp.identify = 1;
	item_tmp.card[0] = 255;
	item_tmp.card[2] = sd->status.partner_id;
	item_tmp.card[3] = sd->status.partner_id >> 16;
	
	if((flag = pc->additem(sd,&item_tmp,1,LOG_TYPE_COMMAND))) {
		clif->additem(sd,0,0,flag);
		map->addflooritem(&item_tmp,1,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
	}
}

/*==========================================
 * @marry by [MouseJstr], fixed by Lupus
 * Marry two players
 *------------------------------------------*/
ACMD(marry) {
	struct map_session_data *pl_sd = NULL;
	char player_name[NAME_LENGTH] = "";
		
	if (!message || !*message || sscanf(message, "%23s", player_name) != 1) {
		clif->message(fd, msg_txt(1172)); // Usage: @marry <char name>
		return false;
	}
	
	if ((pl_sd = map->nick2sd(player_name)) == NULL) {
		clif->message(fd, msg_txt(3));
		return false;
	}
	
	if (pc->marriage(sd, pl_sd) == 0) {
		clif->message(fd, msg_txt(1173)); // They are married... wish them well.
		clif->wedding_effect(&pl_sd->bl); //wedding effect and music [Lupus]
		getring(sd); // Auto-give named rings (Aru)
		getring(pl_sd);
		return true;
	}
	
	clif->message(fd, msg_txt(1174)); // The two cannot wed because one is either a baby or already married.
	return false;
}

/*==========================================
 * @divorce by [MouseJstr], fixed by [Lupus]
 * divorce two players
 *------------------------------------------*/
ACMD(divorce)
{
	if (pc->divorce(sd) != 0) {
		sprintf(atcmd_output, msg_txt(1175), sd->status.name); // '%s' is not married.
		clif->message(fd, atcmd_output);
		return false;
	}
	
	sprintf(atcmd_output, msg_txt(1176), sd->status.name); // '%s' and his/her partner are now divorced.
	clif->message(fd, atcmd_output);
	return true;
}

/*==========================================
 * @changelook by [Celest]
 *------------------------------------------*/
ACMD(changelook)
{
	int i, j = 0, k = 0;
	int pos[7] = { LOOK_HEAD_TOP,LOOK_HEAD_MID,LOOK_HEAD_BOTTOM,LOOK_WEAPON,LOOK_SHIELD,LOOK_SHOES,LOOK_ROBE };
	
	if((i = sscanf(message, "%d %d", &j, &k)) < 1) {
		clif->message(fd, msg_txt(1177)); // Usage: @changelook {<position>} <view id>
		clif->message(fd, msg_txt(1178)); // Position: 1-Top 2-Middle 3-Bottom 4-Weapon 5-Shield 6-Shoes 7-Robe
		return false;
	} else if ( i == 2 ) {
		if (j < 1 || j > 7)
			j = 1;
		j = pos[j - 1];
	} else if( i == 1 ) {	// position not defined, use HEAD_TOP as default
		k = j;	// swap
		j = LOOK_HEAD_TOP;
	}
	
	clif->changelook(&sd->bl,j,k);
	
	return true;
}

/*==========================================
 * @autotrade by durf [Lupus] [Paradox924X]
 * Turns on/off Autotrade for a specific player
 *------------------------------------------*/
ACMD(autotrade) {
	
	if( map->list[sd->bl.m].flag.autotrade != battle_config.autotrade_mapflag ) {
		clif->message(fd, msg_txt(1179)); // Autotrade is not allowed in this map.
		return false;
	}
	
	if( pc_isdead(sd) ) {
		clif->message(fd, msg_txt(1180)); // You cannot autotrade when dead.
		return false;
	}
	
	if( !sd->state.vending && !sd->state.buyingstore ) { //check if player is vending or buying
		clif->message(fd, msg_txt(549)); // "You should have a shop open in order to use @autotrade."
		return false;
	}
	
	sd->state.autotrade = 1;
	if( battle_config.at_timeout ) {
		int timeout = atoi(message);
		status->change_start(NULL,&sd->bl, SC_AUTOTRADE, 10000, 0, 0, 0, 0, ((timeout > 0) ? min(timeout,battle_config.at_timeout) : battle_config.at_timeout) * 60000, 0);
	}

	/* currently standalones are not supporting buyingstores, so we rely on the previous method */
	if( sd->state.buyingstore ) {
		clif->authfail_fd(fd, 15);
		return true;
	}

	
	clif->chsys_quit(sd);
	
	clif->authfail_fd(sd->fd, 15);
	
	
#ifdef AUTOTRADE_PERSISTENCY
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
ACMD(changegm) {
	struct guild *g;
	struct map_session_data *pl_sd;
	
	if (sd->status.guild_id == 0 || (g = sd->guild) == NULL || strcmp(g->master,sd->status.name)) {
		clif->message(fd, msg_txt(1181)); // You need to be a Guild Master to use this command.
		return false;
	}
	
	if( map->list[sd->bl.m].flag.guildlock || map->list[sd->bl.m].flag.gvg_castle ) {
		clif->message(fd, msg_txt(1182)); // You cannot change guild leaders in this map.
		return false;
	}
	
	if( !message[0] ) {
		clif->message(fd, msg_txt(1183)); // Usage: @changegm <guild_member_name>
		return false;
	}
	
	if((pl_sd=map->nick2sd((char *) message)) == NULL || pl_sd->status.guild_id != sd->status.guild_id) {
		clif->message(fd, msg_txt(1184)); // Target character must be online and be a guild member.
		return false;
	}
	
	guild->gm_change(sd->status.guild_id, pl_sd);
	return true;
}

/*==========================================
 * @changeleader by Skotlex
 * Changes the leader of a party.
 *------------------------------------------*/
ACMD(changeleader) {
	
	if( !message[0] ) {
		clif->message(fd, msg_txt(1185)); // Usage: @changeleader <party_member_name>
		return false;
	}
	
	if (party->changeleader(sd, map->nick2sd((char *) message)))
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
		clif->message(fd, msg_txt(282));
		return false;
	}
	
	ARR_FIND( 0, MAX_PARTY, mi, p->data[mi].sd == sd );
	if (mi == MAX_PARTY)
		return false; //Shouldn't happen
	
	if (!p->party.member[mi].leader)
	{
		clif->message(fd, msg_txt(282));
		return false;
	}
	
	if(!message || !*message || sscanf(message, "%15s %15s", w1, w2) < 2)
	{
		clif->message(fd, msg_txt(1186)); // Usage: @partyoption <pickup share: yes/no> <item distribution: yes/no>
		return false;
	}
	
	option = (config_switch(w1)?1:0)|(config_switch(w2)?2:0);
	
	//Change item share type.
	if (option != p->party.item)
		party->changeoption(sd, p->party.exp, option);
	else
		clif->message(fd, msg_txt(286));
	
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
	if(!message || !*message)
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
		snprintf(atcmd_output, sizeof atcmd_output, msg_txt(1187),((double)sd->state.autoloot)/100.); // Autolooting items with drop rates of %0.02f%% and below.
		clif->message(fd, atcmd_output);
	}else
		clif->message(fd, msg_txt(1188)); // Autoloot is now off.
	
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
	
	if (message && *message) {
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
				clif->message(fd, msg_txt(1189)); // Item not found.
				return false;
			}
		}
	}
	
	switch(action) {
		case 1:
			ARR_FIND(0, AUTOLOOTITEM_SIZE, i, sd->state.autolootid[i] == item_data->nameid);
			if (i != AUTOLOOTITEM_SIZE) {
				clif->message(fd, msg_txt(1190)); // You're already autolooting this item.
				return false;
			}
			ARR_FIND(0, AUTOLOOTITEM_SIZE, i, sd->state.autolootid[i] == 0);
			if (i == AUTOLOOTITEM_SIZE) {
				clif->message(fd, msg_txt(1191)); // Your autolootitem list is full. Remove some items first with @autolootid -<item name or ID>.
				return false;
			}
			sd->state.autolootid[i] = item_data->nameid; // Autoloot Activated
			sprintf(atcmd_output, msg_txt(1192), item_data->name, item_data->jname, item_data->nameid); // Autolooting item: '%s'/'%s' {%d}
			clif->message(fd, atcmd_output);
			sd->state.autolooting = 1;
			break;
		case 2:
			ARR_FIND(0, AUTOLOOTITEM_SIZE, i, sd->state.autolootid[i] == item_data->nameid);
			if (i == AUTOLOOTITEM_SIZE) {
				clif->message(fd, msg_txt(1193)); // You're currently not autolooting this item.
				return false;
			}
			sd->state.autolootid[i] = 0;
			sprintf(atcmd_output, msg_txt(1194), item_data->name, item_data->jname, item_data->nameid); // Removed item: '%s'/'%s' {%d} from your autolootitem list.
			clif->message(fd, atcmd_output);
			ARR_FIND(0, AUTOLOOTITEM_SIZE, i, sd->state.autolootid[i] != 0);
			if (i == AUTOLOOTITEM_SIZE) {
				sd->state.autolooting = 0;
			}
			break;
		case 3:
			sprintf(atcmd_output, msg_txt(1195), AUTOLOOTITEM_SIZE); // You can have %d items on your autolootitem list.
			clif->message(fd, atcmd_output);
			clif->message(fd, msg_txt(1196)); // To add an item to the list, use "@alootid +<item name or ID>". To remove an item, use "@alootid -<item name or ID>".
			clif->message(fd, msg_txt(1197)); // "@alootid reset" will clear your autolootitem list.
			ARR_FIND(0, AUTOLOOTITEM_SIZE, i, sd->state.autolootid[i] != 0);
			if (i == AUTOLOOTITEM_SIZE) {
				clif->message(fd, msg_txt(1198)); // Your autolootitem list is empty.
			} else {
				clif->message(fd, msg_txt(1199)); // Items on your autolootitem list:
				for(i = 0; i < AUTOLOOTITEM_SIZE; i++)
				{
					if (sd->state.autolootid[i] == 0)
						continue;
					if (!(item_data = itemdb->exists(sd->state.autolootid[i]))) {
						ShowDebug("Non-existant item %d on autolootitem list (account_id: %d, char_id: %d)", sd->state.autolootid[i], sd->status.account_id, sd->status.char_id);
						continue;
					}
					sprintf(atcmd_output, "'%s'/'%s' {%d}", item_data->name, item_data->jname, item_data->nameid);
					clif->message(fd, atcmd_output);
				}
			}
			break;
		case 4:
			memset(sd->state.autolootid, 0, sizeof(sd->state.autolootid));
			clif->message(fd, msg_txt(1200)); // Your autolootitem list has been reset.
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
ACMD(autoloottype) {
	int i;
	uint8 action = 3; // 1=add, 2=remove, 3=help+list (default), 4=reset
	enum item_types type = -1;
	int ITEM_NONE = 0;

	if (message && *message) {
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
				clif->message(fd, msg_txt(1491)); // Item type not found.
				return false;
			}
		}
	}

	switch (action) {
		case 1:
			if (sd->state.autoloottype&(1<<type)) {
				clif->message(fd, msg_txt(1490)); // You're already autolooting this item type.
				return false;
			}
			sd->state.autoloottype |= (1<<type); // Stores the type
			sprintf(atcmd_output, msg_txt(1492), itemdb->typename(type)); // Autolooting item type: '%s'
			clif->message(fd, atcmd_output);
			break;
		case 2:
			if (!(sd->state.autoloottype&(1<<type))) {
				clif->message(fd, msg_txt(1493)); // You're currently not autolooting this item type.
				return false;
			}
			sd->state.autoloottype &= ~(1<<type);
			sprintf(atcmd_output, msg_txt(1494), itemdb->typename(type)); // Removed item type: '%s' from your autoloottype list.
			clif->message(fd, atcmd_output);
			break;
		case 3:
			clif->message(fd, msg_txt(38)); // Invalid location number, or name.

			{
				// attempt to find the text help string
				const char *text = atcommand_help_string(info);
				if (text) clif->messageln(fd, text); // send the text to the client
			}

			if (sd->state.autoloottype == ITEM_NONE) {
				clif->message(fd, msg_txt(1495)); // Your autoloottype list is empty.
			} else {
				clif->message(fd, msg_txt(1496)); // Item types on your autoloottype list:
				for(i=0; i < IT_MAX; i++) {
					if (sd->state.autoloottype&(1<<i)) {
						sprintf(atcmd_output, " '%s'", itemdb->typename(i));
						clif->message(fd, atcmd_output);
					}
				}
			}
			break;
		case 4:
			sd->state.autoloottype = ITEM_NONE;
			clif->message(fd, msg_txt(1497)); // Your autoloottype list has been reset.
			break;
	}
	return true;
}

/*==========================================
 * It is made to snow.
 *------------------------------------------*/
ACMD(snow) {
	
	if (map->list[sd->bl.m].flag.snow) {
		map->list[sd->bl.m].flag.snow=0;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_txt(1203)); // Snow has stopped falling.
	} else {
		map->list[sd->bl.m].flag.snow=1;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_txt(1204)); // It has started to snow.
	}
	
	return true;
}

/*==========================================
 * Cherry tree snowstorm is made to fall. (Sakura)
 *------------------------------------------*/
ACMD(sakura) {

	if (map->list[sd->bl.m].flag.sakura) {
		map->list[sd->bl.m].flag.sakura=0;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_txt(1205)); // Cherry tree leaves no longer fall.
	} else {
		map->list[sd->bl.m].flag.sakura=1;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_txt(1206)); // Cherry tree leaves have begun to fall.
	}
	return true;
}

/*==========================================
 * Clouds appear.
 *------------------------------------------*/
ACMD(clouds) {

	if (map->list[sd->bl.m].flag.clouds) {
		map->list[sd->bl.m].flag.clouds=0;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_txt(1207)); // The clouds has disappear.
	} else {
		map->list[sd->bl.m].flag.clouds=1;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_txt(1208)); // Clouds appear.
	}
	
	return true;
}

/*==========================================
 * Different type of clouds using effect 516
 *------------------------------------------*/
ACMD(clouds2) {

	if (map->list[sd->bl.m].flag.clouds2) {
		map->list[sd->bl.m].flag.clouds2=0;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_txt(1209)); // The alternative clouds disappear.
	} else {
		map->list[sd->bl.m].flag.clouds2=1;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_txt(1210)); // Alternative clouds appear.
	}
	
	return true;
}

/*==========================================
 * Fog hangs over.
 *------------------------------------------*/
ACMD(fog) {

	if (map->list[sd->bl.m].flag.fog) {
		map->list[sd->bl.m].flag.fog=0;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_txt(1211)); // The fog has gone.
	} else {
		map->list[sd->bl.m].flag.fog=1;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_txt(1212)); // Fog hangs over.
	}
	return true;
}

/*==========================================
 * Fallen leaves fall.
 *------------------------------------------*/
ACMD(leaves) {

	if (map->list[sd->bl.m].flag.leaves) {
		map->list[sd->bl.m].flag.leaves=0;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_txt(1213)); // Leaves no longer fall.
	} else {
		map->list[sd->bl.m].flag.leaves=1;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_txt(1214)); // Fallen leaves fall.
	}
	
	return true;
}

/*==========================================
 * Fireworks appear.
 *------------------------------------------*/
ACMD(fireworks) {

	if (map->list[sd->bl.m].flag.fireworks) {
		map->list[sd->bl.m].flag.fireworks=0;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_txt(1215)); // Fireworks have ended.
	} else {
		map->list[sd->bl.m].flag.fireworks=1;
		clif->weather(sd->bl.m);
		clif->message(fd, msg_txt(1216)); // Fireworks have launched.
	}
	
	return true;
}

/*==========================================
 * Clearing Weather Effects by Dexity
 *------------------------------------------*/
ACMD(clearweather) {

	map->list[sd->bl.m].flag.snow=0;
	map->list[sd->bl.m].flag.sakura=0;
	map->list[sd->bl.m].flag.clouds=0;
	map->list[sd->bl.m].flag.clouds2=0;
	map->list[sd->bl.m].flag.fog=0;
	map->list[sd->bl.m].flag.fireworks=0;
	map->list[sd->bl.m].flag.leaves=0;
	clif->weather(sd->bl.m);
	clif->message(fd, msg_txt(291)); // "Weather effects will disappear after teleporting or refreshing."
	
	return true;
}

/*===============================================================
 * Sound Command - plays a sound for everyone around! [Codemaster]
 *---------------------------------------------------------------*/
ACMD(sound)
{
	char sound_file[100];
	
	memset(sound_file, '\0', sizeof(sound_file));
	
	if(!message || !*message || sscanf(message, "%99[^\n]", sound_file) < 1) {
		clif->message(fd, msg_txt(1217)); // Please enter a sound filename (usage: @sound <filename>).
		return false;
	}
	
	if(strstr(sound_file, ".wav") == NULL)
		strcat(sound_file, ".wav");
	
	clif->soundeffectall(&sd->bl, sound_file, 0, AREA);
	
	return true;
}

/*==========================================
 * 	MOB Search
 *------------------------------------------*/
ACMD(mobsearch)
{
	char mob_name[100];
	int mob_id;
	int number = 0;
	struct s_mapiterator* it;
		
	if (!message || !*message || sscanf(message, "%99[^\n]", mob_name) < 1) {
		clif->message(fd, msg_txt(1218)); // Please enter a monster name (usage: @mobsearch <monster name>).
		return false;
	}
	
	if ((mob_id = atoi(mob_name)) == 0)
		mob_id = mob->db_searchname(mob_name);
	if(mob_id > 0 && mob->db_checkid(mob_id) == 0){
		snprintf(atcmd_output, sizeof atcmd_output, msg_txt(1219),mob_name); // Invalid mob ID %s!
		clif->message(fd, atcmd_output);
		return false;
	}
	if(mob_id == atoi(mob_name) && mob->db(mob_id)->jname)
		strcpy(mob_name,mob->db(mob_id)->jname);	// --ja--
	//				strcpy(mob_name,mob_db(mob_id)->name);	// --en--
	
	snprintf(atcmd_output, sizeof atcmd_output, msg_txt(1220), mob_name, mapindex_id2name(sd->mapindex)); // Mob Search... %s %s
	clif->message(fd, atcmd_output);
	
	it = mapit_geteachmob();
	for(;;)
	{
		TBL_MOB* md = (TBL_MOB*)mapit->next(it);
		if( md == NULL )
			break;// no more mobs
		
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
int atcommand_cleanfloor_sub(struct block_list *bl, va_list ap) {
	nullpo_ret(bl);
	map->clearflooritem(bl);
	
	return 0;
}

ACMD(cleanmap) {
	map->foreachinmap(atcommand->cleanfloor_sub, sd->bl.m, BL_ITEM);
	clif->message(fd, msg_txt(1221)); // All dropped items have been cleaned up.
	return true;
}

ACMD(cleanarea) {
	int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
	
	if (!message || !*message || sscanf(message, "%d %d %d %d", &x0, &y0, &x1, &y1) < 1) {
		map->foreachinrange(atcommand->cleanfloor_sub, &sd->bl, AREA_SIZE * 2, BL_ITEM);
	} else if (sscanf(message, "%d %d %d %d", &x0, &y0, &x1, &y1) == 1) {
		map->foreachinrange(atcommand->cleanfloor_sub, &sd->bl, x0, BL_ITEM);
	} else if (sscanf(message, "%d %d %d %d", &x0, &y0, &x1, &y1) == 4) {
		map->foreachinarea(atcommand->cleanfloor_sub, sd->bl.m, x0, y0, x1, y1, BL_ITEM);
	}
	
	clif->message(fd, msg_txt(1221)); // All dropped items have been cleaned up.
	return true;
}

/*==========================================
 * make a NPC/PET talk
 * @npctalkc [SnakeDrak]
 *------------------------------------------*/
ACMD(npctalk)
{
	char name[NAME_LENGTH],mes[100],temp[100];
	struct npc_data *nd;
	bool ifcolor=(*(info->command + 7) != 'c' && *(info->command + 7) != 'C')?0:1;
	unsigned int color = 0;
	
	if (sd->sc.count && //no "chatting" while muted.
		(sd->sc.data[SC_BERSERK] || (sd->sc.data[SC_DEEP_SLEEP] && sd->sc.data[SC_DEEP_SLEEP]->val2) ||
		 (sd->sc.data[SC_NOCHAT] && sd->sc.data[SC_NOCHAT]->val1&MANNER_NOCHAT)))
		return false;
	
	if(!ifcolor) {
		if (!message || !*message || sscanf(message, "%23[^,], %99[^\n]", name, mes) < 2) {
			clif->message(fd, msg_txt(1222)); // Please enter the correct parameters (usage: @npctalk <npc name>, <message>).
			return false;
		}
	}
	else {
		if (!message || !*message || sscanf(message, "%u %23[^,], %99[^\n]", &color, name, mes) < 3) {
			clif->message(fd, msg_txt(1223)); // Please enter the correct parameters (usage: @npctalkc <color> <npc name>, <message>).
			return false;
		}
	}
	
	if (!(nd = npc->name2id(name))) {
		clif->message(fd, msg_txt(111)); // This NPC doesn't exist
		return false;
	}
	
	strtok(name, "#"); // discard extra name identifier if present
	snprintf(temp, sizeof(temp), "%s : %s", name, mes);
	
	if(ifcolor) clif->messagecolor(&nd->bl,color,temp);
	else clif->disp_overhead(&nd->bl, temp);
	
	return true;
}

ACMD(pettalk)
{
	char mes[100],temp[100];
	struct pet_data *pd;
		
	if ( battle_config.min_chat_delay ) {
		if( DIFF_TICK(sd->cantalk_tick, timer->gettick()) > 0 )
			return true;
		sd->cantalk_tick = timer->gettick() + battle_config.min_chat_delay;
	}
	
	if(!sd->status.pet_id || !(pd=sd->pd))
	{
		clif->message(fd, msg_txt(184));
		return false;
	}
	
	if (sd->sc.count && //no "chatting" while muted.
		(sd->sc.data[SC_BERSERK] || (sd->sc.data[SC_DEEP_SLEEP] && sd->sc.data[SC_DEEP_SLEEP]->val2) ||
		 (sd->sc.data[SC_NOCHAT] && sd->sc.data[SC_NOCHAT]->val1&MANNER_NOCHAT)))
		return false;
	
	if (!message || !*message || sscanf(message, "%99[^\n]", mes) < 1) {
		clif->message(fd, msg_txt(1224)); // Please enter a message (usage: @pettalk <message>).
		return false;
	}
	
	if (message[0] == '/')
	{// pet emotion processing
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
	clif->disp_overhead(&pd->bl, temp);
	
	return true;
}

/// @users - displays the number of players present on each map (and percentage)
/// #users displays on the target user instead of self
ACMD(users)
{
	char buf[CHAT_SIZE_MAX];
	int i;
	int users[MAX_MAPINDEX];
	int users_all;
	struct s_mapiterator* iter;
	
	memset(users, 0, sizeof(users));
	users_all = 0;
	
	// count users on each map
	iter = mapit_getallusers();
	for(;;)
	{
		struct map_session_data* sd2 = (struct map_session_data*)mapit->next(iter);
		if( sd2 == NULL )
			break;// no more users
		
		if( sd2->mapindex >= MAX_MAPINDEX )
			continue;// invalid mapindex
		
		if( users[sd2->mapindex] < INT_MAX ) ++users[sd2->mapindex];
		if( users_all < INT_MAX ) ++users_all;
	}
	mapit->free(iter);
	
	// display results for each map
	for( i = 0; i < MAX_MAPINDEX; ++i )
	{
		if( users[i] == 0 )
			continue;// empty
		
		safesnprintf(buf, sizeof(buf), "%s: %d (%.2f%%)", mapindex_id2name(i), users[i], (float)(100.0f*users[i]/users_all));
		clif->message(sd->fd, buf);
	}
	
	// display overall count
	safesnprintf(buf, sizeof(buf), "all: %d", users_all);
	clif->message(sd->fd, buf);
	
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(reset)
{
	pc->resetstate(sd);
	pc->resetskill(sd,1);
	sprintf(atcmd_output, msg_txt(208), sd->status.name); // '%s' skill and stats points reseted!
	clif->message(fd, atcmd_output);
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
ACMD(summon)
{
	char name[NAME_LENGTH];
	int mob_id = 0;
	int duration = 0;
	struct mob_data *md;
	int64 tick=timer->gettick();
	
	if (!message || !*message || sscanf(message, "%23s %d", name, &duration) < 1)
	{
		clif->message(fd, msg_txt(1225)); // Please enter a monster name (usage: @summon <monster name> {duration}).
		return false;
	}
	
	if (duration < 1)
		duration =1;
	else if (duration > 60)
		duration =60;
	
	if ((mob_id = atoi(name)) == 0)
		mob_id = mob->db_searchname(name);
	if(mob_id == 0 || mob->db_checkid(mob_id) == 0)
	{
		clif->message(fd, msg_txt(40));	// Invalid monster ID or name.
		return false;
	}
	
	md = mob->once_spawn_sub(&sd->bl, sd->bl.m, -1, -1, "--ja--", mob_id, "", SZ_MEDIUM, AI_NONE);
	
	if(!md)
		return false;
	
	md->master_id=sd->bl.id;
	md->special_state.ai=1;
	md->deletetimer=timer->add(tick+(duration*60000),mob->timer_delete,md->bl.id,0);
	clif->specialeffect(&md->bl,344,AREA);
	mob->spawn(md);
	sc_start4(NULL,&md->bl, SC_MODECHANGE, 100, 1, 0, MD_AGGRESSIVE, 0, 60000);
	clif->skill_poseffect(&sd->bl,AM_CALLHOMUN,1,md->bl.x,md->bl.y,tick);
	clif->message(fd, msg_txt(39));	// All monster summoned!
	
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
	
	if (!message || !*message || sscanf(message, "%d", &new_group) != 1) {
		clif->message(fd, msg_txt(1226)); // Usage: @adjgroup <group_id>
		return false;
	}
	
	if (pc->set_group(sd, new_group) != 0) {
		clif->message(fd, msg_txt(1227)); // Specified group does not exist.
		return false;
	}

	clif->message(fd, msg_txt(1228)); // Group changed successfully.
	clif->message(sd->fd, msg_txt(1229)); // Your group has changed.
	return true;
}

/*==========================================
 * @trade by [MouseJstr]
 * Open a trade window with a remote player
 *------------------------------------------*/
ACMD(trade) {
    struct map_session_data *pl_sd = NULL;
	
	if (!message || !*message) {
		clif->message(fd, msg_txt(1230)); // Please enter a player name (usage: @trade <char name>).
		return false;
	}
	
	if ( (pl_sd = map->nick2sd((char *)message)) == NULL ) {
		clif->message(fd, msg_txt(3)); // Character not found.
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
	
	if (!message || !*message || sscanf(message, "%127s %127s", flag, value) != 2) {
		clif->message(fd, msg_txt(1231)); // Usage: @setbattleflag <flag> <value>
		return false;
	}
	
	if (battle->config_set_value(flag, value) == 0) {
		clif->message(fd, msg_txt(1232)); // Unknown battle_config flag.
		return false;
	}
	
	clif->message(fd, msg_txt(1233)); // Set battle_config as requested.
	
	return true;
}

/*==========================================
 * @unmute [Valaris]
 *------------------------------------------*/
ACMD(unmute) {
	struct map_session_data *pl_sd = NULL;
	
	if (!message || !*message) {
		clif->message(fd, msg_txt(1234)); // Please enter a player name (usage: @unmute <char name>).
		return false;
	}
	
	if ( (pl_sd = map->nick2sd((char *)message)) == NULL )
	{
		clif->message(fd, msg_txt(3)); // Character not found.
		return false;
	}
	
	if(!pl_sd->sc.data[SC_NOCHAT]) {
		clif->message(sd->fd,msg_txt(1235)); // Player is not muted.
		return false;
	}
	
	pl_sd->status.manner = 0;
	status_change_end(&pl_sd->bl, SC_NOCHAT, INVALID_TIMER);
	clif->message(sd->fd,msg_txt(1236)); // Player unmuted.
	
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
	
	snprintf(atcmd_output, sizeof(atcmd_output), msg_txt(245), days, hours, minutes, seconds);
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

	pc->resetskill(sd,4);
	// to avoid any problem with equipment and invalid sex, equipment is unequiped.
	for( i=0; i<EQI_MAX; i++ )
		if( sd->equip_index[i] >= 0 ) pc->unequipitem(sd, sd->equip_index[i], 3);
	chrif->changesex(sd);
	return true;
}

/*================================================
 * @mute - Mutes a player for a set amount of time
 *------------------------------------------------*/
ACMD(mute) {
	struct map_session_data *pl_sd = NULL;
	int manner;
	
	if (!message || !*message || sscanf(message, "%d %23[^\n]", &manner, atcmd_player_name) < 1) {
		clif->message(fd, msg_txt(1237)); // Usage: @mute <time> <char name>
		return false;
	}
	
	if ( (pl_sd = map->nick2sd(atcmd_player_name)) == NULL ) {
		clif->message(fd, msg_txt(3)); // Character not found.
		return false;
	}
	
	if ( pc_get_group_level(sd) < pc_get_group_level(pl_sd) )
	{
		clif->message(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return false;
	}
	
	clif->manner_message(sd, 0);
	clif->manner_message(pl_sd, 5);
	
	if( pl_sd->status.manner < manner ) {
		pl_sd->status.manner -= manner;
		sc_start(NULL,&pl_sd->bl,SC_NOCHAT,100,0,0);
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
	clif->refresh(sd);
	return true;
}

ACMD(refreshall)
{
	struct map_session_data* iter_sd;
	struct s_mapiterator* iter;
	
	iter = mapit_getallusers();
	for (iter_sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); iter_sd = (TBL_PC*)mapit->next(iter))
		clif->refresh(iter_sd);
	mapit->free(iter);
	return true;
}

/*==========================================
 * @identify
 * => GM's magnifier.
 *------------------------------------------*/
ACMD(identify)
{
	int i,num;
		
	for(i=num=0;i<MAX_INVENTORY;i++){
		if(sd->status.inventory[i].nameid > 0 && sd->status.inventory[i].identify!=1){
			num++;
		}
	}
	if (num > 0) {
		clif->item_identify_list(sd);
	} else {
		clif->message(fd,msg_txt(1238)); // There are no items to appraise.
	}
	return true;
}

ACMD(misceffect) {
	int effect = 0;

	if (!message || !*message)
		return false;
	if (sscanf(message, "%d", &effect) < 1)
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
	int i, j, k;
	
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(atcmd_output2, '\0', sizeof(atcmd_output2));
	
	if (!message || !*message) {
		clif->message(fd, msg_txt(1239)); // Please enter a monster name/ID (usage: @mobinfo <monster_name_or_monster_ID>).
		return false;
	}
	
	// If monster identifier/name argument is a name
	if ((i = mob->db_checkid(atoi(message)))) {
		mob_array[0] = mob->db(i);
		count = 1;
	} else
		count = mob->db_searchname_array(mob_array, MAX_SEARCH, message, 0);
	
	if (!count) {
		clif->message(fd, msg_txt(40)); // Invalid monster ID or name.
		return false;
	}
	
	if (count > MAX_SEARCH) {
		sprintf(atcmd_output, msg_txt(269), MAX_SEARCH, count);
		clif->message(fd, atcmd_output);
		count = MAX_SEARCH;
	}
	
	for (k = 0; k < count; k++) {
		unsigned int job_exp, base_exp;
		
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
			sprintf(atcmd_output, msg_txt(1240), monster->name, monster->jname, monster->sprite, monster->vd.class_); // MVP Monster: '%s'/'%s'/'%s' (%d)
		else
			sprintf(atcmd_output, msg_txt(1241), monster->name, monster->jname, monster->sprite, monster->vd.class_); // Monster: '%s'/'%s'/'%s' (%d)
		clif->message(fd, atcmd_output);
		
		sprintf(atcmd_output, msg_txt(1242), monster->lv, monster->status.max_hp, base_exp, job_exp, MOB_HIT(monster), MOB_FLEE(monster)); //  Lv:%d  HP:%d  Base EXP:%u  Job EXP:%u  HIT:%d  FLEE:%d
		clif->message(fd, atcmd_output);
		
		sprintf(atcmd_output, msg_txt(1243), //  DEF:%d  MDEF:%d  STR:%d  AGI:%d  VIT:%d  INT:%d  DEX:%d  LUK:%d
				monster->status.def, monster->status.mdef, monster->status.str, monster->status.agi,
				monster->status.vit, monster->status.int_, monster->status.dex, monster->status.luk);
		clif->message(fd, atcmd_output);
		
		sprintf(atcmd_output, msg_txt(1244), //  ATK:%d~%d  Range:%d~%d~%d  Size:%s  Race: %s  Element: %s (Lv:%d)
				monster->status.rhw.atk, monster->status.rhw.atk2, monster->status.rhw.range,
				monster->range2 , monster->range3, msize[monster->status.size],
				mrace[monster->status.race], melement[monster->status.def_ele], monster->status.ele_lv);
		clif->message(fd, atcmd_output);
		
		// drops
		clif->message(fd, msg_txt(1245)); //  Drops:
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
				sprintf(atcmd_output2, " - %s[%d]  %02.02f%%", item_data->jname, item_data->slot, (float)droprate / 100);
			else
				sprintf(atcmd_output2, " - %s  %02.02f%%", item_data->jname, (float)droprate / 100);
			
			strcat(atcmd_output, atcmd_output2);
			
			if (++j % 3 == 0) {
				clif->message(fd, atcmd_output);
				strcpy(atcmd_output, " ");
			}
		}
		
		if (j == 0)
			clif->message(fd, msg_txt(1246)); // This monster has no drops.
		else if (j % 3 != 0)
			clif->message(fd, atcmd_output);
		// mvp
		if (monster->mexp) {
			sprintf(atcmd_output, msg_txt(1247), monster->mexp); //  MVP Bonus EXP:%u
			clif->message(fd, atcmd_output);
			
			strcpy(atcmd_output, msg_txt(1248)); //  MVP Items:
			j = 0;
			for (i = 0; i < MAX_MVP_DROP; i++) {
				if (monster->mvpitem[i].nameid <= 0 || (item_data = itemdb->exists(monster->mvpitem[i].nameid)) == NULL)
					continue;
				if (monster->mvpitem[i].p > 0) {
					j++;
					if(item_data->slot)
						sprintf(atcmd_output2, " %s%s[%d]  %02.02f%%",j != 1 ? "- " : "", item_data->jname, item_data->slot, (float)monster->mvpitem[i].p / 100);
					else
						sprintf(atcmd_output2, " %s%s  %02.02f%%",j != 1 ? "- " : "", item_data->jname, (float)monster->mvpitem[i].p / 100);
					strcat(atcmd_output, atcmd_output2);
				}
			}
			if (j == 0)
				clif->message(fd, msg_txt(1249)); // This monster has no MVP prizes.
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
	struct s_mapiterator* it;
		
	if( sscanf(message, "%99[^\n]", mob_name) < 0 ) {
		clif->message(fd, msg_txt(546)); // Please enter a mob name/id (usage: @showmobs <mob name/id>)
		return false;
	}

	if( (mob_id = atoi(mob_name)) == 0 )
		mob_id = mob->db_searchname(mob_name);

	if( mob_id == 0 ) {
		snprintf(atcmd_output, sizeof atcmd_output, msg_txt(547), mob_name); // Invalid mob name %s!
		clif->message(fd, atcmd_output);
		return false;
	}

	if(mob_id > 0 && mob->db_checkid(mob_id) == 0){
		snprintf(atcmd_output, sizeof atcmd_output, msg_txt(1250),mob_name); // Invalid mob id %s!
		clif->message(fd, atcmd_output);
		return false;
	}
	
	if(mob->db(mob_id)->status.mode&MD_BOSS && !pc_has_permission(sd, PC_PERM_SHOW_BOSS)){	// If player group does not have access to boss mobs.
		clif->message(fd, msg_txt(1251)); // Can't show boss mobs!
		return false;
	}
	
	if(mob_id == atoi(mob_name) && mob->db(mob_id)->jname)
		strcpy(mob_name,mob->db(mob_id)->jname);    // --ja--
	//strcpy(mob_name,mob_db(mob_id)->name);    // --en--
	
	snprintf(atcmd_output, sizeof atcmd_output, msg_txt(1252), // Mob Search... %s %s
			 mob_name, mapindex_id2name(sd->mapindex));
	clif->message(fd, atcmd_output);
	
	it = mapit_geteachmob();
	for(;;)
	{
		TBL_MOB* md = (TBL_MOB*)mapit->next(it);
		if( md == NULL )
			break;// no more mobs
		
		if( md->bl.m != sd->bl.m )
			continue;
		if( mob_id != -1 && md->class_ != mob_id )
			continue;
		if( md->special_state.ai || md->master_id )
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
ACMD(homlevel) {
	TBL_HOM * hd;
	int level = 0;
	enum homun_type htype;
	
	if( !message || !*message || ( level = atoi(message) ) < 1 ) {
		clif->message(fd, msg_txt(1253)); // Please enter a level adjustment (usage: @homlevel <number of levels>).
		return false;
	}
	
	if( !homun_alive(sd->hd) ) {
		clif->message(fd, msg_txt(1254)); // You do not have a homunculus.
		return false;
	}
	
	hd = sd->hd;
		
	if( (htype = homun->class2type(hd->homunculus.class_)) == HT_INVALID ) {
		ShowError("atcommand_homlevel: invalid homun class %d (player %s)\n", hd->homunculus.class_,sd->status.name);
		return false;
	}

	switch( htype ) {
		case HT_REG:
		case HT_EVO:
			if( hd->homunculus.level >= battle_config.hom_max_level ) {
				snprintf(atcmd_output, sizeof(atcmd_output), msg_txt(1478), hd->homunculus.level); // Homun reached its maximum level of '%d'
				clif->message(fd, atcmd_output);
				return true;
			}
			break;
		case HT_S:
			if( hd->homunculus.level >= battle_config.hom_S_max_level ) {
				snprintf(atcmd_output, sizeof(atcmd_output), msg_txt(1478), hd->homunculus.level); // Homun reached its maximum level of '%d'
				clif->message(fd, atcmd_output);
				return true;
			}
			break;
		default:
			ShowError("atcommand_homlevel: unknown htype '%d'\n",htype);
			return false;
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
		clif->message(fd, msg_txt(1254)); // You do not have a homunculus.
		return false;
	}
	
	if ( !homun->evolve(sd->hd) ) {
		clif->message(fd, msg_txt(1255)); // Your homunculus doesn't evolve.
		return false;
	}
	clif->homskillinfoblock(sd);
	return true;
}

ACMD(hommutate) {
	int homun_id;
	enum homun_type m_class, m_id;
	
	if( !homun_alive(sd->hd) ) {
		clif->message(fd, msg_txt(1254)); // You do not have a homunculus.
		return false;
	}
	
	if( !message || !*message ) {
		homun_id = 6048 + (rnd() % 4);
	} else {
		homun_id = atoi(message);
	}
	
	m_class = homun->class2type(sd->hd->homunculus.class_);
	m_id	= homun->class2type(homun_id);
	
	if( m_class != HT_INVALID && m_id != HT_INVALID && m_class == HT_EVO && m_id == HT_S && sd->hd->homunculus.level >= 99 ) {
		homun->mutate(sd->hd, homun_id);
	} else {
		clif->emotion(&sd->hd->bl, E_SWT);
	}
	return true;
}

/*==========================================
 * call choosen homunculus [orn]
 *------------------------------------------*/
ACMD(makehomun) {
	int homunid;
		
	if (!message || !*message) {
		clif->message(fd, msg_txt(1256)); // Please enter a homunculus ID (usage: @makehomun <homunculus id>).
		return false;
	}
	
	homunid = atoi(message);
	
	if( homunid == -1 && sd->status.hom_id && !(sd->hd && homun_alive(sd->hd)) ) {
		if( !sd->hd )
			homun->call(sd);
		else if( sd->hd->homunculus.vaporize )
			homun->ressurect(sd, 100, sd->bl.x, sd->bl.y);
		else
			homun->call(sd);
		return true;
	}
		
	if ( sd->status.hom_id ) {
		clif->message(fd, msg_txt(450));
		return false;
	}
	
	if( homunid < HM_CLASS_BASE || homunid > HM_CLASS_BASE + MAX_HOMUNCULUS_CLASS - 1 )
	{
		clif->message(fd, msg_txt(1257)); // Invalid Homunculus ID.
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
		
	if ( !homun_alive(sd->hd) ) {
		clif->message(fd, msg_txt(1254)); // You do not have a homunculus.
		return false;
	}
	
	if (!message || !*message) {
		clif->message(fd, msg_txt(1258)); // Please enter a friendly value (usage: @homfriendly <friendly value [0-1000]>).
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
		
	if ( !homun_alive(sd->hd) ) {
		clif->message(fd, msg_txt(1254)); // You do not have a homunculus.
		return false;
	}
	
	if (!message || !*message) {
		clif->message(fd, msg_txt(1259)); // Please enter a hunger value (usage: @homhungry <hunger value [0-100]>).
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
	char mes[100],temp[100];
		
	if ( battle_config.min_chat_delay ) {
		if( DIFF_TICK(sd->cantalk_tick, timer->gettick()) > 0 )
			return true;
		sd->cantalk_tick = timer->gettick() + battle_config.min_chat_delay;
	}
	
	if (sd->sc.count && //no "chatting" while muted.
		(sd->sc.data[SC_BERSERK] || (sd->sc.data[SC_DEEP_SLEEP] && sd->sc.data[SC_DEEP_SLEEP]->val2) ||
		 (sd->sc.data[SC_NOCHAT] && sd->sc.data[SC_NOCHAT]->val1&MANNER_NOCHAT)))
		return false;
	
	if ( !homun_alive(sd->hd) ) {
		clif->message(fd, msg_txt(1254)); // You do not have a homunculus.
		return false;
	}
	
	if (!message || !*message || sscanf(message, "%99[^\n]", mes) < 1) {
		clif->message(fd, msg_txt(1260)); // Please enter a message (usage: @homtalk <message>).
		return false;
	}
	
	snprintf(temp, sizeof temp ,"%s : %s", sd->hd->homunculus.name, mes);
	clif->disp_overhead(&sd->hd->bl, temp);
	
	return true;
}

/*==========================================
 * Show homunculus stats
 *------------------------------------------*/
ACMD(hominfo) {
	struct homun_data *hd;
	struct status_data *st;
	
	if ( !homun_alive(sd->hd) ) {
		clif->message(fd, msg_txt(1254)); // You do not have a homunculus.
		return false;
	}
	
	hd = sd->hd;
	st = status->get_status_data(&hd->bl);
	clif->message(fd, msg_txt(1261)); // Homunculus stats:
	
	snprintf(atcmd_output, sizeof(atcmd_output) ,msg_txt(1262), // HP: %d/%d - SP: %d/%d
			 st->hp, st->max_hp, st->sp, st->max_sp);
	clif->message(fd, atcmd_output);
	
	snprintf(atcmd_output, sizeof(atcmd_output) ,msg_txt(1263), // ATK: %d - MATK: %d~%d
			 st->rhw.atk2 +st->batk, st->matk_min, st->matk_max);
	clif->message(fd, atcmd_output);
	
	snprintf(atcmd_output, sizeof(atcmd_output) ,msg_txt(1264), // Hungry: %d - Intimacy: %u
			 hd->homunculus.hunger, hd->homunculus.intimacy/100);
	clif->message(fd, atcmd_output);
	
	snprintf(atcmd_output, sizeof(atcmd_output) ,
			 msg_txt(1265), // Stats: Str %d / Agi %d / Vit %d / Int %d / Dex %d / Luk %d
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
		
	if ( !homun_alive(sd->hd) ) {
		clif->message(fd, msg_txt(1254)); // You do not have a homunculus.
		return false;
	}
	
	hd = sd->hd;
	
	hom = &hd->homunculus;
	db = hd->homunculusDB;
	lv = hom->level;
	
	snprintf(atcmd_output, sizeof(atcmd_output) ,
			 msg_txt(1266), lv, db->name); // Homunculus growth stats (Lv %d %s):
	clif->message(fd, atcmd_output);
	lv--; //Since the first increase is at level 2.
	
	evo = (hom->class_ == db->evo_class);
	min = db->base.HP +lv*db->gmin.HP +(evo?db->emin.HP:0);
	max = db->base.HP +lv*db->gmax.HP +(evo?db->emax.HP:0);;
	snprintf(atcmd_output, sizeof(atcmd_output) ,msg_txt(1267), hom->max_hp, min, max); // Max HP: %d (%d~%d)
	clif->message(fd, atcmd_output);
	
	min = db->base.SP +lv*db->gmin.SP +(evo?db->emin.SP:0);
	max = db->base.SP +lv*db->gmax.SP +(evo?db->emax.SP:0);;
	snprintf(atcmd_output, sizeof(atcmd_output) ,msg_txt(1268), hom->max_sp, min, max); // Max SP: %d (%d~%d)
	clif->message(fd, atcmd_output);
	
	min = db->base.str +lv*(db->gmin.str/10) +(evo?db->emin.str:0);
	max = db->base.str +lv*(db->gmax.str/10) +(evo?db->emax.str:0);;
	snprintf(atcmd_output, sizeof(atcmd_output) ,msg_txt(1269), hom->str/10, min, max); // Str: %d (%d~%d)
	clif->message(fd, atcmd_output);
	
	min = db->base.agi +lv*(db->gmin.agi/10) +(evo?db->emin.agi:0);
	max = db->base.agi +lv*(db->gmax.agi/10) +(evo?db->emax.agi:0);;
	snprintf(atcmd_output, sizeof(atcmd_output) ,msg_txt(1270), hom->agi/10, min, max); // Agi: %d (%d~%d)
	clif->message(fd, atcmd_output);
	
	min = db->base.vit +lv*(db->gmin.vit/10) +(evo?db->emin.vit:0);
	max = db->base.vit +lv*(db->gmax.vit/10) +(evo?db->emax.vit:0);;
	snprintf(atcmd_output, sizeof(atcmd_output) ,msg_txt(1271), hom->vit/10, min, max); // Vit: %d (%d~%d)
	clif->message(fd, atcmd_output);
	
	min = db->base.int_ +lv*(db->gmin.int_/10) +(evo?db->emin.int_:0);
	max = db->base.int_ +lv*(db->gmax.int_/10) +(evo?db->emax.int_:0);;
	snprintf(atcmd_output, sizeof(atcmd_output) ,msg_txt(1272), hom->int_/10, min, max); // Int: %d (%d~%d)
	clif->message(fd, atcmd_output);
	
	min = db->base.dex +lv*(db->gmin.dex/10) +(evo?db->emin.dex:0);
	max = db->base.dex +lv*(db->gmax.dex/10) +(evo?db->emax.dex:0);;
	snprintf(atcmd_output, sizeof(atcmd_output) ,msg_txt(1273), hom->dex/10, min, max); // Dex: %d (%d~%d)
	clif->message(fd, atcmd_output);
	
	min = db->base.luk +lv*(db->gmin.luk/10) +(evo?db->emin.luk:0);
	max = db->base.luk +lv*(db->gmax.luk/10) +(evo?db->emax.luk:0);;
	snprintf(atcmd_output, sizeof(atcmd_output) ,msg_txt(1274), hom->luk/10, min, max); // Luk: %d (%d~%d)
	clif->message(fd, atcmd_output);
	
	return true;
}

ACMD(homshuffle) {
	
	if(!sd->hd)
		return false; // nothing to do
	
	if(!homun->shuffle(sd->hd))
		return false;
	
	clif->message(sd->fd, msg_txt(1275)); // Homunculus stats altered.
	atcommand_homstats(fd, sd, command, message, info); //Print out the new stats
	return true;
}

/*==========================================
 * Show Items DB Info   v 1.0
 * originally by [Lupus]
 *------------------------------------------*/
ACMD(iteminfo)
{
	struct item_data *item_data, *item_array[MAX_SEARCH];
	int i, count = 1;
	
	if (!message || !*message) {
		clif->message(fd, msg_txt(1276)); // Please enter an item name/ID (usage: @ii/@iteminfo <item name/ID>).
		return false;
	}
	if ((item_array[0] = itemdb->exists(atoi(message))) == NULL)
		count = itemdb->search_name_array(item_array, MAX_SEARCH, message, 0);
	
	if (!count) {
		clif->message(fd, msg_txt(19));	// Invalid item ID or name.
		return false;
	}
	
	if (count > MAX_SEARCH) {
		sprintf(atcmd_output, msg_txt(269), MAX_SEARCH, count); // Displaying first %d out of %d matches
		clif->message(fd, atcmd_output);
		count = MAX_SEARCH;
	}
	for (i = 0; i < count; i++) {
		item_data = item_array[i];
		sprintf(atcmd_output, msg_txt(1277), // Item: '%s'/'%s'[%d] (%d) Type: %s | Extra Effect: %s
				item_data->name,item_data->jname,item_data->slot,item_data->nameid,
				itemdb->typename(item_data->type),
				(item_data->script==NULL)? msg_txt(1278) : msg_txt(1279) // None / With script
				);
		clif->message(fd, atcmd_output);
		
		sprintf(atcmd_output, msg_txt(1280), item_data->value_buy, item_data->value_sell, item_data->weight/10. ); // NPC Buy:%dz, Sell:%dz | Weight: %.1f
		clif->message(fd, atcmd_output);
		
		if (item_data->maxchance == -1)
			strcpy(atcmd_output, msg_txt(1281)); //  - Available in the shops only.
		else if ( !battle_config.atcommand_mobinfo_type ) {
			if( item_data->maxchance )
				sprintf(atcmd_output, msg_txt(1282), (float)item_data->maxchance / 100 ); //  - Maximal monsters drop chance: %02.02f%%
			else
				strcpy(atcmd_output, msg_txt(1283)); //  - Monsters don't drop this item.
		}
		clif->message(fd, atcmd_output);
		
	}
	return true;
}

/*==========================================
 * Show who drops the item.
 *------------------------------------------*/
ACMD(whodrops)
{
	struct item_data *item_data, *item_array[MAX_SEARCH];
	int i,j, count = 1;
	
	if (!message || !*message) {
		clif->message(fd, msg_txt(1284)); // Please enter item name/ID (usage: @whodrops <item name/ID>).
		return false;
	}
	if ((item_array[0] = itemdb->exists(atoi(message))) == NULL)
		count = itemdb->search_name_array(item_array, MAX_SEARCH, message, 0);
	
	if (!count) {
		clif->message(fd, msg_txt(19));	// Invalid item ID or name.
		return false;
	}
	
	if (count > MAX_SEARCH) {
		sprintf(atcmd_output, msg_txt(269), MAX_SEARCH, count); // Displaying first %d out of %d matches
		clif->message(fd, atcmd_output);
		count = MAX_SEARCH;
	}
	for (i = 0; i < count; i++) {
		item_data = item_array[i];
		sprintf(atcmd_output, msg_txt(1285), item_data->jname,item_data->slot); // Item: '%s'[%d]
		clif->message(fd, atcmd_output);
		
		if (item_data->mob[0].chance == 0) {
			strcpy(atcmd_output, msg_txt(1286)); //  - Item is not dropped by mobs.
			clif->message(fd, atcmd_output);
		} else {
			sprintf(atcmd_output, msg_txt(1287), MAX_SEARCH); //  - Common mobs with highest drop chance (only max %d are listed):
			clif->message(fd, atcmd_output);
			
			for (j=0; j < MAX_SEARCH && item_data->mob[j].chance > 0; j++)
			{
				sprintf(atcmd_output, "- %s (%02.02f%%)", mob->db(item_data->mob[j].id)->jname, item_data->mob[j].chance/100.);
				clif->message(fd, atcmd_output);
			}
		}
	}
	return true;
}

ACMD(whereis)
{
	struct mob_db *monster, *mob_array[MAX_SEARCH];
	int count;
	int i, j, k;
	
	if (!message || !*message) {
		clif->message(fd, msg_txt(1288)); // Please enter a monster name/ID (usage: @whereis <monster_name_or_monster_ID>).
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
		clif->message(fd, msg_txt(40)); // Invalid monster ID or name.
		return false;
	}
	
	if (count > MAX_SEARCH) {
		sprintf(atcmd_output, msg_txt(269), MAX_SEARCH, count);
		clif->message(fd, atcmd_output);
		count = MAX_SEARCH;
	}
	for (k = 0; k < count; k++) {
		monster = mob_array[k];
		snprintf(atcmd_output, sizeof atcmd_output, msg_txt(1289), monster->jname); // %s spawns in:
		clif->message(fd, atcmd_output);
		
		for (i = 0; i < ARRAYLENGTH(monster->spawn) && monster->spawn[i].qty; i++) {
			j = map->mapindex2mapid(monster->spawn[i].mapindex);
			if (j < 0) continue;
			snprintf(atcmd_output, sizeof atcmd_output, "%s (%d)", map->list[j].name, monster->spawn[i].qty);
			clif->message(fd, atcmd_output);
		}
		if (i == 0)
			clif->message(fd, msg_txt(1290)); // This monster does not spawn normally.
	}
	
	return true;
}

ACMD(version) {
	sprintf(atcmd_output, msg_txt(1296), sysinfo->is64bit() ? 64 : 32, sysinfo->platform()); // Hercules %d-bit for %s
	clif->message(fd, atcmd_output);
	sprintf(atcmd_output, msg_txt(1295), sysinfo->vcstype(), sysinfo->vcsrevision_src(), sysinfo->vcsrevision_scripts()); // %s revision '%s' (src) / '%s' (scripts)
	clif->message(fd, atcmd_output);

	return true;
}

/*==========================================
 * @mutearea by MouseJstr
 *------------------------------------------*/
int atcommand_mutearea_sub(struct block_list *bl,va_list ap)
{ // As it is being used [ACMD(mutearea)] there's no need to be a bool, but if there's need to reuse it, it's better to be this way
	
	int time, id;
	struct map_session_data *pl_sd = (struct map_session_data *)bl;
	if (pl_sd == NULL)
		return 0;
	
	id = va_arg(ap, int);
	time = va_arg(ap, int);
	
	if (id != bl->id && !pc_get_group_level(pl_sd)) {
		pl_sd->status.manner -= time;
		if (pl_sd->status.manner < 0)
			sc_start(NULL,&pl_sd->bl,SC_NOCHAT,100,0,0);
		else
			status_change_end(&pl_sd->bl, SC_NOCHAT, INVALID_TIMER);
	}
	return 1;
}

ACMD(mutearea) {
	int time;
	
	if (!message || !*message) {
		clif->message(fd, msg_txt(1297)); // Please enter a time in minutes (usage: @mutearea/@stfu <time in minutes>).
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
	
	snprintf(buf, CHAT_SIZE_MAX, msg_txt(1298), // Experience rates: Base %.2fx / Job %.2fx
			 battle_config.base_exp_rate/100., battle_config.job_exp_rate/100.);
	clif->message(fd, buf);
	snprintf(buf, CHAT_SIZE_MAX, msg_txt(1299), // Normal Drop Rates: Common %.2fx / Healing %.2fx / Usable %.2fx / Equipment %.2fx / Card %.2fx
			 battle_config.item_rate_common/100., battle_config.item_rate_heal/100., battle_config.item_rate_use/100., battle_config.item_rate_equip/100., battle_config.item_rate_card/100.);
	clif->message(fd, buf);
	snprintf(buf, CHAT_SIZE_MAX, msg_txt(1300), // Boss Drop Rates: Common %.2fx / Healing %.2fx / Usable %.2fx / Equipment %.2fx / Card %.2fx
			 battle_config.item_rate_common_boss/100., battle_config.item_rate_heal_boss/100., battle_config.item_rate_use_boss/100., battle_config.item_rate_equip_boss/100., battle_config.item_rate_card_boss/100.);
	clif->message(fd, buf);
	snprintf(buf, CHAT_SIZE_MAX, msg_txt(1301), // Other Drop Rates: MvP %.2fx / Card-Based %.2fx / Treasure %.2fx
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
	
	if (sd->sc.count && //no "chatting" while muted.
		(sd->sc.data[SC_BERSERK] || (sd->sc.data[SC_DEEP_SLEEP] && sd->sc.data[SC_DEEP_SLEEP]->val2) ||
		 (sd->sc.data[SC_NOCHAT] && sd->sc.data[SC_NOCHAT]->val1&MANNER_NOCHAT)))
		return false;
	
	if (!message || !*message || sscanf(message, "%199[^\n]", tempmes) < 0) {
		clif->message(fd, msg_txt(1302)); // Please enter a message (usage: @me <message>).
		return false;
	}
	
	sprintf(atcmd_output, msg_txt(270), sd->status.name, tempmes);	// *%s %s*
	clif->disp_overhead(&sd->bl, atcmd_output);
	
	return true;
	
}

/*==========================================
 * @size
 * => Resize your character sprite. [Valaris]
 *------------------------------------------*/
ACMD(size)
{
	int size = 0;
	
	size = cap_value(atoi(message),SZ_MEDIUM,SZ_BIG);
	
	if(sd->state.size) {
		sd->state.size = SZ_MEDIUM;
		pc->setpos(sd, sd->mapindex, sd->bl.x, sd->bl.y, CLR_TELEPORT);
	}
	
	sd->state.size = size;
	if( size == SZ_SMALL )
		clif->specialeffect(&sd->bl,420,AREA);
	else if( size == SZ_BIG )
		clif->specialeffect(&sd->bl,422,AREA);
	
	clif->message(fd, msg_txt(1303)); // Size change applied.
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
	for( pl_sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); pl_sd = (TBL_PC*)mapit->next(iter) ) {
		if( pl_sd->state.size != size ) {
			if( pl_sd->state.size ) {
				pl_sd->state.size = SZ_MEDIUM;
				pc->setpos(pl_sd, pl_sd->mapindex, pl_sd->bl.x, pl_sd->bl.y, CLR_TELEPORT);
			}
			
			pl_sd->state.size = size;
			if( size == SZ_SMALL )
				clif->specialeffect(&pl_sd->bl,420,AREA);
			else if( size == SZ_BIG )
				clif->specialeffect(&pl_sd->bl,422,AREA);
		}
	}
	mapit->free(iter);
	
	clif->message(fd, msg_txt(1303)); // Size change applied.
	return true;
}

ACMD(sizeguild)
{
	int size = 0, i;
	char guild_name[NAME_LENGTH];
	struct map_session_data *pl_sd;
	struct guild *g;
	
	memset(guild_name, '\0', sizeof(guild_name));
	
	if( !message || !*message || sscanf(message, "%d %23[^\n]", &size, guild_name) < 2 ) {
		clif->message(fd, msg_txt(1304)); // Please enter guild name/ID (usage: @sizeguild <size> <guild name/ID>).
		return false;
	}
	
	if( (g = guild->searchname(guild_name)) == NULL && (g = guild->search(atoi(guild_name))) == NULL ) {
		clif->message(fd, msg_txt(94)); // Incorrect name/ID, or no one from the guild is online.
		return false;
	}
	
	size = cap_value(size,SZ_MEDIUM,SZ_BIG);
	
	for( i = 0; i < g->max_member; i++ ) {
		if( (pl_sd = g->member[i].sd) && pl_sd->state.size != size ) {
			if( pl_sd->state.size ) {
				pl_sd->state.size = SZ_MEDIUM;
				pc->setpos(pl_sd, pl_sd->mapindex, pl_sd->bl.x, pl_sd->bl.y, CLR_TELEPORT);
			}
			
			pl_sd->state.size = size;
			if( size == SZ_SMALL )
				clif->specialeffect(&pl_sd->bl,420,AREA);
			else if( size == SZ_BIG )
				clif->specialeffect(&pl_sd->bl,422,AREA);
		}
	}
	
	clif->message(fd, msg_txt(1303)); // Size change applied.
	return true;
}

/*==========================================
 * @monsterignore
 * => Makes monsters ignore you. [Valaris]
 *------------------------------------------*/
ACMD(monsterignore) {
	
	if (!sd->state.monster_ignore) {
		sd->state.monster_ignore = 1;
		clif->message(sd->fd, msg_txt(1305)); // You are now immune to attacks.
	} else {
		sd->state.monster_ignore = 0;
		clif->message(sd->fd, msg_txt(1306)); // Returned to normal state.
	}
	
	return true;
}
/*==========================================
 * @fakename
 * => Gives your character a fake name. [Valaris]
 *------------------------------------------*/
ACMD(fakename){
	
	if( !message || !*message )
	{
		if( sd->fakename[0] )
		{
			sd->fakename[0] = '\0';
			clif->charnameack(0, &sd->bl);
			clif->message(sd->fd, msg_txt(1307)); // Returned to real name.
			return true;
		}
		
		clif->message(sd->fd, msg_txt(1308)); // You must enter a name.
		return false;
	}
	
	if( strlen(message) < 2 )
	{
		clif->message(sd->fd, msg_txt(1309)); // Fake name must be at least two characters.
		return false;
	}
	
	safestrncpy(sd->fakename, message, sizeof(sd->fakename));
	clif->charnameack(0, &sd->bl);
	clif->message(sd->fd, msg_txt(1310)); // Fake name enabled.
	
	return true;
}

/*==========================================
 * Ragnarok Resources
 *------------------------------------------*/
ACMD(mapflag) {
#define CHECKFLAG( cmd ) do { if ( map->list[ sd->bl.m ].flag.cmd ) clif->message(sd->fd,#cmd); } while(0)
#define SETFLAG( cmd ) do { \
	if ( strcmp( flag_name , #cmd ) == 0 ) { \
		map->list[ sd->bl.m ].flag.cmd = flag; \
		sprintf(atcmd_output,"[ @mapflag ] %s flag has been set to %s value = %hd",#cmd,flag?"On":"Off",flag); \
		clif->message(sd->fd,atcmd_output); \
		return true; \
	} \
} while(0)

	char flag_name[100];
	short flag=0,i;

	memset(flag_name, '\0', sizeof(flag_name));
	
	if (!message || !*message || (sscanf(message, "%99s %hd", flag_name, &flag) < 1)) {
		clif->message(sd->fd,msg_txt(1311)); // Enabled Mapflags in this map:
		clif->message(sd->fd,"----------------------------------");
		CHECKFLAG(autotrade);         CHECKFLAG(allowks);            CHECKFLAG(nomemo);       CHECKFLAG(noteleport);
		CHECKFLAG(noreturn);          CHECKFLAG(monster_noteleport); CHECKFLAG(nosave);       CHECKFLAG(nobranch);
		CHECKFLAG(noexppenalty);      CHECKFLAG(pvp);                CHECKFLAG(pvp_noparty);  CHECKFLAG(pvp_noguild);
		CHECKFLAG(pvp_nightmaredrop); CHECKFLAG(pvp_nocalcrank);     CHECKFLAG(gvg_castle);   CHECKFLAG(gvg);
		CHECKFLAG(gvg_dungeon);       CHECKFLAG(gvg_noparty);        CHECKFLAG(battleground); CHECKFLAG(nozenypenalty);
		CHECKFLAG(notrade);           CHECKFLAG(noskill);            CHECKFLAG(nowarp);       CHECKFLAG(nowarpto);
		CHECKFLAG(noicewall);         CHECKFLAG(snow);               CHECKFLAG(clouds);       CHECKFLAG(clouds2);
		CHECKFLAG(fog);               CHECKFLAG(fireworks);          CHECKFLAG(sakura);       CHECKFLAG(leaves);
		CHECKFLAG(nobaseexp);
		CHECKFLAG(nojobexp);          CHECKFLAG(nomobloot);          CHECKFLAG(nomvploot);    CHECKFLAG(nightenabled);
		CHECKFLAG(nodrop);            CHECKFLAG(novending);          CHECKFLAG(loadevent);
		CHECKFLAG(nochat);            CHECKFLAG(partylock);          CHECKFLAG(guildlock);    CHECKFLAG(src4instance);
		CHECKFLAG(notomb);            CHECKFLAG(nocashshop);
		clif->message(sd->fd," ");
		clif->message(sd->fd,msg_txt(1312)); // Usage: "@mapflag monster_noteleport 1" (0=Off | 1=On)
		clif->message(sd->fd,msg_txt(1313)); // Type "@mapflag available" to list the available mapflags.
		return true;
	}
	for (i = 0; flag_name[i]; i++) flag_name[i] = TOLOWER(flag_name[i]); //lowercase
	
	if ( strcmp( flag_name , "gvg" ) == 0 ) {
		if( flag && !map->list[sd->bl.m].flag.gvg )
			map->zone_change2(sd->bl.m,strdb_get(map->zone_db, MAP_ZONE_GVG_NAME));
		else if ( !flag && map->list[sd->bl.m].flag.gvg )
			map->zone_change2(sd->bl.m,map->list[sd->bl.m].prev_zone);
	} else if ( strcmp( flag_name , "pvp" ) == 0 ) {
		if( flag && !map->list[sd->bl.m].flag.pvp )
			map->zone_change2(sd->bl.m,strdb_get(map->zone_db, MAP_ZONE_PVP_NAME));
		else if ( !flag && map->list[sd->bl.m].flag.pvp )
			map->zone_change2(sd->bl.m,map->list[sd->bl.m].prev_zone);
	} else if ( strcmp( flag_name , "battleground" ) == 0 ) {
		if( flag && !map->list[sd->bl.m].flag.battleground )
			map->zone_change2(sd->bl.m,strdb_get(map->zone_db, MAP_ZONE_BG_NAME));
		else if ( !flag && map->list[sd->bl.m].flag.battleground )
			map->zone_change2(sd->bl.m,map->list[sd->bl.m].prev_zone);
	}
	
	SETFLAG(autotrade);         SETFLAG(allowks);            SETFLAG(nomemo);       SETFLAG(noteleport);
	SETFLAG(noreturn);          SETFLAG(monster_noteleport); SETFLAG(nosave);       SETFLAG(nobranch);
	SETFLAG(noexppenalty);      SETFLAG(pvp);                SETFLAG(pvp_noparty);  SETFLAG(pvp_noguild);
	SETFLAG(pvp_nightmaredrop); SETFLAG(pvp_nocalcrank);     SETFLAG(gvg_castle);   SETFLAG(gvg);
	SETFLAG(gvg_dungeon);       SETFLAG(gvg_noparty);        SETFLAG(battleground); SETFLAG(nozenypenalty);
	SETFLAG(notrade);           SETFLAG(noskill);            SETFLAG(nowarp);       SETFLAG(nowarpto);
	SETFLAG(noicewall);         SETFLAG(snow);               SETFLAG(clouds);       SETFLAG(clouds2);
	SETFLAG(fog);               SETFLAG(fireworks);          SETFLAG(sakura);       SETFLAG(leaves);
	SETFLAG(nobaseexp);
	SETFLAG(nojobexp);          SETFLAG(nomobloot);          SETFLAG(nomvploot);    SETFLAG(nightenabled);
	SETFLAG(nodrop);            SETFLAG(novending);          SETFLAG(loadevent);
	SETFLAG(nochat);            SETFLAG(partylock);          SETFLAG(guildlock);    SETFLAG(src4instance);
	SETFLAG(notomb);            SETFLAG(nocashshop);
	
	clif->message(sd->fd,msg_txt(1314)); // Invalid flag name or flag.
	clif->message(sd->fd,msg_txt(1312)); // Usage: "@mapflag monster_noteleport 1" (0=Off | 1=On)
	clif->message(sd->fd,msg_txt(1315)); // Available Flags:
	clif->message(sd->fd,"----------------------------------");
	clif->message(sd->fd,"town, autotrade, allowks, nomemo, noteleport, noreturn, monster_noteleport, nosave,");
	clif->message(sd->fd,"nobranch, noexppenalty, pvp, pvp_noparty, pvp_noguild, pvp_nightmaredrop,");
	clif->message(sd->fd,"pvp_nocalcrank, gvg_castle, gvg, gvg_dungeon, gvg_noparty, battleground,");
	clif->message(sd->fd,"nozenypenalty, notrade, noskill, nowarp, nowarpto, noicewall, snow, clouds, clouds2,");
	clif->message(sd->fd,"fog, fireworks, sakura, leaves, nobaseexp, nojobexp, nomobloot,");
	clif->message(sd->fd,"nomvploot, nightenabled, nodrop, novending, loadevent, nochat, partylock,");
	clif->message(sd->fd,"guildlock, src4instance, notomb, nocashshop");
	
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
		clif->message(fd, msg_txt(1316)); // Gained exp will not be shown.
		return true;
	}
	
	sd->state.showexp = 1;
	clif->message(fd, msg_txt(1317)); // Gained exp is now shown.
	return true;
}

ACMD(showzeny)
{
	if (sd->state.showzeny) {
		sd->state.showzeny = 0;
		clif->message(fd, msg_txt(1318)); // Gained zeny will not be shown.
		return true;
	}
	
	sd->state.showzeny = 1;
	clif->message(fd, msg_txt(1319)); // Gained zeny is now shown.
	return true;
}

ACMD(showdelay)
{
	if (sd->state.showdelay) {
		sd->state.showdelay = 0;
		clif->message(fd, msg_txt(1320)); // Skill delay failures will not be shown.
		return true;
	}
	
	sd->state.showdelay = 1;
	clif->message(fd, msg_txt(1321)); // Skill delay failures are now shown.
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
ACMD(invite) {
	unsigned int did = sd->duel_group;
	struct map_session_data *target_sd = map->nick2sd((char *)message);
	
	if(did == 0)
	{
		// "Duel: @invite without @duel."
		clif->message(fd, msg_txt(350));
		return false;
	}
	
	if(duel->list[did].max_players_limit > 0 &&
	   duel->list[did].members_count >= duel->list[did].max_players_limit) {
		
		// "Duel: Limit of players is reached."
		clif->message(fd, msg_txt(351));
		return false;
	}
	
	if(target_sd == NULL) {
		// "Duel: Player not found."
		clif->message(fd, msg_txt(352));
		return false;
	}
	
	if(target_sd->duel_group > 0 || target_sd->duel_invite > 0) {
		// "Duel: Player already in duel."
		clif->message(fd, msg_txt(353));
		return false;
	}
	
	if(battle_config.duel_only_on_same_map && target_sd->bl.m != sd->bl.m)
	{
		// "Duel: You can't invite %s because he/she isn't in the same map."
		sprintf(atcmd_output, msg_txt(364), message);
		clif->message(fd, atcmd_output);
		return false;
	}
	
	duel->invite(did, sd, target_sd);
	// "Duel: Invitation has been sent."
	clif->message(fd, msg_txt(354));
	return true;
}

ACMD(duel) {
	unsigned int maxpl = 0;
	
	if(sd->duel_group > 0) {
		duel->showinfo(sd->duel_group, sd);
		return true;
	}
	
	if(sd->duel_invite > 0) {
		// "Duel: @duel without @reject."
		clif->message(fd, msg_txt(355));
		return false;
	}
	
	if(!duel->checktime(sd)) {
		char output[CHAT_SIZE_MAX];
		// "Duel: You can take part in duel only one time per %d minutes."
		sprintf(output, msg_txt(356), battle_config.duel_time_interval);
		clif->message(fd, output);
		return false;
	}
	
	if( message[0] ) {
		if(sscanf(message, "%d", &maxpl) >= 1) {
			if(maxpl < 2 || maxpl > 65535) {
				clif->message(fd, msg_txt(357)); // "Duel: Invalid value."
				return false;
			}
			duel->create(sd, maxpl);
		} else {
			struct map_session_data *target_sd;
			target_sd = map->nick2sd((char *)message);
			if(target_sd != NULL) {
				unsigned int newduel;
				if((newduel = duel->create(sd, 2)) != -1) {
					if(target_sd->duel_group > 0 ||	target_sd->duel_invite > 0) {
						clif->message(fd, msg_txt(353)); // "Duel: Player already in duel."
						return false;
					}
					duel->invite(newduel, sd, target_sd);
					clif->message(fd, msg_txt(354)); // "Duel: Invitation has been sent."
				}
			} else {
				// "Duel: Player not found."
				clif->message(fd, msg_txt(352));
				return false;
			}
		}
	} else
		duel->create(sd, 0);
	
	return true;
}


ACMD(leave) {
	if(sd->duel_group <= 0) {
		// "Duel: @leave without @duel."
		clif->message(fd, msg_txt(358));
		return false;
	}
	
	duel->leave(sd->duel_group, sd);
	clif->message(fd, msg_txt(359)); // "Duel: You left the duel."
	return true;
}

ACMD(accept) {
	if(!duel->checktime(sd)) {
		char output[CHAT_SIZE_MAX];
		// "Duel: You can take part in duel only one time per %d minutes."
		sprintf(output, msg_txt(356), battle_config.duel_time_interval);
		clif->message(fd, output);
		return false;
	}
	
	if(sd->duel_invite <= 0) {
		// "Duel: @accept without invititation."
		clif->message(fd, msg_txt(360));
		return false;
	}
	
	if( duel->list[sd->duel_invite].max_players_limit > 0
	 && duel->list[sd->duel_invite].members_count >= duel->list[sd->duel_invite].max_players_limit ) {
		// "Duel: Limit of players is reached."
		clif->message(fd, msg_txt(351));
		return false;
	}
	
	duel->accept(sd->duel_invite, sd);
	// "Duel: Invitation has been accepted."
	clif->message(fd, msg_txt(361));
	return true;
}

ACMD(reject) {
	if(sd->duel_invite <= 0) {
		// "Duel: @reject without invititation."
		clif->message(fd, msg_txt(362));
		return false;
	}
	
	duel->reject(sd->duel_invite, sd);
	// "Duel: Invitation has been rejected."
	clif->message(fd, msg_txt(363));
	return true;
}

/*===================================
 * Cash Points
 *-----------------------------------*/
ACMD(cash)
{
	char output[128];
	int value;
	int ret=0;
	
	if( !message || !*message || (value = atoi(message)) == 0 ) {
		clif->message(fd, msg_txt(1322)); // Please enter an amount.
		return false;
	}
	
	if( !strcmpi(info->command,"cash") ) {
		if( value > 0 ) {
			if( (ret=pc->getcash(sd, value, 0)) >= 0){
				// If this option is set, the message is already sent by pc function
				if( !battle_config.cashshop_show_points ){
					sprintf(output, msg_txt(505), ret, sd->cashPoints);
					clif_disp_onlyself(sd, output, strlen(output));
				}
			} else
				clif->message(fd, msg_txt(149)); // Unable to decrease the number/value.
		} else {
			if( (ret=pc->paycash(sd, -value, 0)) >= 0){
			    sprintf(output, msg_txt(410), ret, sd->cashPoints);
			    clif_disp_onlyself(sd, output, strlen(output));
			} else
				clif->message(fd, msg_txt(41)); // Unable to decrease the number/value.
		}
	} else { // @points
		if( value > 0 ) {
			if( (ret=pc->getcash(sd, 0, value)) >= 0) {
				// If this option is set, the message is already sent by pc function
				if( !battle_config.cashshop_show_points ){
					sprintf(output, msg_txt(506), ret, sd->kafraPoints);
					clif_disp_onlyself(sd, output, strlen(output));
				}
			} else
				clif->message(fd, msg_txt(149)); // Unable to decrease the number/value.
		} else {
			if( (ret=pc->paycash(sd, -value, -value)) >= 0){
			    sprintf(output, msg_txt(411), ret, sd->kafraPoints);
			    clif_disp_onlyself(sd, output, strlen(output));
			} else
				clif->message(fd, msg_txt(41)); // Unable to decrease the number/value.
		}
	}
	
	return true;
}

// @clone/@slaveclone/@evilclone <playername> [Valaris]
ACMD(clone) {
	int x=0,y=0,flag=0,master=0,i=0;
	struct map_session_data *pl_sd=NULL;
	
	if (!message || !*message) {
		clif->message(sd->fd,msg_txt(1323)); // You must enter a player name or ID.
		return false;
	}
	
	if((pl_sd=map->nick2sd((char *)message)) == NULL && (pl_sd=map->charid2sd(atoi(message))) == NULL) {
		clif->message(fd, msg_txt(3));	// Character not found.
		return false;
	}
	
	if(pc_get_group_level(pl_sd) > pc_get_group_level(sd)) {
		clif->message(fd, msg_txt(126));	// Cannot clone a player of higher GM level than yourself.
		return false;
	}
	
	if (strcmpi(info->command, "clone") == 0)
		flag = 1;
	else if (strcmpi(info->command, "slaveclone") == 0) {
		flag = 2;
		if(pc_isdead(sd)){
			//"Unable to spawn slave clone."
		    clif->message(fd, msg_txt(129+flag*2));
		    return false;
		}
		master = sd->bl.id;
		if (battle_config.atc_slave_clone_limit
			&& mob->countslave(&sd->bl) >= battle_config.atc_slave_clone_limit) {
			clif->message(fd, msg_txt(127));	// You've reached your slave clones limit.
			return false;
		}
	}
	
	do {
		x = sd->bl.x + (rnd() % 10 - 5);
		y = sd->bl.y + (rnd() % 10 - 5);
	} while (map->getcell(sd->bl.m,x,y,CELL_CHKNOPASS) && i++ < 10);
	
	if (i >= 10) {
		x = sd->bl.x;
		y = sd->bl.y;
	}
	
	if((x = mob->clone_spawn(pl_sd, sd->bl.m, x, y, "", master, 0, flag?1:0, 0)) > 0) {
		clif->message(fd, msg_txt(128+flag*2));	// Evil Clone spawned. Clone spawned. Slave clone spawned.
		return true;
	}
	clif->message(fd, msg_txt(129+flag*2));	// Unable to spawn evil clone. Unable to spawn clone. Unable to spawn slave clone.
	return false;
}

/*=====================================
 * Autorejecting Invites/Deals [LuzZza]
 * Usage: @noask
 *-------------------------------------*/
ACMD(noask)
{
	if(sd->state.noask) {
		clif->message(fd, msg_txt(391)); // Autorejecting is deactivated.
		sd->state.noask = 0;
	} else {
		clif->message(fd, msg_txt(390)); // Autorejecting is activated.
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
	if (!message || !*message) {
		clif->message(sd->fd,msg_txt(277));	// Usage: @request <petition/message to online GMs>.
		return false;
	}
	
	sprintf(atcmd_output, msg_txt(278), message);	// (@request): %s
	intif->wis_message_to_gm(sd->status.name, PC_PERM_RECEIVE_REQUESTS, atcmd_output);
	clif_disp_onlyself(sd, atcmd_output, strlen(atcmd_output));
	clif->message(sd->fd,msg_txt(279));	// @request sent.
	return true;
}

/*==========================================
 * Feel (SG save map) Reset [HiddenDragon]
 *------------------------------------------*/
ACMD(feelreset)
{
	pc->resetfeel(sd);
	clif->message(fd, msg_txt(1324)); // Reset 'Feeling' maps.
	
	return true;
}

/*==========================================
 * AUCTION SYSTEM
 *------------------------------------------*/
ACMD(auction) {
	
	if( !battle_config.feature_auction ) {
		clif->colormes(sd->fd,COLOR_RED,msg_txt(1484));
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
		sd->state.noks = 0;
		clif->message(fd, msg_txt(1325)); // [ K.S Protection Inactive ]
	}
	else
	{
		if( !message || !*message || !strcmpi(message, "party") )
		{ // Default is Party
			sd->state.noks = 2;
			clif->message(fd, msg_txt(1326)); // [ K.S Protection Active - Option: Party ]
		}
		else if( !strcmpi(message, "self") )
		{
			sd->state.noks = 1;
			clif->message(fd, msg_txt(1327)); // [ K.S Protection Active - Option: Self ]
		}
		else if( !strcmpi(message, "guild") )
		{
			sd->state.noks = 3;
			clif->message(fd, msg_txt(1328)); // [ K.S Protection Active - Option: Guild ]
		}
		else
			clif->message(fd, msg_txt(1329)); // Usage: @noks <self|party|guild>
	}
	return true;
}
/*==========================================
 * Map Kill Steal Protection Setting
 *------------------------------------------*/
ACMD(allowks) {
	
	if( map->list[sd->bl.m].flag.allowks ) {
		map->list[sd->bl.m].flag.allowks = 0;
		clif->message(fd, msg_txt(1330)); // [ Map K.S Protection Active ]
	} else {
		map->list[sd->bl.m].flag.allowks = 1;
		clif->message(fd, msg_txt(1331)); // [ Map K.S Protection Inactive ]
	}
	return true;
}

ACMD(resetstat) {
	
	pc->resetstate(sd);
	sprintf(atcmd_output, msg_txt(207), sd->status.name);
	clif->message(fd, atcmd_output);
	return true;
}

ACMD(resetskill) {
	
	pc->resetskill(sd,1);
	sprintf(atcmd_output, msg_txt(206), sd->status.name);
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
		items = sd->status.storage.items;
		size = MAX_STORAGE;
	} else if( strcmpi(info->command, "cartlist") == 0 ) {
		location = "cart";
		items = sd->status.cart;
		size = MAX_CART;
	} else if( strcmpi(info->command, "itemlist") == 0 ) {
		location = "inventory";
		items = sd->status.inventory;
		size = MAX_INVENTORY;
	} else
		return false;
	
	StrBuf->Init(&buf);
	
	count = 0; // total slots occupied
	counter = 0; // total items found
	for( i = 0; i < size; ++i )
	{
		const struct item* it = &items[i];
		struct item_data* itd;
		
		if( it->nameid == 0 || (itd = itemdb->exists(it->nameid)) == NULL )
			continue;
		
		counter += it->amount;
		count++;
		
		if( count == 1 )
		{
			StrBuf->Printf(&buf, msg_txt(1332), location, sd->status.name); // ------ %s items list of '%s' ------
			clif->message(fd, StrBuf->Value(&buf));
			StrBuf->Clear(&buf);
		}
		
		if( it->refine )
			StrBuf->Printf(&buf, "%d %s %+d (%s, id: %d)", it->amount, itd->jname, it->refine, itd->name, it->nameid);
		else
			StrBuf->Printf(&buf, "%d %s (%s, id: %d)", it->amount, itd->jname, itd->name, it->nameid);
		
		if( it->equip ) {
			char equipstr[CHAT_SIZE_MAX];
			strcpy(equipstr, msg_txt(1333)); //  | equipped:
			if( it->equip & EQP_GARMENT )
				strcat(equipstr, msg_txt(1334)); // garment,
			if( it->equip & EQP_ACC_L )
				strcat(equipstr, msg_txt(1335)); // left accessory,
			if( it->equip & EQP_ARMOR )
				strcat(equipstr, msg_txt(1336)); // body/armor,
			if( (it->equip & EQP_ARMS) == EQP_HAND_R )
				strcat(equipstr, msg_txt(1337)); // right hand,
			if( (it->equip & EQP_ARMS) == EQP_HAND_L )
				strcat(equipstr, msg_txt(1338)); // left hand,
			if( (it->equip & EQP_ARMS) == EQP_ARMS )
				strcat(equipstr, msg_txt(1339)); // both hands,
			if( it->equip & EQP_SHOES )
				strcat(equipstr, msg_txt(1340)); // feet,
			if( it->equip & EQP_ACC_R )
				strcat(equipstr, msg_txt(1341)); // right accessory,
			if( (it->equip & EQP_HELM) == EQP_HEAD_LOW )
				strcat(equipstr, msg_txt(1342)); // lower head,
			if( (it->equip & EQP_HELM) == EQP_HEAD_TOP )
				strcat(equipstr, msg_txt(1343)); // top head,
			if( (it->equip & EQP_HELM) == (EQP_HEAD_LOW|EQP_HEAD_TOP) )
				strcat(equipstr, msg_txt(1344)); // lower/top head,
			if( (it->equip & EQP_HELM) == EQP_HEAD_MID )
				strcat(equipstr, msg_txt(1345)); // mid head,
			if( (it->equip & EQP_HELM) == (EQP_HEAD_LOW|EQP_HEAD_MID) )
				strcat(equipstr, msg_txt(1346)); // lower/mid head,
			if( (it->equip & EQP_HELM) == EQP_HELM )
				strcat(equipstr, msg_txt(1347)); // lower/mid/top head,
			// remove final ', '
			equipstr[strlen(equipstr) - 2] = '\0';
			StrBuf->AppendStr(&buf, equipstr);
		}
		
		clif->message(fd, StrBuf->Value(&buf));
		StrBuf->Clear(&buf);
		
		if( it->card[0] == CARD0_PET ) {
			// pet egg
			if (it->card[3])
				StrBuf->Printf(&buf, msg_txt(1348), (unsigned int)MakeDWord(it->card[1], it->card[2])); //  -> (pet egg, pet id: %u, named)
			else
				StrBuf->Printf(&buf, msg_txt(1349), (unsigned int)MakeDWord(it->card[1], it->card[2])); //  -> (pet egg, pet id: %u, unnamed)
		} else if(it->card[0] == CARD0_FORGE) {
			// forged item
			StrBuf->Printf(&buf, msg_txt(1350), (unsigned int)MakeDWord(it->card[2], it->card[3]), it->card[1]>>8, it->card[1]&0x0f); //  -> (crafted item, creator id: %u, star crumbs %d, element %d)
		} else if(it->card[0] == CARD0_CREATE) {
			// created item
			StrBuf->Printf(&buf, msg_txt(1351), (unsigned int)MakeDWord(it->card[2], it->card[3])); //  -> (produced item, creator id: %u)
		} else {
			// normal item
			int counter2 = 0;

			for( j = 0; j < itd->slot; ++j ) {
				struct item_data* card;

				if( it->card[j] == 0 || (card = itemdb->exists(it->card[j])) == NULL )
					continue;

				counter2++;

				if( counter2 == 1 )
					StrBuf->AppendStr(&buf, msg_txt(1352)); //  -> (card(s):

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
		StrBuf->Printf(&buf, msg_txt(1353), location); // No item found in this player's %s.
	else
		StrBuf->Printf(&buf, msg_txt(1354), counter, count, location); // %d item(s) found in %d %s slots.
	
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
	
	sprintf(job_jobname, "Job - %s %s", pc->job_name(sd->status.class_), "(level %d)");
	sprintf(output, msg_txt(53), sd->status.name); // '%s' stats:
	
	clif->message(fd, output);
	
	for (i = 0; output_table[i].format != NULL; i++) {
		sprintf(output, output_table[i].format, output_table[i].value);
		clif->message(fd, output);
	}
	
	return true;
}

ACMD(delitem) {
	char item_name[100];
	int nameid, amount = 0, total, idx;
	struct item_data* id;
		
	if( !message || !*message || ( sscanf(message, "\"%99[^\"]\" %d", item_name, &amount) < 2 && sscanf(message, "%99s %d", item_name, &amount) < 2 ) || amount < 1 )
	{
		clif->message(fd, msg_txt(1355)); // Please enter an item name/ID, a quantity, and a player name (usage: #delitem <player> <item_name_or_ID> <quantity>).
		return false;
	}
	
	if( ( id = itemdb->search_name(item_name) ) != NULL || ( id = itemdb->exists(atoi(item_name)) ) != NULL )
	{
		nameid = id->nameid;
	}
	else
	{
		clif->message(fd, msg_txt(19)); // Invalid item ID or name.
		return false;
	}
	
	total = amount;
	
	// delete items
	while( amount && ( idx = pc->search_inventory(sd, nameid) ) != -1 )
	{
		int delamount = ( amount < sd->status.inventory[idx].amount ) ? amount : sd->status.inventory[idx].amount;
		
		if( sd->inventory_data[idx]->type == IT_PETEGG && sd->status.inventory[idx].card[0] == CARD0_PET )
		{// delete pet
			intif->delete_petdata(MakeDWord(sd->status.inventory[idx].card[1], sd->status.inventory[idx].card[2]));
		}
		pc->delitem(sd, idx, delamount, 0, 0, LOG_TYPE_COMMAND);
		
		amount-= delamount;
	}
	
	// notify target
	sprintf(atcmd_output, msg_txt(113), total-amount); // %d item(s) removed by a GM.
	clif->message(sd->fd, atcmd_output);
	
	// notify source
	if( amount == total )
	{
		clif->message(fd, msg_txt(116)); // Character does not have the item.
	}
	else if( amount )
	{
		sprintf(atcmd_output, msg_txt(115), total-amount, total-amount, total); // %d item(s) removed. Player had only %d on %d items.
		clif->message(fd, atcmd_output);
	}
	else
	{
		sprintf(atcmd_output, msg_txt(114), total); // %d item(s) removed from the player.
		clif->message(fd, atcmd_output);
	}
	
	return true;
}

/*==========================================
 * Custom Fonts
 *------------------------------------------*/
ACMD(font) {
	int font_id;
	
	font_id = atoi(message);
	if( font_id == 0 )
	{
		if( sd->status.font )
		{
			sd->status.font = 0;
			clif->message(fd, msg_txt(1356)); // Returning to normal font.
			clif->font(sd);
		}
		else
		{
			clif->message(fd, msg_txt(1357)); // Use @font <1-9> to change your message font.
			clif->message(fd, msg_txt(1358)); // Use 0 or no parameter to return to normal font.
		}
	}
	else if( font_id < 0 || font_id > 9 )
		clif->message(fd, msg_txt(1359)); // Invalid font. Use a value from 0 to 9.
	else if( font_id != sd->status.font )
	{
		sd->status.font = font_id;
		clif->font(sd);
		clif->message(fd, msg_txt(1360)); // Font changed.
	}
	else
		clif->message(fd, msg_txt(1361)); // Already using this font.
	
	return true;
}

/*==========================================
 * type: 1 = commands (@), 2 = charcommands (#)
 *------------------------------------------*/
void atcommand_commands_sub(struct map_session_data* sd, const int fd, AtCommandType type)
{
	char line_buff[CHATBOX_SIZE];
	char* cur = line_buff;
	AtCommandInfo* cmd;
	DBIterator *iter = db_iterator(atcommand->db);
	int count = 0;
	
	memset(line_buff,' ',CHATBOX_SIZE);
	line_buff[CHATBOX_SIZE-1] = 0;
	
	clif->message(fd, msg_txt(273)); // "Available commands:"
	
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
	
	sprintf(atcmd_output, msg_txt(274), count); // "%d commands found."
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
/* for new mounts */
ACMD(mount2) {
	
	if( sd->sc.option&(OPTION_WUGRIDER|OPTION_RIDING|OPTION_DRAGON|OPTION_MADOGEAR) ) {
		clif->message(fd, msg_txt(1476)); // You are already mounting something else
		return false;
	}
	
	clif->message(sd->fd,msg_txt(1362)); // NOTICE: If you crash with mount your LUA is outdated.
	if( !(sd->sc.data[SC_ALL_RIDING]) ) {
		clif->message(sd->fd,msg_txt(1363)); // You have mounted.
		sc_start(NULL,&sd->bl,SC_ALL_RIDING,100,0,-1);
	} else {
		clif->message(sd->fd,msg_txt(1364)); // You have released your mount.
		status_change_end(&sd->bl, SC_ALL_RIDING, INVALID_TIMER);
	}
	return true;
}

ACMD(accinfo) {
	char query[NAME_LENGTH];
	
	if (!message || !*message || strlen(message) > NAME_LENGTH ) {
		clif->message(fd, msg_txt(1365)); // Usage: @accinfo/@accountinfo <account_id/char name>
		clif->message(fd, msg_txt(1366)); // You may search partial name by making use of '%' in the search, ex. "@accinfo %Mario%" lists all characters whose name contains "Mario".
		return false;
	}
	
	//remove const type
	safestrncpy(query, message, NAME_LENGTH);
	
	intif->request_accinfo( sd->fd, sd->bl.id, pc_get_group_level(sd), query );
	
	return true;
}

/* [Ind] */
ACMD(set) {
	char reg[32], val[128];
	struct script_data* data;
	int toset = 0;
	bool is_str = false;
	size_t len;
	
	if( !message || !*message || (toset = sscanf(message, "%31s %128[^\n]s", reg, val)) < 1  ) {
		clif->message(fd, msg_txt(1367)); // Usage: @set <variable name> <value>
		clif->message(fd, msg_txt(1368)); // Usage: ex. "@set PoringCharVar 50"
		clif->message(fd, msg_txt(1369)); // Usage: ex. "@set PoringCharVarSTR$ Super Duper String"
		clif->message(fd, msg_txt(1370)); // Usage: ex. "@set PoringCharVarSTR$" outputs its value, Super Duper String.
		return false;
	}
	
	/* disabled variable types (they require a proper script state to function, so allowing them would crash the server) */
	if( reg[0] == '.' ) {
		clif->message(fd, msg_txt(1371)); // NPC variables may not be used with @set.
		return false;
	} else if( reg[0] == '\'' ) {
		clif->message(fd, msg_txt(1372)); // Instance variables may not be used with @set.
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
			script->set_var(sd, reg, (void*)__64BPTRSIZE((atoi(val))));
		
	}
	
	CREATE(data, struct script_data,1);
	
	
	if( is_str ) {// string variable
		
		switch( reg[0] ) {
			case '@':
				data->u.str = pc->readregstr(sd, script->add_str(reg));
				break;
			case '$':
				data->u.str = mapreg->readregstr(script->add_str(reg));
				break;
			case '#':
				if( reg[1] == '#' )
					data->u.str = pc_readaccountreg2str(sd, script->add_str(reg));// global
				else
					data->u.str = pc_readaccountregstr(sd, script->add_str(reg));// local
				break;
			default:
				data->u.str = pc_readglobalreg_str(sd, script->add_str(reg));
				break;
		}
		
		if( data->u.str == NULL || data->u.str[0] == '\0' ) {// empty string
			data->type = C_CONSTSTR;
			data->u.str = "";
		} else {// duplicate string
			data->type = C_STR;
			data->u.str = aStrdup(data->u.str);
		}
		
	} else {// integer variable
		
		data->type = C_INT;
		switch( reg[0] ) {
			case '@':
				data->u.num = pc->readreg(sd, script->add_str(reg));
				break;
			case '$':
				data->u.num = mapreg->readreg(script->add_str(reg));
				break;
			case '#':
				if( reg[1] == '#' )
					data->u.num = pc_readaccountreg2(sd, script->add_str(reg));// global
				else
					data->u.num = pc_readaccountreg(sd, script->add_str(reg));// local
				break;
			default:
				data->u.num = pc_readglobalreg(sd, script->add_str(reg));
				break;
		}
		
	}
	
	
	switch( data->type ) {
		case C_INT:
			sprintf(atcmd_output,msg_txt(1373),reg,data->u.num); // %s value is now :%d
			break;
		case C_STR:
			sprintf(atcmd_output,msg_txt(1374),reg,data->u.str); // %s value is now :%s
			break;
		case C_CONSTSTR:
			sprintf(atcmd_output,msg_txt(1375),reg); // %s is empty
			break;
		default:
			sprintf(atcmd_output,msg_txt(1376),reg,data->type); // %s data type is not supported :%u
			break;
	}
	
	clif->message(fd, atcmd_output);
	
	aFree(data);
	
	return true;
}
ACMD(reloadquestdb) {
	quest->reload();
	clif->message(fd, msg_txt(1377)); // Quest database has been reloaded.
	return true;
}
ACMD(addperm) {
	int perm_size = pcg->permission_count;
	bool add = (strcmpi(info->command, "addperm") == 0) ? true : false;
	int i;
	
	if( !message || !*message ) {
		sprintf(atcmd_output,  msg_txt(1378),command); // Usage: %s <permission_name>
		clif->message(fd, atcmd_output);
		clif->message(fd, msg_txt(1379)); // -- Permission List
		for( i = 0; i < perm_size; i++ ) {
			sprintf(atcmd_output,"- %s",pcg->permissions[i].name);
			clif->message(fd, atcmd_output);
		}
		return false;
	}
	
	ARR_FIND(0, perm_size, i, strcmpi(pcg->permissions[i].name, message) == 0);
	
	if( i == perm_size ) {
		sprintf(atcmd_output,msg_txt(1380),message); // '%s' is not a known permission.
		clif->message(fd, atcmd_output);
		clif->message(fd, msg_txt(1379)); // -- Permission List
		for( i = 0; i < perm_size; i++ ) {
			sprintf(atcmd_output,"- %s",pcg->permissions[i].name);
			clif->message(fd, atcmd_output);
		}
		return false;
	}
	
	if( add && (sd->extra_temp_permissions&pcg->permissions[i].permission) ) {
		sprintf(atcmd_output,  msg_txt(1381),sd->status.name,pcg->permissions[i].name); // User '%s' already possesses the '%s' permission.
		clif->message(fd, atcmd_output);
		return false;
	} else if ( !add && !(sd->extra_temp_permissions&pcg->permissions[i].permission) ) {
		sprintf(atcmd_output,  msg_txt(1382),sd->status.name,pcg->permissions[i].name); // User '%s' doesn't possess the '%s' permission.
		clif->message(fd, atcmd_output);
		sprintf(atcmd_output,msg_txt(1383),sd->status.name); // -- User '%s' Permissions
		clif->message(fd, atcmd_output);
		for( i = 0; i < perm_size; i++ ) {
			if( sd->extra_temp_permissions&pcg->permissions[i].permission ) {
				sprintf(atcmd_output,"- %s",pcg->permissions[i].name);
				clif->message(fd, atcmd_output);
			}
		}
		
		return false;
	}
	
	if( add )
		sd->extra_temp_permissions |= pcg->permissions[i].permission;
	else
		sd->extra_temp_permissions &=~ pcg->permissions[i].permission;
	
	
	sprintf(atcmd_output, msg_txt(1384),sd->status.name); // User '%s' permissions updated successfully. The changes are temporary.
	clif->message(fd, atcmd_output);
	
	return true;
}
ACMD(unloadnpcfile) {
	
	if( !message || !*message ) {
		clif->message(fd, msg_txt(1385)); // Usage: @unloadnpcfile <file name>
		return false;
	}
	
	if( npc->unloadfile(message) )
		clif->message(fd, msg_txt(1386)); // File unloaded. Be aware that mapflags and monsters spawned directly are not removed.
	else {
		clif->message(fd, msg_txt(1387)); // File not found.
		return false;
	}
	return true;
}
ACMD(cart) {
#define MC_CART_MDFY(x,idx) do { \
	sd->status.skill[idx].id = (x)?MC_PUSHCART:0; \
	sd->status.skill[idx].lv = (x)?1:0; \
	sd->status.skill[idx].flag = (x)?1:0; \
} while(0)
	
	int val = atoi(message);
	bool need_skill = pc->checkskill(sd, MC_PUSHCART) ? false : true;
	unsigned int index = skill->get_index(MC_PUSHCART);
	
	if( !message || !*message || val < 0 || val > MAX_CARTS ) {
		sprintf(atcmd_output, msg_txt(1390),command,MAX_CARTS); // Unknown Cart (usage: %s <0-%d>).
		clif->message(fd, atcmd_output);
		return false;
	}
	
	if( val == 0 && !pc_iscarton(sd) ) {
		clif->message(fd, msg_txt(1391)); // You do not possess a cart to be removed
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
	
	clif->message(fd, msg_txt(1392)); // Cart Added
	
	return true;
#undef MC_CART_MDFY
}
/* [Ind/Hercules] */
ACMD(join) {
	struct hChSysCh *channel = NULL;
	char name[HCHSYS_NAME_LENGTH], pass[HCHSYS_NAME_LENGTH];
	
	if( !message || !*message || sscanf(message, "%s %s", name, pass) < 1 ) {
		sprintf(atcmd_output, msg_txt(1399),command); // Unknown Channel (usage: %s <#channel_name>)
		clif->message(fd, atcmd_output);
		return false;
	}
	if( hChSys.local && strcmpi(name + 1, hChSys.local_name) == 0 ) {
		if( !map->list[sd->bl.m].channel ) {
			clif->chsys_mjoin(sd);
			if( map->list[sd->bl.m].channel ) /* mjoin might have refused, map has chatting capabilities disabled */
				return true;
		} else
			channel = map->list[sd->bl.m].channel;
	} else if( hChSys.ally && sd->status.guild_id && strcmpi(name + 1, hChSys.ally_name) == 0 ) {
		struct guild *g = sd->guild;
		if( !g ) return false;/* unlikely, but we wont let it crash anyway. */
		channel = g->channel;
	} else if( !( channel = strdb_get(clif->channel_db, name + 1) ) ) {
		sprintf(atcmd_output, msg_txt(1400),name,command); // Unknown Channel '%s' (usage: %s <#channel_name>)
		clif->message(fd, atcmd_output);
		return false;
	}
	
	if( !channel ) {
		sprintf(atcmd_output, msg_txt(1400),name,command); // Unknown Channel '%s' (usage: %s <#channel_name>)
		clif->message(fd, atcmd_output);
		return false;
	}
		
	
	if( idb_exists(channel->users, sd->status.char_id) ) {
		sprintf(atcmd_output, msg_txt(1436),name); // You're already in the '%s' channel
		clif->message(fd, atcmd_output);
		return false;
	}
	if( channel->pass[0] != '\0'  && strcmp(channel->pass,pass) != 0 ) {
		if( pc_has_permission(sd, PC_PERM_HCHSYS_ADMIN) ) {
			sd->stealth = true;
		} else {
			sprintf(atcmd_output, msg_txt(1401),name,command); // '%s' Channel is password protected (usage: %s <#channel_name> <password>)
			clif->message(fd, atcmd_output);
			return false;
		}
	}
	
	if( channel->banned && idb_exists(channel->banned, sd->status.account_id) ) {
		sprintf(atcmd_output, msg_txt(1438),name); // You cannot join the '%s' channel because you've been banned from it
		clif->message(fd, atcmd_output);
		return false;
	}
	
	if( !( channel->opt & hChSys_OPT_ANNOUNCE_JOIN ) ) {
		sprintf(atcmd_output, msg_txt(1403),name); // You're now in the '%s' channel
		clif->message(fd, atcmd_output);
	}
	if( channel->type == hChSys_ALLY ) {
		struct guild *g = sd->guild, *sg = NULL;
		int i;
		for (i = 0; i < MAX_GUILDALLIANCE; i++) {
			if( g->alliance[i].opposition == 0 && g->alliance[i].guild_id && (sg = guild->search(g->alliance[i].guild_id) ) ) {
				if( !(sg->channel->banned && idb_exists(sg->channel->banned, sd->status.account_id))) {
					clif->chsys_join(sg->channel,sd);
				}
			}
		}
	}
	clif->chsys_join(channel,sd);
	
	return true;
}
/* [Ind/Hercules] */
static inline void atcmd_channel_help(int fd, const char *command, bool can_create) {
	sprintf(atcmd_output, msg_txt(1404),command); // %s failed.
	clif->message(fd, atcmd_output);
	clif->message(fd, msg_txt(1414));// --- Available options:
	if( can_create ) {
		sprintf(atcmd_output, msg_txt(1415),command);// -- %s create <channel name> <channel password>
		clif->message(fd, atcmd_output);
		clif->message(fd, msg_txt(1416));// - creates a new channel
	}
	sprintf(atcmd_output, msg_txt(1417),command);// -- %s list
	clif->message(fd, atcmd_output);
	clif->message(fd, msg_txt(1418));// - lists public channels
	if( can_create ) {
		sprintf(atcmd_output, msg_txt(1419),command);// -- %s list colors
		clif->message(fd, atcmd_output);
		clif->message(fd, msg_txt(1420));// - lists colors available to select for custom channels
		sprintf(atcmd_output, msg_txt(1421),command);// -- %s setcolor <channel name> <color name>
		clif->message(fd, atcmd_output);
		clif->message(fd, msg_txt(1422));// - changes <channel name> color to <color name>
	}
	sprintf(atcmd_output, msg_txt(1423),command);// -- %s leave <channel name>
	clif->message(fd, atcmd_output);
	clif->message(fd, msg_txt(1424));// - leaves <channel name>
	sprintf(atcmd_output, msg_txt(1427),command);// -- %s bindto <channel name>
	clif->message(fd, atcmd_output);
	clif->message(fd, msg_txt(1428));// - binds global chat to <channel name>, making anything you type in global be sent to the channel
	sprintf(atcmd_output, msg_txt(1429),command);// -- %s unbind
	clif->message(fd, atcmd_output);
	clif->message(fd, msg_txt(1430));// - unbinds your global chat from its attached channel (if binded)
	sprintf(atcmd_output, msg_txt(1429),command);// -- %s unbind
	clif->message(fd, atcmd_output);
	if( can_create ) {
		sprintf(atcmd_output, msg_txt(1456),command);// -- %s ban <channel name> <character name>
		clif->message(fd, atcmd_output);
		clif->message(fd, msg_txt(1457));// - bans <character name> from <channel name> channel
		sprintf(atcmd_output, msg_txt(1458),command);// -- %s banlist <channel name>
		clif->message(fd, atcmd_output);
		clif->message(fd, msg_txt(1459));// - lists all banned characters from <channel name> channel
		sprintf(atcmd_output, msg_txt(1460),command);// -- %s unban <channel name> <character name>
		clif->message(fd, atcmd_output);
		clif->message(fd, msg_txt(1461));// - unbans <character name> from <channel name> channel
		sprintf(atcmd_output, msg_txt(1467),command);// -- %s unbanall <channel name>
		clif->message(fd, atcmd_output);
		clif->message(fd, msg_txt(1468));// - unbans everyone from <channel name>
		sprintf(atcmd_output, msg_txt(1462),command);// -- %s setopt <channel name> <option name> <option value>
		clif->message(fd, atcmd_output);
		clif->message(fd, msg_txt(1463));// - adds or removes <option name> with <option value> to <channel name> channel
	}
	
}
/* [Ind/Hercules] */
ACMD(channel) {
	struct hChSysCh *channel;
	char subcmd[HCHSYS_NAME_LENGTH], sub1[HCHSYS_NAME_LENGTH], sub2[HCHSYS_NAME_LENGTH], sub3[HCHSYS_NAME_LENGTH];
	unsigned char k = 0;
	sub1[0] = sub2[0] = sub3[0] = '\0';
	
	if( !message || !*message || sscanf(message, "%s %s %s %s", subcmd, sub1, sub2, sub3) < 1 ) {
		atcmd_channel_help(fd,command,( hChSys.allow_user_channel_creation || pc_has_permission(sd, PC_PERM_HCHSYS_ADMIN) ));
		return true;
	}
	
	if( strcmpi(subcmd,"create") == 0 && ( hChSys.allow_user_channel_creation || pc_has_permission(sd, PC_PERM_HCHSYS_ADMIN) ) ) {
		if( sub1[0] != '#' ) {
			clif->message(fd, msg_txt(1405));// Channel name must start with a '#'
			return false;
		} else if ( strlen(sub1) < 3 || strlen(sub1) > HCHSYS_NAME_LENGTH ) {
			sprintf(atcmd_output, msg_txt(1406), HCHSYS_NAME_LENGTH);// Channel length must be between 3 and %d
			clif->message(fd, atcmd_output);
			return false;
		} else if ( sub3[0] != '\0' ) {
			clif->message(fd, msg_txt(1408)); // Channel password may not contain spaces
			return false;
		}
		if( strcmpi(sub1 + 1,hChSys.local_name) == 0 || strcmpi(sub1 + 1,hChSys.ally_name) == 0 || strdb_exists(clif->channel_db, sub1 + 1) ) {
			sprintf(atcmd_output, msg_txt(1407), sub1);// Channel '%s' is not available
			clif->message(fd, atcmd_output);
			return false;
		}
		
		CREATE( channel, struct hChSysCh, 1 );
		
		clif->chsys_create(channel,sub1 + 1,sub2,0);
		
		channel->owner = sd->status.char_id;
		channel->type = hChSys_PRIVATE;
		
		if( !( channel->opt & hChSys_OPT_ANNOUNCE_JOIN ) ) {
			sprintf(atcmd_output, msg_txt(1403),sub1); // You're now in the '%s' channel
			clif->message(fd, atcmd_output);
		}
		
		clif->chsys_join(channel,sd);
		
	} else if ( strcmpi(subcmd,"list") == 0 ) {
		if( sub1[0] != '\0' && strcmpi(sub1,"colors") == 0 ) {
			char mout[40];
			for( k = 0; k < hChSys.colors_count; k++ ) {
				unsigned short msg_len = 1;
				msg_len += sprintf(mout, "[ %s list colors ] : %s",command,hChSys.colors_name[k]);
				
				WFIFOHEAD(fd,msg_len + 12);
				WFIFOW(fd,0) = 0x2C1;
				WFIFOW(fd,2) = msg_len + 12;
				WFIFOL(fd,4) = 0;
				WFIFOL(fd,8) = hChSys.colors[k];
				safestrncpy((char*)WFIFOP(fd,12), mout, msg_len);
				WFIFOSET(fd, msg_len + 12);
			}
		} else {
			DBIterator *iter = db_iterator(clif->channel_db);
			bool show_all = pc_has_permission(sd, PC_PERM_HCHSYS_ADMIN) ? true : false;
			clif->message(fd, msg_txt(1410)); // -- Public Channels
			if( hChSys.local ) {
				sprintf(atcmd_output, msg_txt(1409), hChSys.local_name, map->list[sd->bl.m].channel ? db_size(map->list[sd->bl.m].channel->users) : 0);// - #%s ( %d users )
				clif->message(fd, atcmd_output);
			}
			if( hChSys.ally && sd->status.guild_id ) {
				struct guild *g = sd->guild;
				if( !g ) { dbi_destroy(iter); return false; }
				sprintf(atcmd_output, msg_txt(1409), hChSys.ally_name, db_size(g->channel->users));// - #%s ( %d users )
				clif->message(fd, atcmd_output);
			}
			for(channel = dbi_first(iter); dbi_exists(iter); channel = dbi_next(iter)) {
				if( show_all || channel->type == hChSys_PUBLIC || channel->type == hChSys_IRC ) {
					sprintf(atcmd_output, msg_txt(1409), channel->name, db_size(channel->users));// - #%s ( %d users )
					clif->message(fd, atcmd_output);
				}
			}
			dbi_destroy(iter);
		}
	} else if ( strcmpi(subcmd,"setcolor") == 0 ) {
		
		if( sub1[0] != '#' ) {
			clif->message(fd, msg_txt(1405));// Channel name must start with a '#'
			return false;
		}
		
		if( !(channel = strdb_get(clif->channel_db, sub1 + 1)) ) {
			sprintf(atcmd_output, msg_txt(1407), sub1);// Channel '%s' is not available
			clif->message(fd, atcmd_output);
			return false;
		}
		
		if( channel->owner != sd->status.char_id && !pc_has_permission(sd, PC_PERM_HCHSYS_ADMIN) ) {
			sprintf(atcmd_output, msg_txt(1412), sub1);// You're not the owner of channel '%s'
			clif->message(fd, atcmd_output);
			return false;
		}
		
		for( k = 0; k < hChSys.colors_count; k++ ) {
			if( strcmpi(sub2,hChSys.colors_name[k]) == 0 )
				break;
		}
		if( k == hChSys.colors_count ) {
			sprintf(atcmd_output, msg_txt(1411), sub2);// Unknown color '%s'
			clif->message(fd, atcmd_output);
			return false;
		}
		channel->color = k;
		sprintf(atcmd_output, msg_txt(1413),sub1,hChSys.colors_name[k]);// '%s' channel color updated to '%s'
		clif->message(fd, atcmd_output);
	} else if ( strcmpi(subcmd,"leave") == 0 ) {
		
		if( sub1[0] != '#' ) {
			clif->message(fd, msg_txt(1405));// Channel name must start with a '#'
			return false;
		}
		for(k = 0; k < sd->channel_count; k++) {
			if( strcmpi(sub1+1,sd->channels[k]->name) == 0 )
				break;
		}
		if( k == sd->channel_count ) {
			sprintf(atcmd_output, msg_txt(1425),sub1);// You're not part of the '%s' channel
			clif->message(fd, atcmd_output);
			return false;
		}
		if( sd->channels[k]->type == hChSys_ALLY ) {
			do {
				for(k = 0; k < sd->channel_count; k++) {
					if( sd->channels[k]->type == hChSys_ALLY ) {
						clif->chsys_left(sd->channels[k],sd);
						break;
					}
				}
			} while( k != sd->channel_count );
		} else
			clif->chsys_left(sd->channels[k],sd);
		sprintf(atcmd_output, msg_txt(1426),sub1); // You've left the '%s' channel
		clif->message(fd, atcmd_output);
	} else if ( strcmpi(subcmd,"bindto") == 0 ) {
		
		if( sub1[0] != '#' ) {
			clif->message(fd, msg_txt(1405));// Channel name must start with a '#'
			return false;
		}
		
		for(k = 0; k < sd->channel_count; k++) {
			if( strcmpi(sub1+1,sd->channels[k]->name) == 0 )
				break;
		}
		if( k == sd->channel_count ) {
			sprintf(atcmd_output, msg_txt(1425),sub1);// You're not part of the '%s' channel
			clif->message(fd, atcmd_output);
			return false;
		}
		
		sd->gcbind = sd->channels[k];
		sprintf(atcmd_output, msg_txt(1431),sub1); // Your global chat is now binded to the '%s' channel
		clif->message(fd, atcmd_output);
	} else if ( strcmpi(subcmd,"unbind") == 0 ) {
		
		if( sd->gcbind == NULL ) {
			clif->message(fd, msg_txt(1432));// Your global chat is not binded to any channel
			return false;
		}
		
		sprintf(atcmd_output, msg_txt(1433),sd->gcbind->name); // Your global chat is now unbinded from the '#%s' channel
		clif->message(fd, atcmd_output);
		
		sd->gcbind = NULL;
	} else if ( strcmpi(subcmd,"ban") == 0 ) {
		struct map_session_data *pl_sd = NULL;
		struct hChSysBanEntry *entry = NULL;
		
		if( sub1[0] != '#' ) {
			clif->message(fd, msg_txt(1405));// Channel name must start with a '#'
			return false;
		}
		
		if( !(channel = strdb_get(clif->channel_db, sub1 + 1)) ) {
			sprintf(atcmd_output, msg_txt(1407), sub1);// Channel '%s' is not available
			clif->message(fd, atcmd_output);
			return false;
		}
		
		if( channel->owner != sd->status.char_id && !pc_has_permission(sd, PC_PERM_HCHSYS_ADMIN) ) {
			sprintf(atcmd_output, msg_txt(1412), sub1);// You're not the owner of channel '%s'
			clif->message(fd, atcmd_output);
			return false;
		}
		
		if (!message || !*message || sscanf(message, "%s %s %24[^\n]", subcmd, sub1, sub2) < 1) {
			sprintf(atcmd_output, msg_txt(1434), sub2);// Player '%s' was not found
			clif->message(fd, atcmd_output);
			return false;
		}

		if( sub2[0] == '\0' || ( pl_sd = map->nick2sd(sub2) ) == NULL ) {
			sprintf(atcmd_output, msg_txt(1434), sub2);// Player '%s' was not found
			clif->message(fd, atcmd_output);
			return false;
		 }
		
		if( pc_has_permission(pl_sd, PC_PERM_HCHSYS_ADMIN) ) {
			clif->message(fd, msg_txt(1464)); // Ban failed, not possible to ban this user.
			return false;
		}
		
		if( channel->banned && idb_exists(channel->banned,pl_sd->status.account_id) ) {
			sprintf(atcmd_output, msg_txt(1465), pl_sd->status.name);// Player '%s' is already banned from this channel
			clif->message(fd, atcmd_output);
			return false;
		}
		
		if( !channel->banned )
			channel->banned = idb_alloc(DB_OPT_BASE|DB_OPT_ALLOW_NULL_DATA|DB_OPT_RELEASE_DATA);
		
		CREATE(entry, struct hChSysBanEntry, 1);
		
		safestrncpy(entry->name, pl_sd->status.name, NAME_LENGTH);
		
		idb_put(channel->banned, pl_sd->status.account_id, entry);
		
		clif->chsys_left(channel,pl_sd);
		
		sprintf(atcmd_output, msg_txt(1437),pl_sd->status.name,sub1); // Player '%s' has now been banned from '%s' channel
		clif->message(fd, atcmd_output);
	} else if ( strcmpi(subcmd,"unban") == 0 ) {
		struct map_session_data *pl_sd = NULL;
		
		if( sub1[0] != '#' ) {
			clif->message(fd, msg_txt(1405));// Channel name must start with a '#'
			return false;
		}
		
		if( !(channel = strdb_get(clif->channel_db, sub1 + 1)) ) {
			sprintf(atcmd_output, msg_txt(1407), sub1);// Channel '%s' is not available
			clif->message(fd, atcmd_output);
			return false;
		}
		
		if( channel->owner != sd->status.char_id && !pc_has_permission(sd, PC_PERM_HCHSYS_ADMIN) ) {
			sprintf(atcmd_output, msg_txt(1412), sub1);// You're not the owner of channel '%s'
			clif->message(fd, atcmd_output);
			return false;
		}
		
		if( !channel->banned ) {
			sprintf(atcmd_output, msg_txt(1439), sub1);// Channel '%s' has no banned players
			clif->message(fd, atcmd_output);
			return false;
		}
		
		if( sub2[0] == '\0' || ( pl_sd = map->nick2sd(sub2) ) == NULL ) {
			sprintf(atcmd_output, msg_txt(1434), sub2);// Player '%s' was not found
			clif->message(fd, atcmd_output);
			return false;
		}
		
		if( !idb_exists(channel->banned,pl_sd->status.account_id) ) {
			sprintf(atcmd_output, msg_txt(1440), pl_sd->status.name);// Player '%s' is not banned from this channel
			clif->message(fd, atcmd_output);
			return false;
		}
		
		idb_remove(channel->banned, pl_sd->status.account_id);
		
		if( !db_size(channel->banned) ) {
			db_destroy(channel->banned);
			channel->banned = NULL;
		}
		
		sprintf(atcmd_output, msg_txt(1441),pl_sd->status.name,sub1); // Player '%s' has now been unbanned from the '%s' channel
		clif->message(fd, atcmd_output);
	} else if ( strcmpi(subcmd,"unbanall") == 0 ) {
		if( sub1[0] != '#' ) {
			clif->message(fd, msg_txt(1405));// Channel name must start with a '#'
			return false;
		}
		
		if( !(channel = strdb_get(clif->channel_db, sub1 + 1)) ) {
			sprintf(atcmd_output, msg_txt(1407), sub1);// Channel '%s' is not available
			clif->message(fd, atcmd_output);
			return false;
		}
		
		if( channel->owner != sd->status.char_id && !pc_has_permission(sd, PC_PERM_HCHSYS_ADMIN) ) {
			sprintf(atcmd_output, msg_txt(1412), sub1);// You're not the owner of channel '%s'
			clif->message(fd, atcmd_output);
			return false;
		}
		
		if( !channel->banned ) {
			sprintf(atcmd_output, msg_txt(1439), sub1);// Channel '%s' has no banned players
			clif->message(fd, atcmd_output);
			return false;
		}
		
		db_destroy(channel->banned);
		channel->banned = NULL;
		
		sprintf(atcmd_output, msg_txt(1442),sub1); // Removed all bans from '%s' channel
		clif->message(fd, atcmd_output);
	} else if ( strcmpi(subcmd,"banlist") == 0 ) {
		DBIterator *iter = NULL;
		DBKey key;
		DBData *data;
		bool isA = pc_has_permission(sd, PC_PERM_HCHSYS_ADMIN)?true:false;
		if( sub1[0] != '#' ) {
			clif->message(fd, msg_txt(1405));// Channel name must start with a '#'
			return false;
		}
		
		if( !(channel = strdb_get(clif->channel_db, sub1 + 1)) ) {
			sprintf(atcmd_output, msg_txt(1407), sub1);// Channel '%s' is not available
			clif->message(fd, atcmd_output);
			return false;
		}
		
		if( channel->owner != sd->status.char_id && !isA ) {
			sprintf(atcmd_output, msg_txt(1412), sub1);// You're not the owner of channel '%s'
			clif->message(fd, atcmd_output);
			return false;
		}
		
		if( !channel->banned ) {
			sprintf(atcmd_output, msg_txt(1439), sub1);// Channel '%s' has no banned players
			clif->message(fd, atcmd_output);
			return false;
		}
		sprintf(atcmd_output, msg_txt(1443), channel->name);// -- '%s' ban list
		clif->message(fd, atcmd_output);
		
		iter = db_iterator(channel->banned);
		
		for( data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key) ) {
			struct hChSysBanEntry * entry = DB->data2ptr(data);
			
			if( !isA )
				sprintf(atcmd_output, msg_txt(1444), entry->name);// - %s %s
			else
				sprintf(atcmd_output, msg_txt(1445), entry->name, key.i);// - %s (%d)
			
			clif->message(fd, atcmd_output);
		}
		
		dbi_destroy(iter);
		
	} else if ( strcmpi(subcmd,"setopt") == 0 ) {
		const char* opt_str[3] = {
			"None",
			"JoinAnnounce",
			"MessageDelay",
		};
		
		if( sub1[0] != '#' ) {
			clif->message(fd, msg_txt(1405));// Channel name must start with a '#'
			return false;
		}
		
		if( !(channel = strdb_get(clif->channel_db, sub1 + 1)) ) {
			sprintf(atcmd_output, msg_txt(1407), sub1);// Channel '%s' is not available
			clif->message(fd, atcmd_output);
			return false;
		}
		
		if( channel->owner != sd->status.char_id && !pc_has_permission(sd, PC_PERM_HCHSYS_ADMIN) ) {
			sprintf(atcmd_output, msg_txt(1412), sub1);// You're not the owner of channel '%s'
			clif->message(fd, atcmd_output);
			return false;
		}
		
		if( sub2[0] == '\0' ) {
			clif->message(fd, msg_txt(1446));// You need to input a option
			return false;
		}
		
		for( k = 1; k < 3; k++ ) {
			if( strcmpi(sub2,opt_str[k]) == 0 )
				break;
		}
		
		if( k == 3 ) {
			sprintf(atcmd_output, msg_txt(1447), sub2);// '%s' is not a known channel option
			clif->message(fd, atcmd_output);
			clif->message(fd, msg_txt(1448)); // -- Available options
			for( k = 1; k < 3; k++ ) {
				sprintf(atcmd_output, msg_txt(1444), opt_str[k]);// - '%s'
				clif->message(fd, atcmd_output);
			}
			return false;
		}
		
		if( sub3[0] == '\0' ) {
			if ( k == hChSys_OPT_MSG_DELAY ) {
				sprintf(atcmd_output, msg_txt(1466), opt_str[k]);// For '%s' you need the amount of seconds (from 0 to 10)
				clif->message(fd, atcmd_output);
				return false;
			} else if( channel->opt & k ) {
				sprintf(atcmd_output, msg_txt(1449), opt_str[k],opt_str[k]); // option '%s' is already enabled, if you'd like to disable it type '@channel setopt %s 0'
				clif->message(fd, atcmd_output);
				return false;
			} else {
				channel->opt |= k;
				sprintf(atcmd_output, msg_txt(1450), opt_str[k],channel->name);//option '%s' is now enabled for channel '%s'
				clif->message(fd, atcmd_output);
				return true;
			}
		} else {
			int v = atoi(sub3);
			if( k == hChSys_OPT_MSG_DELAY ) {
				if( v < 0 || v > 10 ) {
					sprintf(atcmd_output, msg_txt(1451), v, opt_str[k]);// value '%d' for option '%s' is out of range (limit is 0-10)
					clif->message(fd, atcmd_output);
					return false;
				}
				if( v == 0 ) {
					channel->opt &=~ k;
					channel->msg_delay = 0;
					sprintf(atcmd_output, msg_txt(1453), opt_str[k],channel->name,v);// option '%s' is now disabled for channel '%s'
					clif->message(fd, atcmd_output);
					return true;
				} else {
					channel->opt |= k;
					channel->msg_delay = v;
					sprintf(atcmd_output, msg_txt(1452), opt_str[k],channel->name,v);// option '%s' is now enabled for channel '%s' with %d seconds
					clif->message(fd, atcmd_output);
					return true;
				}
			} else {
				if( v ) {
					if( channel->opt & k ) {
						sprintf(atcmd_output, msg_txt(1449), opt_str[k],opt_str[k]); // option '%s' is already enabled, if you'd like to disable it type '@channel opt %s 0'
						clif->message(fd, atcmd_output);
						return false;
					} else {
						channel->opt |= k;
						sprintf(atcmd_output, msg_txt(1454), opt_str[k],channel->name);//option '%s' is now enabled for channel '%s'
						clif->message(fd, atcmd_output);
					}
				} else {
					if( !(channel->opt & k) ) {
						sprintf(atcmd_output, msg_txt(1454), opt_str[k],channel->name); // option '%s' is not enabled on channel '%s'
						clif->message(fd, atcmd_output);
						return false;
					} else {
						channel->opt &=~ k;
						sprintf(atcmd_output, msg_txt(1453), opt_str[k],channel->name);// option '%s' is now disabled for channel '%s'
						clif->message(fd, atcmd_output);
						return true;
					}
				}
			}
			
		}
		
	} else {
		atcmd_channel_help(fd,command,( hChSys.allow_user_channel_creation || pc_has_permission(sd, PC_PERM_HCHSYS_ADMIN) ));
	}
	
	return true;
}
/* debug only, delete after */
ACMD(fontcolor) {
	unsigned char k;
	unsigned short msg_len = 1;
	char mout[40];

	if( !message || !*message ) {
		for( k = 0; k < hChSys.colors_count; k++ ) {
			msg_len += sprintf(mout, "[ %s ] : %s",command,hChSys.colors_name[k]);
			
			WFIFOHEAD(fd,msg_len + 12);
			WFIFOW(fd,0) = 0x2C1;
			WFIFOW(fd,2) = msg_len + 12;
			WFIFOL(fd,4) = 0;
			WFIFOL(fd,8) = hChSys.colors[k];
			safestrncpy((char*)WFIFOP(fd,12), mout, msg_len);
			WFIFOSET(fd, msg_len + 12);
		}
		return false;
	}
	
	if( message[0] == '0' ) {
		sd->fontcolor = 0;
		return true;
	}
	
	for( k = 0; k < hChSys.colors_count; k++ ) {
		if( strcmpi(message,hChSys.colors_name[k]) == 0 )
			break;
	}
	if( k == hChSys.colors_count ) {
		sprintf(atcmd_output, msg_txt(1411), message);// Unknown color '%s'
		clif->message(fd, atcmd_output);
		return false;
	}
	
	sd->fontcolor = k + 1;
	msg_len += sprintf(mout, "Color changed to '%s'",hChSys.colors_name[k]);
	
	WFIFOHEAD(fd,msg_len + 12);
	WFIFOW(fd,0) = 0x2C1;
	WFIFOW(fd,2) = msg_len + 12;
	WFIFOL(fd,4) = 0;
	WFIFOL(fd,8) = hChSys.colors[k];
	safestrncpy((char*)WFIFOP(fd,12), mout, msg_len);
	WFIFOSET(fd, msg_len + 12);
	return true;
}
ACMD(searchstore){
	int val = atoi(message);
	
	switch( val ) {
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
ACMD(costume){
	const char* names[] = {
		"Wedding",
		"Xmas",
		"Summer",
		"Hanbok",
#if PACKETVER >= 20131218
		"Oktoberfest",
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
	};
	unsigned short k = 0, len = ARRAYLENGTH(names);
	
	if( !message || !*message ) {
		for( k = 0; k < len; k++ ) {
			if( sd->sc.data[name2id[k]] ) {
				sprintf(atcmd_output,msg_txt(1473),names[k]);//Costume '%s' removed.
				clif->message(sd->fd,atcmd_output);
				status_change_end(&sd->bl,name2id[k],INVALID_TIMER);
				return true;
			}
		}
		
		clif->message(sd->fd,msg_txt(1472));
		for( k = 0; k < len; k++ ) {
			sprintf(atcmd_output,msg_txt(1471),names[k]);//-- %s
			clif->message(sd->fd,atcmd_output);
		}
		return false;
	}
	
	for( k = 0; k < len; k++ ) {
		if( sd->sc.data[name2id[k]] ) {
			sprintf(atcmd_output,msg_txt(1470),names[k]);// You're already with a '%s' costume, type '@costume' to remove it.
			clif->message(sd->fd,atcmd_output);
			return false;
		}
	}
	
	for( k = 0; k < len; k++ ) {
		if( strcmpi(message,names[k]) == 0 )
			break;
	}
	if( k == len ) {
		sprintf(atcmd_output,msg_txt(1469),message);// '%s' is not a known costume
		clif->message(sd->fd,atcmd_output);
		return false;
	}
	
	sc_start(NULL,&sd->bl, name2id[k], 100, 0, -1);
	
	return true;
}
/* for debugging purposes (so users can easily provide us with debug info) */
/* should be trashed as soon as its no longer necessary */
ACMD(skdebug) {
	sprintf(atcmd_output,"second: %d; third: %d",sd->sktree.second,sd->sktree.third);
	clif->message(fd,atcmd_output);
	sprintf(atcmd_output,"pc_calc_skilltree_normalize_job: %d",pc->calc_skilltree_normalize_job(sd));
	clif->message(fd,atcmd_output);
	sprintf(atcmd_output,"change_lv_2nd/3rd: %d/%d",sd->change_level_2nd,sd->change_level_3rd);
	clif->message(fd,atcmd_output);
	sprintf(atcmd_output,"pc_calc_skillpoint:%d",pc->calc_skillpoint(sd));
	clif->message(fd,atcmd_output);
	return true;
}
/**
 * Fills the reference of available commands in atcommand DBMap
 **/
#define ACMD_DEF(x) { #x, atcommand_ ## x, NULL, NULL, NULL, true }
#define ACMD_DEF2(x2, x) { x2, atcommand_ ## x, NULL, NULL, NULL, true }
void atcommand_basecommands(void) {
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
		ACMD_DEF(mute),
		ACMD_DEF(refresh),
		ACMD_DEF(refreshall),
		ACMD_DEF(identify),
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
		ACMD_DEF(reloadquestdb),
		ACMD_DEF(undisguiseguild),
		ACMD_DEF(disguiseguild),
		ACMD_DEF(sizeall),
		ACMD_DEF(sizeguild),
		ACMD_DEF(addperm),
		ACMD_DEF2("rmvperm", addperm),
		ACMD_DEF(unloadnpcfile),
		ACMD_DEF(cart),
		ACMD_DEF(mount2),
		ACMD_DEF(join),
		ACMD_DEF(channel),
		ACMD_DEF(fontcolor),
		ACMD_DEF(searchstore),
		ACMD_DEF(costume),
		ACMD_DEF(skdebug),
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

bool atcommand_add(char *name,AtCommandFunc func, bool replace) {
	AtCommandInfo* cmd;

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
AtCommandInfo* atcommand_exists(const char* name) {
	return strdb_get(atcommand->db, name);
}

AtCommandInfo* get_atcommandinfo_byname(const char *name) {
	AtCommandInfo *cmd;
	if ((cmd = strdb_get(atcommand->db, name)))
		return cmd;
	return NULL;
}

const char* atcommand_checkalias(const char *aliasname) {
	AliasInfo *alias_info = NULL;
	if ((alias_info = (AliasInfo*)strdb_get(atcommand->alias_db, aliasname)) != NULL)
		return alias_info->command->command;
	return aliasname;
}

/// AtCommand suggestion
void atcommand_get_suggestions(struct map_session_data* sd, const char *name, bool is_atcmd_cmd) {
	DBIterator* atcommand_iter;
	DBIterator* alias_iter;
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
		strcpy(buffer, msg_txt(205));
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
 * Executes an at-command
 * @param fd             fd associated to the invoking character
 * @param sd             sd associated to the invoking character
 * @param message        atcommand arguments
 * @param player_invoked true if the command was invoked by a player, false if invoked by the server (bypassing any restrictions)
 */
bool atcommand_exec(const int fd, struct map_session_data *sd, const char *message, bool player_invoked) {
	char charname[NAME_LENGTH], params[100];
	char charname2[NAME_LENGTH], params2[100];
	char command[100];
	char output[CHAT_SIZE_MAX];

	//Reconstructed message
	char atcmd_msg[CHAT_SIZE_MAX];

	TBL_PC * ssd = NULL; //sd for target
	AtCommandInfo * info;

	nullpo_retr(false, sd);

	//Shouldn't happen
	if ( !message || !*message )
		return false;

	//Block NOCHAT but do not display it as a normal message
	if ( sd->sc.data[SC_NOCHAT] && sd->sc.data[SC_NOCHAT]->val1&MANNER_NOCOMMAND )
		return true;

	// skip 10/11-langtype's codepage indicator, if detected
	if ( message[0] == '|' && strlen(message) >= 4 && (message[3] == atcommand->at_symbol || message[3] == atcommand->char_symbol) )
		message += 3;

	//Should display as a normal message
	if ( *message != atcommand->at_symbol && *message != atcommand->char_symbol )
		return false;

	if (player_invoked) {
		//Commands are disabled on maps flagged as 'nocommand'
		if ( map->list[sd->bl.m].nocommand && pc_get_group_level(sd) < map->list[sd->bl.m].nocommand ) {
			clif->message(fd, msg_txt(143));
			return false;
		}
	}

	if (*message == atcommand->char_symbol) {
		do {
			int x, y, z;

			//Checks to see if #command has a name or a name + parameters.
			x = sscanf(message, "%99s \"%23[^\"]\" %99[^\n]", command, charname, params);
			y = sscanf(message, "%99s %23s %99[^\n]", command, charname2, params2);

			//z always has the value of the scan that was successful
			z = ( x > 1 ) ? x : y;

			//#command + name means the sufficient target was used and anything else after
			//can be looked at by the actual command function since most scan to see if the
			//right parameters are used.
			if ( x > 2 ) {
				sprintf(atcmd_msg, "%s %s", command, params);
				break;
			}
			else if ( y > 2 ) {
				sprintf(atcmd_msg, "%s %s", command, params2);
				break;
			}
			//Regardless of what style the #command is used, if it's correct, it will always have
			//this value if there is no parameter. Send it as just the #command
			else if ( z == 2 ) {
				sprintf(atcmd_msg, "%s", command);
				break;
			}

			if( !pc_get_group_level(sd) ) {
				if( x >= 1 || y >= 1 ) { /* we have command */
					info = atcommand->get_info_byname(atcommand->check_alias(command + 1));
					if( !info || info->char_groups[pcg->get_idx(sd->group)] == 0 ) /* if we can't use or doesn't exist: don't even display the command failed message */
							return false;
				} else
					return false;/* display as normal message */
			}

			sprintf(output, msg_txt(1388), atcommand->char_symbol); // Charcommand failed (usage: %c<command> <char name> <parameters>).
			clif->message(fd, output);
			return true;
		} while(0);
	}
	else /*if (*message == atcommand->at_symbol)*/ {
		//atcmd_msg is constructed above differently for charcommands
		//it's copied from message if not a charcommand so it can
		//pass through the rest of the code compatible with both symbols
		sprintf(atcmd_msg, "%s", message);
	}

	if( battle_config.idletime_criteria & BCIDLE_ATCOMMAND )
		sd->idletime = sockt->last_tick;
	
	//Clearing these to be used once more.
	memset(command, '\0', sizeof(command));
	memset(params, '\0', sizeof(params));

	//check to see if any params exist within this command
	if( sscanf(atcmd_msg, "%99s %99[^\n]", command, params) < 2 )
		params[0] = '\0';

	// @commands (script based)
	if(player_invoked && atcommand->binding_count > 0) {
		struct atcmd_binding_data * binding;

		// Get atcommand binding
		binding = atcommand->get_bind_byname(command);

		// Check if the binding isn't NULL and there is a NPC event, level of usage met, et cetera
		if( binding != NULL
		 && binding->npc_event[0]
		 && (
		       (*atcmd_msg == atcommand->at_symbol && pc_get_group_level(sd) >= binding->group_lv)
		    || (*atcmd_msg == atcommand->char_symbol && pc_get_group_level(sd) >= binding->group_lv_char)
		    )
		) {
			// Check if self or character invoking; if self == character invoked, then self invoke.
			bool invokeFlag = ((*atcmd_msg == atcommand->at_symbol) ? 1 : 0);
			
			// Check if the command initiated is a character command
			if (*message == atcommand->char_symbol
			 && (ssd = map->nick2sd(charname)) == NULL
			 && (ssd = map->nick2sd(charname2)) == NULL
			) {
				sprintf(output, msg_txt(1389), command); // %s failed. Player not found.
				clif->message(fd, output);
				return true;
			}
			
			if( binding->log ) /* log only if this command should be logged [Ind/Hercules] */
				logs->atcommand(sd, atcmd_msg);
			
			npc->do_atcmd_event((invokeFlag ? sd : ssd), command, params, binding->npc_event);
			return true;
		}
	}

	//Grab the command information and check for the proper GM level required to use it or if the command exists
	info = atcommand->get_info_byname(atcommand->check_alias(command + 1));
	if (info == NULL) {
		if( pc_get_group_level(sd) ) { // TODO: remove or replace with proper permission
			sprintf(output, msg_txt(153), command); // "%s is Unknown Command."
			clif->message(fd, output);
			atcommand->get_suggestions(sd, command + 1, *message == atcommand->at_symbol);
			return true;
		} else
			return false;
	}

	if (player_invoked) {
		int i;
		if ((*command == atcommand->at_symbol && info->at_groups[pcg->get_idx(sd->group)] == 0) ||
		    (*command == atcommand->char_symbol && info->char_groups[pcg->get_idx(sd->group)] == 0) ) {
			return false;
		}
		if( pc_isdead(sd) && pc_has_permission(sd,PC_PERM_DISABLE_CMD_DEAD) ) {
			clif->message(fd, msg_txt(1393)); // You can't use commands while dead
			return true;
		}
		for(i = 0; i < map->list[sd->bl.m].zone->disabled_commands_count; i++) {
			if( info->func == map->list[sd->bl.m].zone->disabled_commands[i]->cmd ) {
				if( pc_get_group_level(sd) < map->list[sd->bl.m].zone->disabled_commands[i]->group_lv ) {
					clif->colormes(sd->fd,COLOR_RED,"This command is disabled in this area");
					return true;
				} else
					break;/* already found the matching command, no need to keep checking -- just go on */
			}
		}
	}
	
	// Check if target is valid only if confirmed that player can use command.
	if (*message == atcommand->char_symbol
	 && (ssd = map->nick2sd(charname)) == NULL
	 && (ssd = map->nick2sd(charname2)) == NULL
	) {
		sprintf(output, msg_txt(1389), command); // %s failed. Player not found.
		clif->message(fd, output);
		return true;
	}

	//Attempt to use the command
	if ( (info->func(fd, (*atcmd_msg == atcommand->at_symbol) ? sd : ssd, command, params,info) != true) ) {
#ifdef AUTOTRADE_PERSISTENCY
		// Autotrade was successful if standalone is set
		if( ((*atcmd_msg == atcommand->at_symbol) ? sd->state.standalone : ssd->state.standalone) )
			return true;
#endif
		sprintf(output,msg_txt(154), command); // %s failed.
		clif->message(fd, output);
		return true;
	}

	if( info->log ) /* log only if this command should be logged [Ind/Hercules] */
		logs->atcommand(sd, *atcmd_msg == atcommand->at_symbol ? atcmd_msg : message);

	return true;
}

/*==========================================
 *
 *------------------------------------------*/
void atcommand_config_read(const char* config_filename) {
	config_t atcommand_config;
	config_setting_t *aliases = NULL, *help = NULL, *nolog = NULL;
	const char *symbol = NULL;
	int num_aliases = 0;

	if (libconfig->read_file(&atcommand_config, config_filename))
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
			config_setting_t *command;
			const char *commandname = NULL;
			int j = 0, alias_count = 0;
			AtCommandInfo *commandinfo = NULL;

			command = libconfig->setting_get_elem(aliases, i);
			if (config_setting_type(command) != CONFIG_TYPE_ARRAY)
				continue;
			commandname = config_setting_name(command);
			if ( !( commandinfo = atcommand_exists(commandname) ) ) {
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
			config_setting_t *command;
			const char *commandname = NULL;
			AtCommandInfo *commandinfo = NULL;
			
			command = libconfig->setting_get_elem(nolog, i);
			commandname = config_setting_name(command);
			if ( !( commandinfo = atcommand_exists(commandname) ) ) {
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
			config_setting_t *command;
			const char *commandname;
			AtCommandInfo *commandinfo = NULL;

			command = libconfig->setting_get_elem(help, i);
			commandname = config_setting_name(command);
			if ( !( commandinfo = atcommand_exists(commandname) ) )
				ShowConfigWarning(command, "atcommand_config_read: command %s does not exist", commandname);
			else {
				if( commandinfo->help == NULL ) {
					const char *str = libconfig->setting_get_string(command);
					size_t len = strlen(str);
					commandinfo->help = aMalloc( len * sizeof(char) );
					safestrncpy(commandinfo->help, str, len);
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
static inline int AtCommandType2idx(AtCommandType type) { return (type-1); }

/**
 * Loads permissions for groups to use commands.
 *
 */
void atcommand_db_load_groups(GroupSettings **groups, config_setting_t **commands_, size_t sz)
{
	DBIterator *iter = db_iterator(atcommand->db);
	AtCommandInfo *atcmd;

	for (atcmd = dbi_first(iter); dbi_exists(iter); atcmd = dbi_next(iter)) {
		int i;
		CREATE(atcmd->at_groups, char, sz);
		CREATE(atcmd->char_groups, char, sz);
		for (i = 0; i < sz; i++) {
			GroupSettings *group = groups[i];
			config_setting_t *commands = commands_[i];
			int result = 0;
			int idx = -1;
			
			if (group == NULL) {
				ShowError("atcommand_db_load_groups: group is NULL\n");
				continue;
			}

			idx = pcg->get_idx(group);
			if (idx < 0 || idx >= sz) {
				ShowError("atcommand_db_load_groups: index (%d) out of bounds [0,%d]\n", idx, sz - 1);
				continue;
			}
			
			if (pcg->has_permission(group, PC_PERM_USE_ALL_COMMANDS)) {
				atcmd->at_groups[idx] = atcmd->char_groups[idx] = 1;
				continue;
			}
			
			if (commands != NULL) {
				config_setting_t *cmd = NULL;

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
					if (config_setting_get_bool_elem(cmd, AtCommandType2idx(COMMAND_ATCOMMAND))) {
						atcmd->at_groups[idx] = 1;
					}
					if (config_setting_get_bool_elem(cmd, AtCommandType2idx(COMMAND_CHARCOMMAND))) {
						atcmd->char_groups[idx] = 1;
					}
				}
			}
		}
	}
	dbi_destroy(iter);
	return;
}

bool atcommand_can_use(struct map_session_data *sd, const char *command) {
	AtCommandInfo *info = atcommand->get_info_byname(atcommand->check_alias(command + 1));
	
	if (info == NULL)
		return false;
	
	if ((*command == atcommand->at_symbol && info->at_groups[pcg->get_idx(sd->group)] != 0) ||
		(*command == atcommand->char_symbol && info->char_groups[pcg->get_idx(sd->group)] != 0) ) {
		return true;
	}
	
	return false;
}
bool atcommand_can_use2(struct map_session_data *sd, const char *command, AtCommandType type) {
	AtCommandInfo *info = atcommand->get_info_byname(atcommand->check_alias(command));
	
	if (info == NULL)
		return false;
	
	if ((type == COMMAND_ATCOMMAND && info->at_groups[pcg->get_idx(sd->group)] != 0) ||
		(type == COMMAND_CHARCOMMAND && info->char_groups[pcg->get_idx(sd->group)] != 0) ) {
		return true;
	}
	
	return false;
}
bool atcommand_hp_add(char *name, AtCommandFunc func) {
	/* if commands are added after group permissions are thrown in, they end up with no permissions */
	/* so we restrict commands to be linked in during boot */
	if( runflag == MAPSERVER_ST_RUNNING ) {
		ShowDebug("atcommand_hp_add: Commands can't be added after server is ready, skipping '%s'...\n",name);
		return false;
	}
	
	return HPM_map_add_atcommand(name,func);
}

/**
 * @see DBApply
 */
int atcommand_db_clear_sub(DBKey key, DBData *data, va_list args) {
	AtCommandInfo *cmd = DB->data2ptr(data);
	aFree(cmd->at_groups);
	aFree(cmd->char_groups);
	if (cmd->help != NULL)
		aFree(cmd->help);
	return 0;
}

void atcommand_db_clear(void) {
	if( atcommand->db != NULL ) {
		atcommand->db->destroy(atcommand->db, atcommand->cmd_db_clear_sub);
		atcommand->db = NULL;
	}
	if( atcommand->alias_db != NULL ) {
		db_destroy(atcommand->alias_db);
		atcommand->alias_db = NULL;
	}
}

void atcommand_doload(void) {
	if( runflag >= MAPSERVER_ST_RUNNING )
		atcommand->cmd_db_clear();
	if( atcommand->db == NULL )
		atcommand->db = stridb_alloc(DB_OPT_DUP_KEY|DB_OPT_RELEASE_DATA, ATCOMMAND_LENGTH);
	if( atcommand->alias_db == NULL )
		atcommand->alias_db = stridb_alloc(DB_OPT_DUP_KEY|DB_OPT_RELEASE_DATA, ATCOMMAND_LENGTH);
	atcommand->base_commands(); //fills initial atcommand_db with known commands
	atcommand->config_read(map->ATCOMMAND_CONF_FILENAME);
}

void do_init_atcommand(bool minimal) {
	if (minimal)
		return;

	atcommand->at_symbol = '@';
	atcommand->char_symbol = '#';
	atcommand->binding_count = 0;
	
	atcommand->doload();
}

void do_final_atcommand(void) {
	atcommand->cmd_db_clear();
}

void atcommand_defaults(void) {
	atcommand = &atcommand_s;
	
	atcommand->db = NULL;
	atcommand->alias_db = NULL;
	
	memset(atcommand->msg_table, 0, sizeof(atcommand->msg_table));
	
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
	atcommand->cmd_db_clear = atcommand_db_clear;
	atcommand->cmd_db_clear_sub = atcommand_db_clear_sub;
	atcommand->doload = atcommand_doload;
	atcommand->base_commands = atcommand_basecommands;
	atcommand->add = atcommand_add;
	atcommand->msg = atcommand_msg;
}
