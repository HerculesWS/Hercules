// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/nullpo.h"
#include "../common/malloc.h"
#include "../common/strlib.h"
#include "map.h"
#include "battle.h"
#include "chrif.h"
#include "clif.h"
#include "pc.h"
#include "intif.h"
#include "log.h"
#include "storage.h"
#include "party.h"
#include "guild.h"
#include "pet.h"
#include "atcommand.h"
#include "mercenary.h"
#include "homunculus.h"
#include "elemental.h"
#include "mail.h"
#include "quest.h"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>


struct intif_interface intif_s;

#define inter_fd (chrif->fd) // alias

//-----------------------------------------------------------------
// Send to inter server

int CheckForCharServer(void)
{
	return ((chrif->fd <= 0) || session[chrif->fd] == NULL || session[chrif->fd]->wdata == NULL);
}

// pet
int intif_create_pet(int account_id,int char_id,short pet_class,short pet_lv,short pet_egg_id,
	short pet_equip,short intimate,short hungry,char rename_flag,char incuvate,char *pet_name)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, 24 + NAME_LENGTH);
	WFIFOW(inter_fd,0) = 0x3080;
	WFIFOL(inter_fd,2) = account_id;
	WFIFOL(inter_fd,6) = char_id;
	WFIFOW(inter_fd,10) = pet_class;
	WFIFOW(inter_fd,12) = pet_lv;
	WFIFOW(inter_fd,14) = pet_egg_id;
	WFIFOW(inter_fd,16) = pet_equip;
	WFIFOW(inter_fd,18) = intimate;
	WFIFOW(inter_fd,20) = hungry;
	WFIFOB(inter_fd,22) = rename_flag;
	WFIFOB(inter_fd,23) = incuvate;
	memcpy(WFIFOP(inter_fd,24),pet_name,NAME_LENGTH);
	WFIFOSET(inter_fd,24+NAME_LENGTH);

	return 0;
}

int intif_request_petdata(int account_id,int char_id,int pet_id)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, 14);
	WFIFOW(inter_fd,0) = 0x3081;
	WFIFOL(inter_fd,2) = account_id;
	WFIFOL(inter_fd,6) = char_id;
	WFIFOL(inter_fd,10) = pet_id;
	WFIFOSET(inter_fd,14);

	return 0;
}

int intif_save_petdata(int account_id,struct s_pet *p)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, sizeof(struct s_pet) + 8);
	WFIFOW(inter_fd,0) = 0x3082;
	WFIFOW(inter_fd,2) = sizeof(struct s_pet) + 8;
	WFIFOL(inter_fd,4) = account_id;
	memcpy(WFIFOP(inter_fd,8),p,sizeof(struct s_pet));
	WFIFOSET(inter_fd,WFIFOW(inter_fd,2));

	return 0;
}

int intif_delete_petdata(int pet_id)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x3083;
	WFIFOL(inter_fd,2) = pet_id;
	WFIFOSET(inter_fd,6);

	return 1;
}

int intif_rename(struct map_session_data *sd, int type, char *name)
{
	if (intif->CheckForCharServer())
		return 1;

	WFIFOHEAD(inter_fd,NAME_LENGTH+12);
	WFIFOW(inter_fd,0) = 0x3006;
	WFIFOL(inter_fd,2) = sd->status.account_id;
	WFIFOL(inter_fd,6) = sd->status.char_id;
	WFIFOB(inter_fd,10) = type;  //Type: 0 - PC, 1 - PET, 2 - HOM
	memcpy(WFIFOP(inter_fd,11),name, NAME_LENGTH);
	WFIFOSET(inter_fd,NAME_LENGTH+12);
	return 0;
}

// GM Send a message
int intif_broadcast(const char* mes, int len, int type)
{
	int lp = (type|BC_COLOR_MASK) ? 4 : 0;

	// Send to the local players
	clif->broadcast(NULL, mes, len, type, ALL_CLIENT);

	if (intif->CheckForCharServer())
		return 0;

	if (chrif->other_mapserver_count < 1)
		return 0; //No need to send.

	WFIFOHEAD(inter_fd, 16 + lp + len);
	WFIFOW(inter_fd,0)  = 0x3000;
	WFIFOW(inter_fd,2)  = 16 + lp + len;
	WFIFOL(inter_fd,4)  = 0xFF000000; // 0xFF000000 color signals standard broadcast
	WFIFOW(inter_fd,8)  = 0; // fontType not used with standard broadcast
	WFIFOW(inter_fd,10) = 0; // fontSize not used with standard broadcast
	WFIFOW(inter_fd,12) = 0; // fontAlign not used with standard broadcast
	WFIFOW(inter_fd,14) = 0; // fontY not used with standard broadcast
	if( type|BC_BLUE )
		WFIFOL(inter_fd,16) = 0x65756c62; //If there's "blue" at the beginning of the message, game client will display it in blue instead of yellow.
	else if( type|BC_WOE )
		WFIFOL(inter_fd,16) = 0x73737373; //If there's "ssss", game client will recognize message as 'WoE broadcast'.
	memcpy(WFIFOP(inter_fd,16 + lp), mes, len);
	WFIFOSET(inter_fd, WFIFOW(inter_fd,2));
	return 0;
}

int intif_broadcast2(const char* mes, int len, unsigned long fontColor, short fontType, short fontSize, short fontAlign, short fontY)
{
	// Send to the local players
	clif->broadcast2(NULL, mes, len, fontColor, fontType, fontSize, fontAlign, fontY, ALL_CLIENT);

	if (intif->CheckForCharServer())
		return 0;

	if (chrif->other_mapserver_count < 1)
		return 0; //No need to send.

	WFIFOHEAD(inter_fd, 16 + len);
	WFIFOW(inter_fd,0)  = 0x3000;
	WFIFOW(inter_fd,2)  = 16 + len;
	WFIFOL(inter_fd,4)  = fontColor;
	WFIFOW(inter_fd,8)  = fontType;
	WFIFOW(inter_fd,10) = fontSize;
	WFIFOW(inter_fd,12) = fontAlign;
	WFIFOW(inter_fd,14) = fontY;
	memcpy(WFIFOP(inter_fd,16), mes, len);
	WFIFOSET(inter_fd, WFIFOW(inter_fd,2));
	return 0;
}

/// send a message using the main chat system
/// <sd>         the source of message
/// <message>    the message that was sent
int intif_main_message(struct map_session_data* sd, const char* message)
{
	char output[256];

	nullpo_ret(sd);

	// format the message for main broadcasting
	snprintf( output, sizeof(output), msg_txt(386), sd->status.name, message );

	// send the message using the inter-server broadcast service
	intif_broadcast2( output, strlen(output) + 1, 0xFE000000, 0, 0, 0, 0 );

	// log the chat message
	logs->chat( LOG_CHAT_MAINCHAT, 0, sd->status.char_id, sd->status.account_id, mapindex_id2name(sd->mapindex), sd->bl.x, sd->bl.y, NULL, message );

	return 0;
}

// The transmission of Wisp/Page to inter-server (player not found on this server)
int intif_wis_message(struct map_session_data *sd, char *nick, char *mes, int mes_len)
{
	nullpo_ret(sd);
	if (intif->CheckForCharServer())
		return 0;

	if (chrif->other_mapserver_count < 1)
	{	//Character not found.
		clif->wis_end(sd->fd, 1);
		return 0;
	}

	WFIFOHEAD(inter_fd,mes_len + 52);
	WFIFOW(inter_fd,0) = 0x3001;
	WFIFOW(inter_fd,2) = mes_len + 52;
	memcpy(WFIFOP(inter_fd,4), sd->status.name, NAME_LENGTH);
	memcpy(WFIFOP(inter_fd,4+NAME_LENGTH), nick, NAME_LENGTH);
	memcpy(WFIFOP(inter_fd,4+2*NAME_LENGTH), mes, mes_len);
	WFIFOSET(inter_fd, WFIFOW(inter_fd,2));

	if (battle_config.etc_log)
		ShowInfo("intif_wis_message from %s to %s (message: '%s')\n", sd->status.name, nick, mes);

	return 0;
}

// The reply of Wisp/page
int intif_wis_replay(int id, int flag)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,7);
	WFIFOW(inter_fd,0) = 0x3002;
	WFIFOL(inter_fd,2) = id;
	WFIFOB(inter_fd,6) = flag; // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
	WFIFOSET(inter_fd,7);

	if (battle_config.etc_log)
		ShowInfo("intif_wis_replay: id: %d, flag:%d\n", id, flag);

	return 0;
}

// The transmission of GM only Wisp/Page from server to inter-server
int intif_wis_message_to_gm(char *wisp_name, int permission, char *mes)
{
	int mes_len;
	if (intif->CheckForCharServer())
		return 0;
	mes_len = strlen(mes) + 1; // + null
	WFIFOHEAD(inter_fd, mes_len + 32);
	WFIFOW(inter_fd,0) = 0x3003;
	WFIFOW(inter_fd,2) = mes_len + 32;
	memcpy(WFIFOP(inter_fd,4), wisp_name, NAME_LENGTH);
	WFIFOL(inter_fd,4+NAME_LENGTH) = permission;
	memcpy(WFIFOP(inter_fd,8+NAME_LENGTH), mes, mes_len);
	WFIFOSET(inter_fd, WFIFOW(inter_fd,2));

	if (battle_config.etc_log)
		ShowNotice("intif_wis_message_to_gm: from: '%s', required permission: %d, message: '%s'.\n", wisp_name, permission, mes);

	return 0;
}

int intif_regtostr(char* str, struct global_reg *reg, int qty)
{
	int len =0, i;

	for (i = 0; i < qty; i++) {
		len+= sprintf(str+len, "%s", reg[i].str)+1; //We add 1 to consider the '\0' in place.
		len+= sprintf(str+len, "%s", reg[i].value)+1;
	}
	return len;
}

//Request for saving registry values.
int intif_saveregistry(struct map_session_data *sd, int type)
{
	struct global_reg *reg;
	int count;
	int i, p;

	if (intif->CheckForCharServer())
		return -1;

	switch (type) {
	case 3: //Character reg
		reg = sd->save_reg.global;
		count = sd->save_reg.global_num;
		sd->state.reg_dirty &= ~0x4;
	break;
	case 2: //Account reg
		reg = sd->save_reg.account;
		count = sd->save_reg.account_num;
		sd->state.reg_dirty &= ~0x2;
	break;
	case 1: //Account2 reg
		reg = sd->save_reg.account2;
		count = sd->save_reg.account2_num;
		sd->state.reg_dirty &= ~0x1;
	break;
	default: //Broken code?
		ShowError("intif_saveregistry: Invalid type %d\n", type);
		return -1;
	}
	WFIFOHEAD(inter_fd, 288 * MAX_REG_NUM+13);
	WFIFOW(inter_fd,0)=0x3004;
	WFIFOL(inter_fd,4)=sd->status.account_id;
	WFIFOL(inter_fd,8)=sd->status.char_id;
	WFIFOB(inter_fd,12)=type;
	for( p = 13, i = 0; i < count; i++ ) {
		if (reg[i].str[0] != '\0' && reg[i].value[0] != '\0') {
			p+= sprintf((char*)WFIFOP(inter_fd,p), "%s", reg[i].str)+1; //We add 1 to consider the '\0' in place.
			p+= sprintf((char*)WFIFOP(inter_fd,p), "%s", reg[i].value)+1;
		}
	}
	WFIFOW(inter_fd,2)=p;
	WFIFOSET(inter_fd,WFIFOW(inter_fd,2));
	return 0;
}

//Request the registries for this player.
int intif_request_registry(struct map_session_data *sd, int flag)
{
	nullpo_ret(sd);

	sd->save_reg.account2_num = -1;
	sd->save_reg.account_num = -1;
	sd->save_reg.global_num = -1;

	if (intif->CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x3005;
	WFIFOL(inter_fd,2) = sd->status.account_id;
	WFIFOL(inter_fd,6) = sd->status.char_id;
	WFIFOB(inter_fd,10) = (flag&1?1:0); //Request Acc Reg 2
	WFIFOB(inter_fd,11) = (flag&2?1:0); //Request Acc Reg
	WFIFOB(inter_fd,12) = (flag&4?1:0); //Request Char Reg
	WFIFOSET(inter_fd,13);

	return 0;
}

int intif_request_guild_storage(int account_id,int guild_id)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x3018;
	WFIFOL(inter_fd,2) = account_id;
	WFIFOL(inter_fd,6) = guild_id;
	WFIFOSET(inter_fd,10);
	return 0;
}
int intif_send_guild_storage(int account_id,struct guild_storage *gstor)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,sizeof(struct guild_storage)+12);
	WFIFOW(inter_fd,0) = 0x3019;
	WFIFOW(inter_fd,2) = (unsigned short)sizeof(struct guild_storage)+12;
	WFIFOL(inter_fd,4) = account_id;
	WFIFOL(inter_fd,8) = gstor->guild_id;
	memcpy( WFIFOP(inter_fd,12),gstor, sizeof(struct guild_storage) );
	WFIFOSET(inter_fd,WFIFOW(inter_fd,2));
	return 0;
}

// Party creation request
int intif_create_party(struct party_member *member,char *name,int item,int item2)
{
	if (intif->CheckForCharServer())
		return 0;
	nullpo_ret(member);

	WFIFOHEAD(inter_fd,64);
	WFIFOW(inter_fd,0) = 0x3020;
	WFIFOW(inter_fd,2) = 30+sizeof(struct party_member);
	memcpy(WFIFOP(inter_fd,4),name, NAME_LENGTH);
	WFIFOB(inter_fd,28)= item;
	WFIFOB(inter_fd,29)= item2;
	memcpy(WFIFOP(inter_fd,30), member, sizeof(struct party_member));
	WFIFOSET(inter_fd,WFIFOW(inter_fd, 2));
	return 0;
}

// Party information request
int intif_request_partyinfo(int party_id, int char_id)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x3021;
	WFIFOL(inter_fd,2) = party_id;
	WFIFOL(inter_fd,6) = char_id;
	WFIFOSET(inter_fd,10);
	return 0;
}

// Request to add a member to party
int intif_party_addmember(int party_id,struct party_member *member)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,42);
	WFIFOW(inter_fd,0)=0x3022;
	WFIFOW(inter_fd,2)=8+sizeof(struct party_member);
	WFIFOL(inter_fd,4)=party_id;
	memcpy(WFIFOP(inter_fd,8),member,sizeof(struct party_member));
	WFIFOSET(inter_fd,WFIFOW(inter_fd, 2));
	return 1;
}

// Request to change party configuration (exp,item share)
int intif_party_changeoption(int party_id,int account_id,int exp,int item)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,14);
	WFIFOW(inter_fd,0)=0x3023;
	WFIFOL(inter_fd,2)=party_id;
	WFIFOL(inter_fd,6)=account_id;
	WFIFOW(inter_fd,10)=exp;
	WFIFOW(inter_fd,12)=item;
	WFIFOSET(inter_fd,14);
	return 0;
}

// Request to leave party
int intif_party_leave(int party_id,int account_id, int char_id)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,14);
	WFIFOW(inter_fd,0)=0x3024;
	WFIFOL(inter_fd,2)=party_id;
	WFIFOL(inter_fd,6)=account_id;
	WFIFOL(inter_fd,10)=char_id;
	WFIFOSET(inter_fd,14);
	return 0;
}

// Request keeping party for new map ??
int intif_party_changemap(struct map_session_data *sd,int online) {
	int16 m, mapindex;

	if (intif->CheckForCharServer())
		return 0;
	if(!sd)
		return 0;

	if( (m=map->mapindex2mapid(sd->mapindex)) >= 0 && map->list[m].instance_id >= 0 )
		mapindex = map_id2index(map->list[m].instance_src_map);
	else
		mapindex = sd->mapindex;

	WFIFOHEAD(inter_fd,19);
	WFIFOW(inter_fd,0)=0x3025;
	WFIFOL(inter_fd,2)=sd->status.party_id;
	WFIFOL(inter_fd,6)=sd->status.account_id;
	WFIFOL(inter_fd,10)=sd->status.char_id;
	WFIFOW(inter_fd,14)=mapindex;
	WFIFOB(inter_fd,16)=online;
	WFIFOW(inter_fd,17)=sd->status.base_level;
	WFIFOSET(inter_fd,19);
	return 1;
}

// Request breaking party
int intif_break_party(int party_id)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0)=0x3026;
	WFIFOL(inter_fd,2)=party_id;
	WFIFOSET(inter_fd,6);
	return 0;
}

// Sending party chat
int intif_party_message(int party_id,int account_id,const char *mes,int len)
{
	if (intif->CheckForCharServer())
		return 0;

	if (chrif->other_mapserver_count < 1)
		return 0; //No need to send.

	WFIFOHEAD(inter_fd,len + 12);
	WFIFOW(inter_fd,0)=0x3027;
	WFIFOW(inter_fd,2)=len+12;
	WFIFOL(inter_fd,4)=party_id;
	WFIFOL(inter_fd,8)=account_id;
	memcpy(WFIFOP(inter_fd,12),mes,len);
	WFIFOSET(inter_fd,len+12);
	return 0;
}

// Request a new leader for party
int intif_party_leaderchange(int party_id,int account_id,int char_id)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,14);
	WFIFOW(inter_fd,0)=0x3029;
	WFIFOL(inter_fd,2)=party_id;
	WFIFOL(inter_fd,6)=account_id;
	WFIFOL(inter_fd,10)=char_id;
	WFIFOSET(inter_fd,14);
	return 0;
}

// Request a Guild creation
int intif_guild_create(const char *name,const struct guild_member *master)
{
	if (intif->CheckForCharServer())
		return 0;
	nullpo_ret(master);

	WFIFOHEAD(inter_fd,sizeof(struct guild_member)+(8+NAME_LENGTH));
	WFIFOW(inter_fd,0)=0x3030;
	WFIFOW(inter_fd,2)=sizeof(struct guild_member)+(8+NAME_LENGTH);
	WFIFOL(inter_fd,4)=master->account_id;
	memcpy(WFIFOP(inter_fd,8),name,NAME_LENGTH);
	memcpy(WFIFOP(inter_fd,8+NAME_LENGTH),master,sizeof(struct guild_member));
	WFIFOSET(inter_fd,WFIFOW(inter_fd,2));
	return 0;
}

// Request Guild information
int intif_guild_request_info(int guild_id)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x3031;
	WFIFOL(inter_fd,2) = guild_id;
	WFIFOSET(inter_fd,6);
	return 0;
}

// Request to add member to the guild
int intif_guild_addmember(int guild_id,struct guild_member *m)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,sizeof(struct guild_member)+8);
	WFIFOW(inter_fd,0) = 0x3032;
	WFIFOW(inter_fd,2) = sizeof(struct guild_member)+8;
	WFIFOL(inter_fd,4) = guild_id;
	memcpy(WFIFOP(inter_fd,8),m,sizeof(struct guild_member));
	WFIFOSET(inter_fd,WFIFOW(inter_fd,2));
	return 0;
}

// Request a new leader for guild
int intif_guild_change_gm(int guild_id, const char* name, int len)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, len + 8);
	WFIFOW(inter_fd, 0)=0x3033;
	WFIFOW(inter_fd, 2)=len+8;
	WFIFOL(inter_fd, 4)=guild_id;
	memcpy(WFIFOP(inter_fd,8),name,len);
	WFIFOSET(inter_fd,len+8);
	return 0;
}

// Request to leave guild
int intif_guild_leave(int guild_id,int account_id,int char_id,int flag,const char *mes)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, 55);
	WFIFOW(inter_fd, 0) = 0x3034;
	WFIFOL(inter_fd, 2) = guild_id;
	WFIFOL(inter_fd, 6) = account_id;
	WFIFOL(inter_fd,10) = char_id;
	WFIFOB(inter_fd,14) = flag;
	safestrncpy((char*)WFIFOP(inter_fd,15),mes,40);
	WFIFOSET(inter_fd,55);
	return 0;
}

//Update request / Lv online status of the guild members
int intif_guild_memberinfoshort(int guild_id,int account_id,int char_id,int online,int lv,int class_)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, 19);
	WFIFOW(inter_fd, 0) = 0x3035;
	WFIFOL(inter_fd, 2) = guild_id;
	WFIFOL(inter_fd, 6) = account_id;
	WFIFOL(inter_fd,10) = char_id;
	WFIFOB(inter_fd,14) = online;
	WFIFOW(inter_fd,15) = lv;
	WFIFOW(inter_fd,17) = class_;
	WFIFOSET(inter_fd,19);
	return 0;
}

//Guild disbanded notification
int intif_guild_break(int guild_id)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, 6);
	WFIFOW(inter_fd, 0) = 0x3036;
	WFIFOL(inter_fd, 2) = guild_id;
	WFIFOSET(inter_fd,6);
	return 0;
}

// Send a guild message
int intif_guild_message(int guild_id,int account_id,const char *mes,int len)
{
	if (intif->CheckForCharServer())
		return 0;

	if (chrif->other_mapserver_count < 1)
		return 0; //No need to send.

	WFIFOHEAD(inter_fd, len + 12);
	WFIFOW(inter_fd,0)=0x3037;
	WFIFOW(inter_fd,2)=len+12;
	WFIFOL(inter_fd,4)=guild_id;
	WFIFOL(inter_fd,8)=account_id;
	memcpy(WFIFOP(inter_fd,12),mes,len);
	WFIFOSET(inter_fd,len+12);

	return 0;
}

// Request a change of Guild basic information
int intif_guild_change_basicinfo(int guild_id,int type,const void *data,int len)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, len + 10);
	WFIFOW(inter_fd,0)=0x3039;
	WFIFOW(inter_fd,2)=len+10;
	WFIFOL(inter_fd,4)=guild_id;
	WFIFOW(inter_fd,8)=type;
	memcpy(WFIFOP(inter_fd,10),data,len);
	WFIFOSET(inter_fd,len+10);
	return 0;
}

// Request a change of Guild member information
int intif_guild_change_memberinfo(int guild_id,int account_id,int char_id,
	int type,const void *data,int len)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, len + 18);
	WFIFOW(inter_fd, 0)=0x303a;
	WFIFOW(inter_fd, 2)=len+18;
	WFIFOL(inter_fd, 4)=guild_id;
	WFIFOL(inter_fd, 8)=account_id;
	WFIFOL(inter_fd,12)=char_id;
	WFIFOW(inter_fd,16)=type;
	memcpy(WFIFOP(inter_fd,18),data,len);
	WFIFOSET(inter_fd,len+18);
	return 0;
}

// Request a change of Guild title
int intif_guild_position(int guild_id,int idx,struct guild_position *p)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, sizeof(struct guild_position)+12);
	WFIFOW(inter_fd,0)=0x303b;
	WFIFOW(inter_fd,2)=sizeof(struct guild_position)+12;
	WFIFOL(inter_fd,4)=guild_id;
	WFIFOL(inter_fd,8)=idx;
	memcpy(WFIFOP(inter_fd,12),p,sizeof(struct guild_position));
	WFIFOSET(inter_fd,WFIFOW(inter_fd,2));
	return 0;
}

// Request an update of Guildskill skill_id
int intif_guild_skillup(int guild_id, uint16 skill_id, int account_id, int max)
{
	if( intif->CheckForCharServer() )
		return 0;
	WFIFOHEAD(inter_fd, 18);
	WFIFOW(inter_fd, 0)  = 0x303c;
	WFIFOL(inter_fd, 2)  = guild_id;
	WFIFOL(inter_fd, 6)  = skill_id;
	WFIFOL(inter_fd, 10) = account_id;
	WFIFOL(inter_fd, 14) = max;
	WFIFOSET(inter_fd, 18);
	return 0;
}

// Request a new guild relationship
int intif_guild_alliance(int guild_id1,int guild_id2,int account_id1,int account_id2,int flag)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,19);
	WFIFOW(inter_fd, 0)=0x303d;
	WFIFOL(inter_fd, 2)=guild_id1;
	WFIFOL(inter_fd, 6)=guild_id2;
	WFIFOL(inter_fd,10)=account_id1;
	WFIFOL(inter_fd,14)=account_id2;
	WFIFOB(inter_fd,18)=flag;
	WFIFOSET(inter_fd,19);
	return 0;
}

// Request to change guild notice
int intif_guild_notice(int guild_id,const char *mes1,const char *mes2)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,186);
	WFIFOW(inter_fd,0)=0x303e;
	WFIFOL(inter_fd,2)=guild_id;
	memcpy(WFIFOP(inter_fd,6),mes1,MAX_GUILDMES1);
	memcpy(WFIFOP(inter_fd,66),mes2,MAX_GUILDMES2);
	WFIFOSET(inter_fd,186);
	return 0;
}

// Request to change guild emblem
int intif_guild_emblem(int guild_id,int len,const char *data)
{
	if (intif->CheckForCharServer())
		return 0;
	if(guild_id<=0 || len<0 || len>2000)
		return 0;
	WFIFOHEAD(inter_fd,len + 12);
	WFIFOW(inter_fd,0)=0x303f;
	WFIFOW(inter_fd,2)=len+12;
	WFIFOL(inter_fd,4)=guild_id;
	WFIFOL(inter_fd,8)=0;
	memcpy(WFIFOP(inter_fd,12),data,len);
	WFIFOSET(inter_fd,len+12);
	return 0;
}

/**
 * Requests guild castles data from char-server.
 * @param num Number of castles, size of castle_ids array.
 * @param castle_ids Pointer to array of castle IDs.
 */
int intif_guild_castle_dataload(int num, int *castle_ids)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, 4 + num * sizeof(int));
	WFIFOW(inter_fd, 0) = 0x3040;
	WFIFOW(inter_fd, 2) = 4 + num * sizeof(int);
	memcpy(WFIFOP(inter_fd, 4), castle_ids, num * sizeof(int));
	WFIFOSET(inter_fd, WFIFOW(inter_fd, 2));
	return 1;
}


// Request change castle guild owner and save data
int intif_guild_castle_datasave(int castle_id,int index, int value)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,9);
	WFIFOW(inter_fd,0)=0x3041;
	WFIFOW(inter_fd,2)=castle_id;
	WFIFOB(inter_fd,4)=index;
	WFIFOL(inter_fd,5)=value;
	WFIFOSET(inter_fd,9);
	return 1;
}

//-----------------------------------------------------------------
// Homunculus Packets send to Inter server [albator]
//-----------------------------------------------------------------

int intif_homunculus_create(int account_id, struct s_homunculus *sh)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, sizeof(struct s_homunculus)+8);
	WFIFOW(inter_fd,0) = 0x3090;
	WFIFOW(inter_fd,2) = sizeof(struct s_homunculus)+8;
	WFIFOL(inter_fd,4) = account_id;
	memcpy(WFIFOP(inter_fd,8),sh,sizeof(struct s_homunculus));
	WFIFOSET(inter_fd, WFIFOW(inter_fd,2));
	return 0;
}

bool intif_homunculus_requestload(int account_id, int homun_id) {
	if (intif->CheckForCharServer())
		return false;
	WFIFOHEAD(inter_fd, 10);
	WFIFOW(inter_fd,0) = 0x3091;
	WFIFOL(inter_fd,2) = account_id;
	WFIFOL(inter_fd,6) = homun_id;
	WFIFOSET(inter_fd, 10);
	return true;
}

int intif_homunculus_requestsave(int account_id, struct s_homunculus* sh)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, sizeof(struct s_homunculus)+8);
	WFIFOW(inter_fd,0) = 0x3092;
	WFIFOW(inter_fd,2) = sizeof(struct s_homunculus)+8;
	WFIFOL(inter_fd,4) = account_id;
	memcpy(WFIFOP(inter_fd,8),sh,sizeof(struct s_homunculus));
	WFIFOSET(inter_fd, WFIFOW(inter_fd,2));
	return 0;

}

int intif_homunculus_requestdelete(int homun_id)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, 6);
	WFIFOW(inter_fd, 0) = 0x3093;
	WFIFOL(inter_fd,2) = homun_id;
	WFIFOSET(inter_fd,6);
	return 0;

}


//-----------------------------------------------------------------
// Packets receive from inter server

// Wisp/Page reception // rewritten by [Yor]
void intif_parse_WisMessage(int fd) {
	struct map_session_data* sd;
	char *wisp_source;
	char name[NAME_LENGTH];
	int id, i;

	id=RFIFOL(fd,4);

	safestrncpy(name, (char*)RFIFOP(fd,32), NAME_LENGTH);
	sd = map->nick2sd(name);
	if(sd == NULL || strcmp(sd->status.name, name) != 0) {
		//Not found
		intif_wis_replay(id,1);
		return;
	}
	if(sd->state.ignoreAll) {
		intif_wis_replay(id, 2);
		return;
	}
	wisp_source = (char *) RFIFOP(fd,8); // speed up [Yor]
	for(i=0; i < MAX_IGNORE_LIST &&
		sd->ignore[i].name[0] != '\0' &&
		strcmp(sd->ignore[i].name, wisp_source) != 0
		; i++);

	if (i < MAX_IGNORE_LIST && sd->ignore[i].name[0] != '\0')
	{	//Ignored
		intif_wis_replay(id, 2);
		return;
	}
	//Success to send whisper.
	clif->wis_message(sd->fd, wisp_source, (char*)RFIFOP(fd,56),RFIFOW(fd,2)-56);
	intif_wis_replay(id,0);   // succes
}

// Wisp/page transmission result reception
void intif_parse_WisEnd(int fd) {
	struct map_session_data* sd;

	if (battle_config.etc_log)
		ShowInfo("intif_parse_wisend: player: %s, flag: %d\n", RFIFOP(fd,2), RFIFOB(fd,26)); // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
	sd = (struct map_session_data *)map->nick2sd((char *) RFIFOP(fd,2));
	if (sd != NULL)
		clif->wis_end(sd->fd, RFIFOB(fd,26));

	return;
}

int mapif_parse_WisToGM_sub(struct map_session_data* sd,va_list va) {
	int permission = va_arg(va, int);
	char *wisp_name;
	char *message;
	int len;

	if (!pc->has_permission(sd, permission))
		return 0;
	wisp_name = va_arg(va, char*);
	message = va_arg(va, char*);
	len = va_arg(va, int);
	clif->wis_message(sd->fd, wisp_name, message, len);
	return 1;
}

// Received wisp message from map-server via char-server for ALL gm
// 0x3003/0x3803 <packet_len>.w <wispname>.24B <permission>.l <message>.?B
void mapif_parse_WisToGM(int fd)
{
	int permission, mes_len;
	char Wisp_name[NAME_LENGTH];
	char mbuf[255];
	char *message;

	mes_len =  RFIFOW(fd,2) - 32;
	message = (char *) (mes_len >= 255 ? (char *) aMalloc(mes_len) : mbuf);

	permission = RFIFOL(fd,28);
	safestrncpy(Wisp_name, (char*)RFIFOP(fd,4), NAME_LENGTH);
	safestrncpy(message, (char*)RFIFOP(fd,32), mes_len);
	// information is sent to all online GM
	map->foreachpc(mapif_parse_WisToGM_sub, permission, Wisp_name, message, mes_len);

	if (message != mbuf)
		aFree(message);
}

// Request player registre
void intif_parse_Registers(int fd)
{
	int j,p,len,max, flag;
	struct map_session_data *sd;
	struct global_reg *reg;
	int *qty;
	int account_id = RFIFOL(fd,4), char_id = RFIFOL(fd,8);
	struct auth_node *node = chrif->auth_check(account_id, char_id, ST_LOGIN);
	if (node)
		sd = node->sd;
	else { //Normally registries should arrive for in log-in chars.
		sd = map->id2sd(account_id);
		if (sd && RFIFOB(fd,12) == 3 && sd->status.char_id != char_id)
			sd = NULL; //Character registry from another character.
	}
	if (!sd) return;

	flag = (sd->save_reg.global_num == -1 || sd->save_reg.account_num == -1 || sd->save_reg.account2_num == -1);

	switch (RFIFOB(fd,12)) {
		case 3: //Character Registry
			reg = sd->save_reg.global;
			qty = &sd->save_reg.global_num;
			max = GLOBAL_REG_NUM;
		break;
		case 2: //Account Registry
			reg = sd->save_reg.account;
			qty = &sd->save_reg.account_num;
			max = ACCOUNT_REG_NUM;
		break;
		case 1: //Account2 Registry
			reg = sd->save_reg.account2;
			qty = &sd->save_reg.account2_num;
			max = ACCOUNT_REG2_NUM;
		break;
		default:
			ShowError("intif_parse_Registers: Unrecognized type %d\n",RFIFOB(fd,12));
			return;
	}
	for(j=0,p=13;j<max && p<RFIFOW(fd,2);j++){
		sscanf((char*)RFIFOP(fd,p), "%31c%n", reg[j].str,&len);
		reg[j].str[len]='\0';
		p += len+1; //+1 to skip the '\0' between strings.
		sscanf((char*)RFIFOP(fd,p), "%255c%n", reg[j].value,&len);
		reg[j].value[len]='\0';
		p += len+1;
	}
	*qty = j;

	if (flag && sd->save_reg.global_num > -1 && sd->save_reg.account_num > -1 && sd->save_reg.account2_num > -1)
		pc->reg_received(sd); //Received all registry values, execute init scripts and what-not. [Skotlex]
}

void intif_parse_LoadGuildStorage(int fd)
{
	struct guild_storage *gstor;
	struct map_session_data *sd;
	int guild_id, flag;

	guild_id = RFIFOL(fd,8);
	flag = RFIFOL(fd,12);
	if(guild_id <= 0)
		return;
	sd=map->id2sd( RFIFOL(fd,4) );
	if( flag ){ //If flag != 0, we attach a player and open the storage
		if(sd==NULL){
			ShowError("intif_parse_LoadGuildStorage: user not found %d\n",RFIFOL(fd,4));
			return;
		}
	}
	gstor=gstorage->id2storage(guild_id);
	if(!gstor) {
		ShowWarning("intif_parse_LoadGuildStorage: error guild_id %d not exist\n",guild_id);
		return;
	}
	if (gstor->storage_status == 1) { // Already open.. lets ignore this update
		ShowWarning("intif_parse_LoadGuildStorage: storage received for a client already open (User %d:%d)\n", flag?sd->status.account_id:0, flag?sd->status.char_id:0);
		return;
	}
	if (gstor->dirty) { // Already have storage, and it has been modified and not saved yet! Exploit! [Skotlex]
		ShowWarning("intif_parse_LoadGuildStorage: received storage for an already modified non-saved storage! (User %d:%d)\n", flag?sd->status.account_id:0, flag?sd->status.char_id:0);
		return;
	}
	if( RFIFOW(fd,2)-13 != sizeof(struct guild_storage) ){
		ShowError("intif_parse_LoadGuildStorage: data size error %d %d\n",RFIFOW(fd,2)-13 , sizeof(struct guild_storage));
 		gstor->storage_status = 0;
		return;
	}

	memcpy(gstor,RFIFOP(fd,13),sizeof(struct guild_storage));
	if( flag )
		gstorage->open(sd);
}

// ACK guild_storage saved
void intif_parse_SaveGuildStorage(int fd)
{
	gstorage->saved(/*RFIFOL(fd,2), */RFIFOL(fd,6));
}

// ACK party creation
void intif_parse_PartyCreated(int fd)
{
	if(battle_config.etc_log)
		ShowInfo("intif: party created by account %d\n\n", RFIFOL(fd,2));
	party->created(RFIFOL(fd,2), RFIFOL(fd,6),RFIFOB(fd,10),RFIFOL(fd,11), (char *)RFIFOP(fd,15));
}

// Receive party info
void intif_parse_PartyInfo(int fd)
{
	if( RFIFOW(fd,2) == 12 ){
		ShowWarning("intif: party noinfo (char_id=%d party_id=%d)\n", RFIFOL(fd,4), RFIFOL(fd,8));
		party->recv_noinfo(RFIFOL(fd,8), RFIFOL(fd,4));
		return;
	}

	if( RFIFOW(fd,2) != 8+sizeof(struct party) )
		ShowError("intif: party info : data size error (char_id=%d party_id=%d packet_len=%d expected_len=%d)\n", RFIFOL(fd,4), RFIFOL(fd,8), RFIFOW(fd,2), 8+sizeof(struct party));
	party->recv_info((struct party *)RFIFOP(fd,8), RFIFOL(fd,4));
}

// ACK adding party member
void intif_parse_PartyMemberAdded(int fd)
{
	if(battle_config.etc_log)
		ShowInfo("intif: party member added Party (%d), Account(%d), Char(%d)\n",RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10));
	party->member_added(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10), RFIFOB(fd, 14));
}

// ACK changing party option
void intif_parse_PartyOptionChanged(int fd)
{
	party->optionchanged(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOW(fd,10),RFIFOW(fd,12),RFIFOB(fd,14));
}

// ACK member leaving party
void intif_parse_PartyMemberWithdraw(int fd)
{
	if(battle_config.etc_log)
		ShowInfo("intif: party member withdraw: Party(%d), Account(%d), Char(%d)\n",RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10));
	party->member_withdraw(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10));
}

// ACK party break
void intif_parse_PartyBroken(int fd) {
	party->broken(RFIFOL(fd,2));
}

// ACK party on new map
void intif_parse_PartyMove(int fd)
{
	party->recv_movemap(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOW(fd,14),RFIFOB(fd,16),RFIFOW(fd,17));
}

// ACK party messages
void intif_parse_PartyMessage(int fd) {
	party->recv_message(RFIFOL(fd,4),RFIFOL(fd,8),(char *) RFIFOP(fd,12),RFIFOW(fd,2)-12);
}

// ACK guild creation
void intif_parse_GuildCreated(int fd) {
	guild->created(RFIFOL(fd,2),RFIFOL(fd,6));
}

// ACK guild infos
void intif_parse_GuildInfo(int fd) {
	if(RFIFOW(fd,2) == 8) {
		ShowWarning("intif: guild noinfo %d\n",RFIFOL(fd,4));
		guild->recv_noinfo(RFIFOL(fd,4));
		return;
	}
	if( RFIFOW(fd,2)!=sizeof(struct guild)+4 )
		ShowError("intif: guild info : data size error Gid: %d recv size: %d Expected size: %d\n",RFIFOL(fd,4),RFIFOW(fd,2),sizeof(struct guild)+4);
	guild->recv_info((struct guild *)RFIFOP(fd,4));
}

// ACK adding guild member
void intif_parse_GuildMemberAdded(int fd) {
	if(battle_config.etc_log)
		ShowInfo("intif: guild member added %d %d %d %d\n",RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOB(fd,14));
	guild->member_added(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOB(fd,14));
}

// ACK member leaving guild
void intif_parse_GuildMemberWithdraw(int fd) {
	guild->member_withdraw(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOB(fd,14),(char *)RFIFOP(fd,55),(char *)RFIFOP(fd,15));
}

// ACK guild member basic info
void intif_parse_GuildMemberInfoShort(int fd) {
	guild->recv_memberinfoshort(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOB(fd,14),RFIFOW(fd,15),RFIFOW(fd,17));
}

// ACK guild break
void intif_parse_GuildBroken(int fd) {
	guild->broken(RFIFOL(fd,2),RFIFOB(fd,6));
}

// basic guild info change notice
// 0x3839 <packet len>.w <guild id>.l <type>.w <data>.?b
void intif_parse_GuildBasicInfoChanged(int fd) {
	//int len = RFIFOW(fd,2) - 10;
	int guild_id = RFIFOL(fd,4);
	int type = RFIFOW(fd,8);
	//void* data = RFIFOP(fd,10);

	struct guild* g = guild->search(guild_id);
	if( g == NULL )
		return;

	switch(type) {
		case GBI_EXP:        g->exp = RFIFOQ(fd,10); break;
		case GBI_GUILDLV:    g->guild_lv = RFIFOW(fd,10); break;
		case GBI_SKILLPOINT: g->skill_point = RFIFOL(fd,10); break;
	}
}

// guild member info change notice
// 0x383a <packet len>.w <guild id>.l <account id>.l <char id>.l <type>.w <data>.?b
void intif_parse_GuildMemberInfoChanged(int fd) {
	//int len = RFIFOW(fd,2) - 18;
	int guild_id = RFIFOL(fd,4);
	int account_id = RFIFOL(fd,8);
	int char_id = RFIFOL(fd,12);
	int type = RFIFOW(fd,16);
	//void* data = RFIFOP(fd,18);

	struct guild* g;
	int idx;

	g = guild->search(guild_id);
	if( g == NULL )
		return;

	idx = guild->getindex(g,account_id,char_id);
	if( idx == -1 )
		return;

	switch( type ) {
		case GMI_POSITION:   g->member[idx].position   = RFIFOW(fd,18); guild->memberposition_changed(g,idx,RFIFOW(fd,18)); break;
		case GMI_EXP:        g->member[idx].exp        = RFIFOQ(fd,18); break;
		case GMI_HAIR:       g->member[idx].hair       = RFIFOW(fd,18); break;
		case GMI_HAIR_COLOR: g->member[idx].hair_color = RFIFOW(fd,18); break;
		case GMI_GENDER:     g->member[idx].gender     = RFIFOW(fd,18); break;
		case GMI_CLASS:      g->member[idx].class_     = RFIFOW(fd,18); break;
		case GMI_LEVEL:      g->member[idx].lv         = RFIFOW(fd,18); break;
	}
}

// ACK change of guild title
void intif_parse_GuildPosition(int fd) {
	if( RFIFOW(fd,2)!=sizeof(struct guild_position)+12 )
		ShowError("intif: guild info : data size error\n %d %d %d",RFIFOL(fd,4),RFIFOW(fd,2),sizeof(struct guild_position)+12);
	guild->position_changed(RFIFOL(fd,4),RFIFOL(fd,8),(struct guild_position *)RFIFOP(fd,12));
}

// ACK change of guild skill update
void intif_parse_GuildSkillUp(int fd) {
	guild->skillupack(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10));
}

// ACK change of guild relationship
void intif_parse_GuildAlliance(int fd) {
	guild->allianceack(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOL(fd,14),RFIFOB(fd,18),(char *) RFIFOP(fd,19),(char *) RFIFOP(fd,43));
}

// ACK change of guild notice
void intif_parse_GuildNotice(int fd) {
	guild->notice_changed(RFIFOL(fd,2),(char *) RFIFOP(fd,6),(char *) RFIFOP(fd,66));
}

// ACK change of guild emblem
void intif_parse_GuildEmblem(int fd) {
	guild->emblem_changed(RFIFOW(fd,2)-12,RFIFOL(fd,4),RFIFOL(fd,8), (char *)RFIFOP(fd,12));
}

// ACK guild message
void intif_parse_GuildMessage(int fd) {
	guild->recv_message(RFIFOL(fd,4),RFIFOL(fd,8),(char *) RFIFOP(fd,12),RFIFOW(fd,2)-12);
}

// Reply guild castle data request
void intif_parse_GuildCastleDataLoad(int fd) {
	guild->castledataloadack(RFIFOW(fd,2), (struct guild_castle *)RFIFOP(fd,4));
}

// ACK change of guildmaster
void intif_parse_GuildMasterChanged(int fd) {
	guild->gm_changed(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10));
}

// Request pet creation
void intif_parse_CreatePet(int fd) {
	pet->get_egg(RFIFOL(fd,2),RFIFOL(fd,7),RFIFOB(fd,6));
}

// ACK pet data
void intif_parse_RecvPetData(int fd) {
	struct s_pet p;
	int len;
	len=RFIFOW(fd,2);
	if(sizeof(struct s_pet)!=len-9) {
		if(battle_config.etc_log)
			ShowError("intif: pet data: data size error %d %d\n",sizeof(struct s_pet),len-9);
	} else {
		memcpy(&p,RFIFOP(fd,9),sizeof(struct s_pet));
		pet->recv_petdata(RFIFOL(fd,4),&p,RFIFOB(fd,8));
	}
}
/* Really? Whats the point, shouldn't be sent when successful then [Ind] */
// ACK pet save data
void intif_parse_SavePetOk(int fd) {
	if(RFIFOB(fd,6) == 1)
		ShowError("pet data save failure\n");
}
/* Really? Whats the point, shouldn't be sent when successful then [Ind] */
// ACK deleting pet
void intif_parse_DeletePetOk(int fd) {
	if(RFIFOB(fd,2) == 1)
		ShowError("pet data delete failure\n");
}

// ACK changing name resquest, players,pets,hommon
void intif_parse_ChangeNameOk(int fd)
{
	struct map_session_data *sd = NULL;
	if((sd=map->id2sd(RFIFOL(fd,2)))==NULL ||
		sd->status.char_id != RFIFOL(fd,6))
		return;

	switch (RFIFOB(fd,10)) {
	case 0: //Players [NOT SUPPORTED YET]
		break;
	case 1: //Pets
		pet->change_name_ack(sd, (char*)RFIFOP(fd,12), RFIFOB(fd,11));
		break;
	case 2: //Hom
		homun->change_name_ack(sd, (char*)RFIFOP(fd,12), RFIFOB(fd,11));
		break;
	}
	return;
}

//----------------------------------------------------------------
// Homunculus recv packets [albator]

void intif_parse_CreateHomunculus(int fd) {
	int len = RFIFOW(fd,2)-9;
	if(sizeof(struct s_homunculus)!=len) {
		if(battle_config.etc_log)
			ShowError("intif: create homun data: data size error %d != %d\n",sizeof(struct s_homunculus),len);
		return;
	}
	homun->recv_data(RFIFOL(fd,4), (struct s_homunculus*)RFIFOP(fd,9), RFIFOB(fd,8)) ;
}

void intif_parse_RecvHomunculusData(int fd) {
	int len = RFIFOW(fd,2)-9;

	if(sizeof(struct s_homunculus)!=len) {
		if(battle_config.etc_log)
			ShowError("intif: homun data: data size error %d %d\n",sizeof(struct s_homunculus),len);
		return;
	}
	homun->recv_data(RFIFOL(fd,4), (struct s_homunculus*)RFIFOP(fd,9), RFIFOB(fd,8));
}

/* Really? Whats the point, shouldn't be sent when successful then [Ind] */
void intif_parse_SaveHomunculusOk(int fd) {
	if(RFIFOB(fd,6) != 1)
		ShowError("homunculus data save failure for account %d\n", RFIFOL(fd,2));
}

/* Really? Whats the point, shouldn't be sent when successful then [Ind] */
void intif_parse_DeleteHomunculusOk(int fd) {
	if(RFIFOB(fd,2) != 1)
		ShowError("Homunculus data delete failure\n");
}

/**************************************

QUESTLOG SYSTEM FUNCTIONS

***************************************/

/**
 * Requests a character's quest log entries to the inter server.
 *
 * @param sd Character's data
 */
void intif_request_questlog(TBL_PC *sd) {
	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x3060;
	WFIFOL(inter_fd,2) = sd->status.char_id;
	WFIFOSET(inter_fd,6);
}

/**
 * Parses the received quest log entries for a character from the inter server.
 *
 * Received in reply to the requests made by intif_request_questlog.
 *
 * @see intif_parse
 */
void intif_parse_QuestLog(int fd) {
	int char_id = RFIFOL(fd, 4), num_received = (RFIFOW(fd, 2)-8)/sizeof(struct quest);
	TBL_PC *sd = map->charid2sd(char_id);

	if (!sd) // User not online anymore
		return;

	if (num_received == 0) {
		if (sd->quest_log) {
			aFree(sd->quest_log);
			sd->quest_log = NULL;
		}
		sd->num_quests = sd->avail_quests = 0;
	} else {
		struct quest *received = (struct quest *)RFIFOP(fd, 8);
		int i, k = num_received;
		if (sd->quest_log) {
			RECREATE(sd->quest_log, struct quest, num_received);
		} else {
			CREATE(sd->quest_log, struct quest, num_received);
		}

		for (i = 0; i < num_received; i++) {
			if( quest->db(received[i].quest_id) == &quest->dummy ) {
				ShowError("intif_parse_QuestLog: quest %d not found in DB.\n", received[i].quest_id);
				continue;
			}
			if (received->state != Q_COMPLETE) {
				// Insert at the beginning
				memcpy(&sd->quest_log[sd->avail_quests++], &received[i], sizeof(struct quest));
			} else {
				// Insert at the end
				memcpy(&sd->quest_log[--k], &received[i], sizeof(struct quest));
			}
			sd->num_quests++;
		}
		if (sd->avail_quests < k) {
			// sd->avail_quests and k didn't meet in the middle: some entries were skipped
			if (k < num_received) // Move the entries at the end to fill the gap
				memmove(&sd->quest_log[k], &sd->quest_log[sd->avail_quests], sizeof(struct quest)*(num_received - k));
			sd->quest_log = aRealloc(sd->quest_log, sizeof(struct quest)*sd->num_quests);
		}
	}

	quest->pc_login(sd);
}

/**
 * Parses the quest log save ack for a character from the inter server.
 *
 * Received in reply to the requests made by intif_quest_save.
 *
 * @see intif_parse
 */
void intif_parse_QuestSave(int fd) {
	int cid = RFIFOL(fd, 2);
	TBL_PC *sd = map->id2sd(cid);

	if( !RFIFOB(fd, 6) )
		ShowError("intif_parse_QuestSave: Failed to save quest(s) for character %d!\n", cid);
	else if( sd )
		sd->save_quest = false;
}

/**
 * Requests to the inter server to save a character's quest log entries.
 *
 * @param sd Character's data
 * @return 0 in case of success, nonzero otherwise
 */
int intif_quest_save(TBL_PC *sd) {
	int len = sizeof(struct quest)*sd->num_quests + 8;

	if(intif->CheckForCharServer())
		return 1;

	WFIFOHEAD(inter_fd, len);
	WFIFOW(inter_fd,0) = 0x3061;
	WFIFOW(inter_fd,2) = len;
	WFIFOL(inter_fd,4) = sd->status.char_id;
	if( sd->num_quests )
		memcpy(WFIFOP(inter_fd,8), sd->quest_log, sizeof(struct quest)*sd->num_quests);
	WFIFOSET(inter_fd,  len);

	return 0;
}

/*==========================================
 * MAIL SYSTEM
 * By Zephyrus
 *==========================================*/

/*------------------------------------------
 * Inbox Request
 * flag: 0 Update Inbox | 1 OpenMail
 *------------------------------------------*/
int intif_Mail_requestinbox(int char_id, unsigned char flag)
{
	if (intif->CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,7);
	WFIFOW(inter_fd,0) = 0x3048;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOB(inter_fd,6) = flag;
	WFIFOSET(inter_fd,7);

	return 0;
}

void intif_parse_MailInboxReceived(int fd) {
	struct map_session_data *sd;
	unsigned char flag = RFIFOB(fd,8);

	sd = map->charid2sd(RFIFOL(fd,4));

	if (sd == NULL) {
		ShowError("intif_parse_MailInboxReceived: char not found %d\n",RFIFOL(fd,4));
		return;
	}

	if (RFIFOW(fd,2) - 9 != sizeof(struct mail_data)) {
		ShowError("intif_parse_MailInboxReceived: data size error %d %d\n", RFIFOW(fd,2) - 9, sizeof(struct mail_data));
		return;
	}

	//FIXME: this operation is not safe [ultramage]
	memcpy(&sd->mail.inbox, RFIFOP(fd,9), sizeof(struct mail_data));
	sd->mail.changed = false; // cache is now in sync

	if (flag)
		clif->mail_refreshinbox(sd);
	else if( battle_config.mail_show_status && ( battle_config.mail_show_status == 1 || sd->mail.inbox.unread ) ) {
		char output[128];
		sprintf(output, msg_txt(510), sd->mail.inbox.unchecked, sd->mail.inbox.unread + sd->mail.inbox.unchecked);
		clif->disp_onlyself(sd, output, strlen(output));
	}
}
/*------------------------------------------
 * Mail Read
 *------------------------------------------*/
int intif_Mail_read(int mail_id)
{
	if (intif->CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x3049;
	WFIFOL(inter_fd,2) = mail_id;
	WFIFOSET(inter_fd,6);

	return 0;
}
/*------------------------------------------
 * Get Attachment
 *------------------------------------------*/
int intif_Mail_getattach(int char_id, int mail_id)
{
	if (intif->CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x304a;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOL(inter_fd,6) = mail_id;
	WFIFOSET(inter_fd, 10);

	return 0;
}

void intif_parse_MailGetAttach(int fd) {
	struct map_session_data *sd;
	struct item item;
	int zeny = RFIFOL(fd,8);

	sd = map->charid2sd( RFIFOL(fd,4) );

	if (sd == NULL) {
		ShowError("intif_parse_MailGetAttach: char not found %d\n",RFIFOL(fd,4));
		return;
	}

	if (RFIFOW(fd,2) - 12 != sizeof(struct item)) {
		ShowError("intif_parse_MailGetAttach: data size error %d %d\n", RFIFOW(fd,2) - 16, sizeof(struct item));
		return;
	}

	memcpy(&item, RFIFOP(fd,12), sizeof(struct item));

	mail->getattachment(sd, zeny, &item);
}
/*------------------------------------------
 * Delete Message
 *------------------------------------------*/
int intif_Mail_delete(int char_id, int mail_id)
{
	if (intif->CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x304b;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOL(inter_fd,6) = mail_id;
	WFIFOSET(inter_fd,10);

	return 0;
}

void intif_parse_MailDelete(int fd) {
	struct map_session_data *sd;
	int char_id = RFIFOL(fd,2);
	int mail_id = RFIFOL(fd,6);
	bool failed = RFIFOB(fd,10);
	
	if ( (sd = map->charid2sd(char_id)) == NULL) {
		ShowError("intif_parse_MailDelete: char not found %d\n", char_id);
		return;
	}

	if (!failed) {
		int i;
		ARR_FIND(0, MAIL_MAX_INBOX, i, sd->mail.inbox.msg[i].id == mail_id);
		if( i < MAIL_MAX_INBOX ) {
			memset(&sd->mail.inbox.msg[i], 0, sizeof(struct mail_message));
			sd->mail.inbox.amount--;
		}

		if( sd->mail.inbox.full )
			intif_Mail_requestinbox(sd->status.char_id, 1); // Free space is available for new mails
	}

	clif->mail_delete(sd->fd, mail_id, failed);
}
/*------------------------------------------
 * Return Message
 *------------------------------------------*/
int intif_Mail_return(int char_id, int mail_id)
{
	if (intif->CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x304c;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOL(inter_fd,6) = mail_id;
	WFIFOSET(inter_fd,10);

	return 0;
}

void intif_parse_MailReturn(int fd) {
	struct map_session_data *sd = map->charid2sd(RFIFOL(fd,2));
	int mail_id = RFIFOL(fd,6);
	short fail = RFIFOB(fd,10);

	if( sd == NULL ) {
		ShowError("intif_parse_MailReturn: char not found %d\n",RFIFOL(fd,2));
		return;
	}

	if( !fail ) {
		int i;
		ARR_FIND(0, MAIL_MAX_INBOX, i, sd->mail.inbox.msg[i].id == mail_id);
		if( i < MAIL_MAX_INBOX ) {
			memset(&sd->mail.inbox.msg[i], 0, sizeof(struct mail_message));
			sd->mail.inbox.amount--;
		}

		if( sd->mail.inbox.full )
			intif_Mail_requestinbox(sd->status.char_id, 1); // Free space is available for new mails
	}

	clif->mail_return(sd->fd, mail_id, fail);
}
/*------------------------------------------
 * Send Mail
 *------------------------------------------*/
int intif_Mail_send(int account_id, struct mail_message *msg)
{
	int len = sizeof(struct mail_message) + 8;

	if (intif->CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,len);
	WFIFOW(inter_fd,0) = 0x304d;
	WFIFOW(inter_fd,2) = len;
	WFIFOL(inter_fd,4) = account_id;
	memcpy(WFIFOP(inter_fd,8), msg, sizeof(struct mail_message));
	WFIFOSET(inter_fd,len);

	return 1;
}

void intif_parse_MailSend(int fd) {
	struct mail_message msg;
	struct map_session_data *sd;
	bool fail;

	if( RFIFOW(fd,2) - 4 != sizeof(struct mail_message) ) {
		ShowError("intif_parse_MailSend: data size error %d %d\n", RFIFOW(fd,2) - 4, sizeof(struct mail_message));
		return;
	}

	memcpy(&msg, RFIFOP(fd,4), sizeof(struct mail_message));
	fail = (msg.id == 0);

	// notify sender
	sd = map->charid2sd(msg.send_id);
	if( sd != NULL ) {
		if( fail )
			mail->deliveryfail(sd, &msg);
		else {
			clif->mail_send(sd->fd, false);
			if( map->save_settings&16 )
				chrif->save(sd, 0);
		}
	}
}

void intif_parse_MailNew(int fd) {
	struct map_session_data *sd = map->charid2sd(RFIFOL(fd,2));
	int mail_id = RFIFOL(fd,6);
	const char* sender_name = (char*)RFIFOP(fd,10);
	const char* title = (char*)RFIFOP(fd,34);

	if( sd == NULL )
		return;

	sd->mail.changed = true;
	clif->mail_new(sd->fd, mail_id, sender_name, title);
}

/*==========================================
 * AUCTION SYSTEM
 * By Zephyrus
 *==========================================*/
int intif_Auction_requestlist(int char_id, short type, int price, const char* searchtext, short page)
{
	int len = NAME_LENGTH + 16;

	if( intif->CheckForCharServer() )
		return 0;

	WFIFOHEAD(inter_fd,len);
	WFIFOW(inter_fd,0) = 0x3050;
	WFIFOW(inter_fd,2) = len;
	WFIFOL(inter_fd,4) = char_id;
	WFIFOW(inter_fd,8) = type;
	WFIFOL(inter_fd,10) = price;
	WFIFOW(inter_fd,14) = page;
	memcpy(WFIFOP(inter_fd,16), searchtext, NAME_LENGTH);
	WFIFOSET(inter_fd,len);

	return 0;
}

void intif_parse_AuctionResults(int fd) {
	struct map_session_data *sd = map->charid2sd(RFIFOL(fd,4));
	short count = RFIFOW(fd,8);
	short pages = RFIFOW(fd,10);
	uint8* data = RFIFOP(fd,12);

	if( sd == NULL )
		return;

	clif->auction_results(sd, count, pages, data);
}

int intif_Auction_register(struct auction_data *auction)
{
	int len = sizeof(struct auction_data) + 4;

	if( intif->CheckForCharServer() )
		return 0;

	WFIFOHEAD(inter_fd,len);
	WFIFOW(inter_fd,0) = 0x3051;
	WFIFOW(inter_fd,2) = len;
	memcpy(WFIFOP(inter_fd,4), auction, sizeof(struct auction_data));
	WFIFOSET(inter_fd,len);

	return 1;
}

void intif_parse_AuctionRegister(int fd) {
	struct map_session_data *sd;
	struct auction_data auction;

	if( RFIFOW(fd,2) - 4 != sizeof(struct auction_data) ) {
		ShowError("intif_parse_AuctionRegister: data size error %d %d\n", RFIFOW(fd,2) - 4, sizeof(struct auction_data));
		return;
	}

	memcpy(&auction, RFIFOP(fd,4), sizeof(struct auction_data));
	if( (sd = map->charid2sd(auction.seller_id)) == NULL )
		return;

	if( auction.auction_id > 0 ) {
		clif->auction_message(sd->fd, 1); // Confirmation Packet ??
		if( map->save_settings&32 )
			chrif->save(sd,0);
	} else {
		int zeny = auction.hours*battle_config.auction_feeperhour;

		clif->auction_message(sd->fd, 4);
		pc->additem(sd, &auction.item, auction.item.amount, LOG_TYPE_AUCTION);

		pc->getzeny(sd, zeny, LOG_TYPE_AUCTION, NULL);
	}
}

int intif_Auction_cancel(int char_id, unsigned int auction_id)
{
	if( intif->CheckForCharServer() )
		return 0;

	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x3052;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOL(inter_fd,6) = auction_id;
	WFIFOSET(inter_fd,10);

	return 0;
}

void intif_parse_AuctionCancel(int fd) {
	struct map_session_data *sd = map->charid2sd(RFIFOL(fd,2));
	int result = RFIFOB(fd,6);

	if( sd == NULL )
		return;

	switch( result ) {
		case 0: clif->auction_message(sd->fd, 2); break;
		case 1: clif->auction_close(sd->fd, 2); break;
		case 2: clif->auction_close(sd->fd, 1); break;
		case 3: clif->auction_message(sd->fd, 3); break;
	}
}

int intif_Auction_close(int char_id, unsigned int auction_id)
{
	if( intif->CheckForCharServer() )
		return 0;

	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x3053;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOL(inter_fd,6) = auction_id;
	WFIFOSET(inter_fd,10);

	return 0;
}

void intif_parse_AuctionClose(int fd) {
	struct map_session_data *sd = map->charid2sd(RFIFOL(fd,2));
	unsigned char result = RFIFOB(fd,6);

	if( sd == NULL )
		return;

	clif->auction_close(sd->fd, result);
	if( result == 0 ) {
		// FIXME: Leeching off a parse function
		clif->pAuction_cancelreg(fd, sd);
		intif_Auction_requestlist(sd->status.char_id, 6, 0, "", 1);
	}
}

int intif_Auction_bid(int char_id, const char* name, unsigned int auction_id, int bid)
{
	int len = 16 + NAME_LENGTH;

	if( intif->CheckForCharServer() )
		return 0;

	WFIFOHEAD(inter_fd,len);
	WFIFOW(inter_fd,0) = 0x3055;
	WFIFOW(inter_fd,2) = len;
	WFIFOL(inter_fd,4) = char_id;
	WFIFOL(inter_fd,8) = auction_id;
	WFIFOL(inter_fd,12) = bid;
	memcpy(WFIFOP(inter_fd,16), name, NAME_LENGTH);
	WFIFOSET(inter_fd,len);

	return 0;
}

void intif_parse_AuctionBid(int fd) {
	struct map_session_data *sd = map->charid2sd(RFIFOL(fd,2));
	int bid = RFIFOL(fd,6);
	unsigned char result = RFIFOB(fd,10);

	if( sd == NULL )
		return;

	clif->auction_message(sd->fd, result);
	if( bid > 0 ) {
		pc->getzeny(sd, bid, LOG_TYPE_AUCTION,NULL);
	}
	if( result == 1 ) { // To update the list, display your buy list
		clif->pAuction_cancelreg(fd, sd);
		intif_Auction_requestlist(sd->status.char_id, 7, 0, "", 1);
	}
}

// Used to send 'You have won the auction' and 'You failed to won the auction' messages
void intif_parse_AuctionMessage(int fd) {
	struct map_session_data *sd = map->charid2sd(RFIFOL(fd,2));
	unsigned char result = RFIFOB(fd,6);

	if( sd == NULL )
		return;

	clif->auction_message(sd->fd, result);
}

/*==========================================
 * Mercenary's System
 *------------------------------------------*/
int intif_mercenary_create(struct s_mercenary *merc)
{
	int size = sizeof(struct s_mercenary) + 4;

	if( intif->CheckForCharServer() )
		return 0;

	WFIFOHEAD(inter_fd,size);
	WFIFOW(inter_fd,0) = 0x3070;
	WFIFOW(inter_fd,2) = size;
	memcpy(WFIFOP(inter_fd,4), merc, sizeof(struct s_mercenary));
	WFIFOSET(inter_fd,size);
	return 0;
}

void intif_parse_MercenaryReceived(int fd) {
	int len = RFIFOW(fd,2) - 5;
	
	if( sizeof(struct s_mercenary) != len ) {
		if( battle_config.etc_log )
			ShowError("intif: create mercenary data size error %d != %d\n", sizeof(struct s_mercenary), len);
		return;
	}

	mercenary->data_received((struct s_mercenary*)RFIFOP(fd,5), RFIFOB(fd,4));
}

int intif_mercenary_request(int merc_id, int char_id)
{
	if (intif->CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x3071;
	WFIFOL(inter_fd,2) = merc_id;
	WFIFOL(inter_fd,6) = char_id;
	WFIFOSET(inter_fd,10);
	return 0;
}

int intif_mercenary_delete(int merc_id)
{
	if (intif->CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x3072;
	WFIFOL(inter_fd,2) = merc_id;
	WFIFOSET(inter_fd,6);
	return 0;
}
/* Really? Whats the point, shouldn't be sent when successful then [Ind] */
void intif_parse_MercenaryDeleted(int fd) {
	if( RFIFOB(fd,2) != 1 )
		ShowError("Mercenary data delete failure\n");
}

int intif_mercenary_save(struct s_mercenary *merc)
{
	int size = sizeof(struct s_mercenary) + 4;

	if( intif->CheckForCharServer() )
		return 0;

	WFIFOHEAD(inter_fd,size);
	WFIFOW(inter_fd,0) = 0x3073;
	WFIFOW(inter_fd,2) = size;
	memcpy(WFIFOP(inter_fd,4), merc, sizeof(struct s_mercenary));
	WFIFOSET(inter_fd,size);
	return 0;
}
/* Really? Whats the point, shouldn't be sent when successful then [Ind] */
void intif_parse_MercenarySaved(int fd) {
	if( RFIFOB(fd,2) != 1 )
		ShowError("Mercenary data save failure\n");
}

/*==========================================
 * Elemental's System
 *------------------------------------------*/
int intif_elemental_create(struct s_elemental *ele)
{
	int size = sizeof(struct s_elemental) + 4;

	if( intif->CheckForCharServer() )
		return 0;

	WFIFOHEAD(inter_fd,size);
	WFIFOW(inter_fd,0) = 0x307c;
	WFIFOW(inter_fd,2) = size;
	memcpy(WFIFOP(inter_fd,4), ele, sizeof(struct s_elemental));
	WFIFOSET(inter_fd,size);
	return 0;
}

void intif_parse_ElementalReceived(int fd) {
	int len = RFIFOW(fd,2) - 5;
	
	if( sizeof(struct s_elemental) != len ) {
		if( battle_config.etc_log )
			ShowError("intif: create elemental data size error %d != %d\n", sizeof(struct s_elemental), len);
		return;
	}

	elemental->data_received((struct s_elemental*)RFIFOP(fd,5), RFIFOB(fd,4));
}

int intif_elemental_request(int ele_id, int char_id)
{
	if (intif->CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x307d;
	WFIFOL(inter_fd,2) = ele_id;
	WFIFOL(inter_fd,6) = char_id;
	WFIFOSET(inter_fd,10);
	return 0;
}

int intif_elemental_delete(int ele_id)
{
	if (intif->CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x307e;
	WFIFOL(inter_fd,2) = ele_id;
	WFIFOSET(inter_fd,6);
	return 0;
}
/* Really? Whats the point, shouldn't be sent when successful then [Ind] */
void intif_parse_ElementalDeleted(int fd) {
	if( RFIFOB(fd,2) != 1 )
		ShowError("Elemental data delete failure\n");
}

int intif_elemental_save(struct s_elemental *ele)
{
	int size = sizeof(struct s_elemental) + 4;

	if( intif->CheckForCharServer() )
		return 0;

	WFIFOHEAD(inter_fd,size);
	WFIFOW(inter_fd,0) = 0x307f;
	WFIFOW(inter_fd,2) = size;
	memcpy(WFIFOP(inter_fd,4), ele, sizeof(struct s_elemental));
	WFIFOSET(inter_fd,size);
	return 0;
}
/* Really? Whats the point, shouldn't be sent when successful then [Ind] */
void intif_parse_ElementalSaved(int fd) {
	if( RFIFOB(fd,2) != 1 )
		ShowError("Elemental data save failure\n");
}

void intif_request_accinfo( int u_fd, int aid, int group_lv, char* query ) {


	WFIFOHEAD(inter_fd,2 + 4 + 4 + 4 + NAME_LENGTH);

	WFIFOW(inter_fd,0) = 0x3007;
	WFIFOL(inter_fd,2) = u_fd;
	WFIFOL(inter_fd,6) = aid;
	WFIFOL(inter_fd,10) = group_lv;
    safestrncpy((char *)WFIFOP(inter_fd,14), query, NAME_LENGTH);

	WFIFOSET(inter_fd,2 + 4 + 4 + 4 + NAME_LENGTH);

	return;
}

void intif_parse_MessageToFD(int fd) {
	int u_fd = RFIFOL(fd,4);

	if( session[u_fd] && session[u_fd]->session_data ) {
		int aid = RFIFOL(fd,8);
		struct map_session_data * sd = session[u_fd]->session_data;
		/* matching e.g. previous fd owner didn't dc during request or is still the same */
		if( sd->bl.id == aid ) {
			char msg[512];
			safestrncpy(msg, (char*)RFIFOP(fd,12), RFIFOW(fd,2) - 12);
			clif->message(u_fd,msg);
		}

	}

	return;
}
/*==========================================
 * Item Bound System [Xantara][Mhalicot]
 *------------------------------------------*/
void intif_itembound_req(int char_id,int aid,int guild_id) {
#ifdef GP_BOUND_ITEMS
	struct guild_storage *gstor = gstorage->id2storage2(guild_id);
	WFIFOHEAD(inter_fd,12);
	WFIFOW(inter_fd,0) = 0x3056;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOL(inter_fd,6) = aid;
	WFIFOW(inter_fd,10) = guild_id;
	WFIFOSET(inter_fd,12);
	if(gstor)
		gstor->lock = 1; //Lock for retrieval process
#endif
}
 
//3856
void intif_parse_Itembound_ack(int fd) {
#ifdef GP_BOUND_ITEMS
	struct guild_storage *gstor;
	int guild_id = RFIFOW(fd,6);

	gstor = gstorage->id2storage2(guild_id);
	if(gstor)
		gstor->lock = 0; //Unlock now that operation is completed
#endif
}
//-----------------------------------------------------------------
// Communication from the inter server
// Return a 0 (false) if there were any errors.
// 1, 2 if there are not enough to return the length of the packet if the packet processing
int intif_parse(int fd)
{
	int packet_len, cmd;
	cmd = RFIFOW(fd,0);
    // Verify ID of the packet
	if(cmd<0x3800 || cmd>=0x3800+(sizeof(intif->packet_len_table)/sizeof(intif->packet_len_table[0])) ||
	   intif->packet_len_table[cmd-0x3800]==0){
	   	return 0;
	}
    // Check the length of the packet
	packet_len = intif->packet_len_table[cmd-0x3800];
	if(packet_len==-1){
		if(RFIFOREST(fd)<4)
			return 2;
		packet_len = RFIFOW(fd,2);
	}
	if((int)RFIFOREST(fd)<packet_len){
		return 2;
	}
    // Processing branch
	switch(cmd){
		case 0x3800:
			if (RFIFOL(fd,4) == 0xFF000000) //Normal announce.
				clif->broadcast(NULL, (char *) RFIFOP(fd,16), packet_len-16, BC_DEFAULT, ALL_CLIENT);
			else //Color announce.
				clif->broadcast2(NULL, (char *) RFIFOP(fd,16), packet_len-16, RFIFOL(fd,4), RFIFOW(fd,8), RFIFOW(fd,10), RFIFOW(fd,12), RFIFOW(fd,14), ALL_CLIENT);
			break;
		case 0x3801:	intif->pWisMessage(fd); break;
		case 0x3802:	intif->pWisEnd(fd); break;
		case 0x3803:	intif->pWisToGM(fd); break;
		case 0x3804:	intif->pRegisters(fd); break;
		case 0x3806:	intif->pChangeNameOk(fd); break;
		case 0x3807:	intif->pMessageToFD(fd); break;
		case 0x3818:	intif->pLoadGuildStorage(fd); break;
		case 0x3819:	intif->pSaveGuildStorage(fd); break;
		case 0x3820:	intif->pPartyCreated(fd); break;
		case 0x3821:	intif->pPartyInfo(fd); break;
		case 0x3822:	intif->pPartyMemberAdded(fd); break;
		case 0x3823:	intif->pPartyOptionChanged(fd); break;
		case 0x3824:	intif->pPartyMemberWithdraw(fd); break;
		case 0x3825:	intif->pPartyMove(fd); break;
		case 0x3826:	intif->pPartyBroken(fd); break;
		case 0x3827:	intif->pPartyMessage(fd); break;
		case 0x3830:	intif->pGuildCreated(fd); break;
		case 0x3831:	intif->pGuildInfo(fd); break;
		case 0x3832:	intif->pGuildMemberAdded(fd); break;
		case 0x3834:	intif->pGuildMemberWithdraw(fd); break;
		case 0x3835:	intif->pGuildMemberInfoShort(fd); break;
		case 0x3836:	intif->pGuildBroken(fd); break;
		case 0x3837:	intif->pGuildMessage(fd); break;
		case 0x3839:	intif->pGuildBasicInfoChanged(fd); break;
		case 0x383a:	intif->pGuildMemberInfoChanged(fd); break;
		case 0x383b:	intif->pGuildPosition(fd); break;
		case 0x383c:	intif->pGuildSkillUp(fd); break;
		case 0x383d:	intif->pGuildAlliance(fd); break;
		case 0x383e:	intif->pGuildNotice(fd); break;
		case 0x383f:	intif->pGuildEmblem(fd); break;
		case 0x3840:	intif->pGuildCastleDataLoad(fd); break;
		case 0x3843:	intif->pGuildMasterChanged(fd); break;
			
		//Quest system
		case 0x3860:	intif->pQuestLog(fd); break;
		case 0x3861:	intif->pQuestSave(fd); break;
			
		// Mail System
		case 0x3848:	intif->pMailInboxReceived(fd); break;
		case 0x3849:	intif->pMailNew(fd); break;
		case 0x384a:	intif->pMailGetAttach(fd); break;
		case 0x384b:	intif->pMailDelete(fd); break;
		case 0x384c:	intif->pMailReturn(fd); break;
		case 0x384d:	intif->pMailSend(fd); break;
		// Auction System
		case 0x3850:	intif->pAuctionResults(fd); break;
		case 0x3851:	intif->pAuctionRegister(fd); break;
		case 0x3852:	intif->pAuctionCancel(fd); break;
		case 0x3853:	intif->pAuctionClose(fd); break;
		case 0x3854:	intif->pAuctionMessage(fd); break;
		case 0x3855:	intif->pAuctionBid(fd); break;
		//Bound items
		case 0x3856:
#ifdef GP_BOUND_ITEMS
			intif->pItembound_ack(fd);
#else
			ShowWarning("intif_parse: Received 0x3856 with GP_BOUND_ITEMS disabled !!!\n")
#endif
			break;
		// Mercenary System
		case 0x3870:	intif->pMercenaryReceived(fd); break;
		case 0x3871:	intif->pMercenaryDeleted(fd); break;
		case 0x3872:	intif->pMercenarySaved(fd); break;
		// Elemental System
		case 0x387c:	intif->pElementalReceived(fd); break;
		case 0x387d:	intif->pElementalDeleted(fd); break;
		case 0x387e:	intif->pElementalSaved(fd); break;
			
		case 0x3880:	intif->pCreatePet(fd); break;
		case 0x3881:	intif->pRecvPetData(fd); break;
		case 0x3882:	intif->pSavePetOk(fd); break;
		case 0x3883:	intif->pDeletePetOk(fd); break;
		case 0x3890:	intif->pCreateHomunculus(fd); break;
		case 0x3891:	intif->pRecvHomunculusData(fd); break;
		case 0x3892:	intif->pSaveHomunculusOk(fd); break;
		case 0x3893:	intif->pDeleteHomunculusOk(fd); break;
	default:
		ShowError("intif_parse : unknown packet %d %x\n",fd,RFIFOW(fd,0));
		return 0;
	}
    // Skip packet
	RFIFOSKIP(fd,packet_len);
	return 1;
}

/*=====================================
* Default Functions : intif.h 
* Generated by HerculesInterfaceMaker
* created by Susu
*-------------------------------------*/
void intif_defaults(void) {
	const int packet_len_table [INTIF_PACKET_LEN_TABLE_SIZE] = {
		-1,-1,27,-1, -1, 0,37,-1,  0, 0, 0, 0,  0, 0,  0, 0, //0x3800-0x380f
		 0, 0, 0, 0,  0, 0, 0, 0, -1,11, 0, 0,  0, 0,  0, 0, //0x3810
		39,-1,15,15, 14,19, 7,-1,  0, 0, 0, 0,  0, 0,  0, 0, //0x3820
		10,-1,15, 0, 79,19, 7,-1,  0,-1,-1,-1, 14,67,186,-1, //0x3830
		-1, 0, 0,14,  0, 0, 0, 0, -1,74,-1,11, 11,-1,  0, 0, //0x3840
		-1,-1, 7, 7,  7,11, 8, 0,  0, 0, 0, 0,  0, 0,  0, 0, //0x3850  Auctions [Zephyrus] itembound[Akinari]
		-1, 7, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, //0x3860  Quests [Kevin] [Inkfish]
		-1, 3, 3, 0,  0, 0, 0, 0,  0, 0, 0, 0, -1, 3,  3, 0, //0x3870  Mercenaries [Zephyrus] / Elemental [pakpil]
		11,-1, 7, 3,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, //0x3880
		-1,-1, 7, 3,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, //0x3890  Homunculus [albator]
	};

	intif = &intif_s;

	/* */
	memcpy(intif->packet_len_table,&packet_len_table,sizeof(intif->packet_len_table));
	
	/* funcs */
	intif->parse = intif_parse;
	intif->create_pet = intif_create_pet;
	intif->broadcast = intif_broadcast;
	intif->broadcast2 = intif_broadcast2;
	intif->main_message = intif_main_message;
	intif->wis_message = intif_wis_message;
	intif->wis_message_to_gm = intif_wis_message_to_gm;
	intif->saveregistry = intif_saveregistry;
	intif->request_registry = intif_request_registry;
	intif->request_guild_storage = intif_request_guild_storage;
	intif->send_guild_storage = intif_send_guild_storage;
	intif->create_party = intif_create_party;
	intif->request_partyinfo = intif_request_partyinfo;
	intif->party_addmember = intif_party_addmember;
	intif->party_changeoption = intif_party_changeoption;
	intif->party_leave = intif_party_leave;
	intif->party_changemap = intif_party_changemap;
	intif->break_party = intif_break_party;
	intif->party_message = intif_party_message;
	intif->party_leaderchange = intif_party_leaderchange;
	intif->guild_create = intif_guild_create;
	intif->guild_request_info = intif_guild_request_info;
	intif->guild_addmember = intif_guild_addmember;
	intif->guild_leave = intif_guild_leave;
	intif->guild_memberinfoshort = intif_guild_memberinfoshort;
	intif->guild_break = intif_guild_break;
	intif->guild_message = intif_guild_message;
	intif->guild_change_gm = intif_guild_change_gm;
	intif->guild_change_basicinfo = intif_guild_change_basicinfo;
	intif->guild_change_memberinfo = intif_guild_change_memberinfo;
	intif->guild_position = intif_guild_position;
	intif->guild_skillup = intif_guild_skillup;
	intif->guild_alliance = intif_guild_alliance;
	intif->guild_notice = intif_guild_notice;
	intif->guild_emblem = intif_guild_emblem;
	intif->guild_castle_dataload = intif_guild_castle_dataload;
	intif->guild_castle_datasave = intif_guild_castle_datasave;
	intif->request_petdata = intif_request_petdata;
	intif->save_petdata = intif_save_petdata;
	intif->delete_petdata = intif_delete_petdata;
	intif->rename = intif_rename;
	intif->homunculus_create = intif_homunculus_create;
	intif->homunculus_requestload = intif_homunculus_requestload;
	intif->homunculus_requestsave = intif_homunculus_requestsave;
	intif->homunculus_requestdelete = intif_homunculus_requestdelete;
	/******QUEST SYTEM*******/
	intif->request_questlog = intif_request_questlog;
	intif->quest_save = intif_quest_save;
	// MERCENARY SYSTEM
	intif->mercenary_create = intif_mercenary_create;
	intif->mercenary_request = intif_mercenary_request;
	intif->mercenary_delete = intif_mercenary_delete;
	intif->mercenary_save = intif_mercenary_save;
	// MAIL SYSTEM
	intif->Mail_requestinbox = intif_Mail_requestinbox;
	intif->Mail_read = intif_Mail_read;
	intif->Mail_getattach = intif_Mail_getattach;
	intif->Mail_delete = intif_Mail_delete;
	intif->Mail_return = intif_Mail_return;
	intif->Mail_send = intif_Mail_send;
	// AUCTION SYSTEM
	intif->Auction_requestlist = intif_Auction_requestlist;
	intif->Auction_register = intif_Auction_register;
	intif->Auction_cancel = intif_Auction_cancel;
	intif->Auction_close = intif_Auction_close;
	intif->Auction_bid = intif_Auction_bid;
	// ELEMENTAL SYSTEM
	intif->elemental_create = intif_elemental_create;
	intif->elemental_request = intif_elemental_request;
	intif->elemental_delete = intif_elemental_delete;
	intif->elemental_save = intif_elemental_save;
	/* @accinfo */
	intif->request_accinfo = intif_request_accinfo;
	/* */
	intif->CheckForCharServer = CheckForCharServer;
	/* */
	intif->itembound_req = intif_itembound_req;
	/* parse functions */
	intif->pWisMessage = intif_parse_WisMessage;
	intif->pWisEnd = intif_parse_WisEnd;
	intif->pWisToGM_sub = mapif_parse_WisToGM_sub;
	intif->pWisToGM = mapif_parse_WisToGM;
	intif->pRegisters = intif_parse_Registers;
	intif->pChangeNameOk = intif_parse_ChangeNameOk;
	intif->pMessageToFD = intif_parse_MessageToFD;
	intif->pLoadGuildStorage = intif_parse_LoadGuildStorage;
	intif->pSaveGuildStorage = intif_parse_SaveGuildStorage;
	intif->pPartyCreated = intif_parse_PartyCreated;
	intif->pPartyInfo = intif_parse_PartyInfo;
	intif->pPartyMemberAdded = intif_parse_PartyMemberAdded;
	intif->pPartyOptionChanged = intif_parse_PartyOptionChanged;
	intif->pPartyMemberWithdraw = intif_parse_PartyMemberWithdraw;
	intif->pPartyMove = intif_parse_PartyMove;
	intif->pPartyBroken = intif_parse_PartyBroken;
	intif->pPartyMessage = intif_parse_PartyMessage;
	intif->pGuildCreated = intif_parse_GuildCreated;
	intif->pGuildInfo = intif_parse_GuildInfo;
	intif->pGuildMemberAdded = intif_parse_GuildMemberAdded;
	intif->pGuildMemberWithdraw = intif_parse_GuildMemberWithdraw;
	intif->pGuildMemberInfoShort = intif_parse_GuildMemberInfoShort;
	intif->pGuildBroken = intif_parse_GuildBroken;
	intif->pGuildMessage = intif_parse_GuildMessage;
	intif->pGuildBasicInfoChanged = intif_parse_GuildBasicInfoChanged;
	intif->pGuildMemberInfoChanged = intif_parse_GuildMemberInfoChanged;
	intif->pGuildPosition = intif_parse_GuildPosition;
	intif->pGuildSkillUp = intif_parse_GuildSkillUp;
	intif->pGuildAlliance = intif_parse_GuildAlliance;
	intif->pGuildNotice = intif_parse_GuildNotice;
	intif->pGuildEmblem = intif_parse_GuildEmblem;
	intif->pGuildCastleDataLoad = intif_parse_GuildCastleDataLoad;
	intif->pGuildMasterChanged = intif_parse_GuildMasterChanged;
	intif->pQuestLog = intif_parse_QuestLog;
	intif->pQuestSave = intif_parse_QuestSave;
	intif->pMailInboxReceived = intif_parse_MailInboxReceived;
	intif->pMailNew = intif_parse_MailNew;
	intif->pMailGetAttach = intif_parse_MailGetAttach;
	intif->pMailDelete = intif_parse_MailDelete;
	intif->pMailReturn = intif_parse_MailReturn;
	intif->pMailSend = intif_parse_MailSend;
	intif->pAuctionResults = intif_parse_AuctionResults;
	intif->pAuctionRegister = intif_parse_AuctionRegister;
	intif->pAuctionCancel = intif_parse_AuctionCancel;
	intif->pAuctionClose = intif_parse_AuctionClose;
	intif->pAuctionMessage = intif_parse_AuctionMessage;
	intif->pAuctionBid = intif_parse_AuctionBid;
	intif->pItembound_ack = intif_parse_Itembound_ack;
	intif->pMercenaryReceived = intif_parse_MercenaryReceived;
	intif->pMercenaryDeleted = intif_parse_MercenaryDeleted;
	intif->pMercenarySaved = intif_parse_MercenarySaved;
	intif->pElementalReceived = intif_parse_ElementalReceived;
	intif->pElementalDeleted = intif_parse_ElementalDeleted;
	intif->pElementalSaved = intif_parse_ElementalSaved;
	intif->pCreatePet = intif_parse_CreatePet;
	intif->pRecvPetData = intif_parse_RecvPetData;
	intif->pSavePetOk = intif_parse_SavePetOk;
	intif->pDeletePetOk = intif_parse_DeletePetOk;
	intif->pCreateHomunculus = intif_parse_CreateHomunculus;
	intif->pRecvHomunculusData = intif_parse_RecvHomunculusData;
	intif->pSaveHomunculusOk = intif_parse_SaveHomunculusOk;
	intif->pDeleteHomunculusOk = intif_parse_DeleteHomunculusOk;
}
