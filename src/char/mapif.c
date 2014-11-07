// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#define HERCULES_CORE

#include "mapif.h"

#include <stdlib.h>

#include "char.h"
#include "int_auction.h"
#include "int_homun.h"
#include "int_guild.h"
#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/random.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"

void mapif_ban(int id, unsigned int flag, int status);
void mapif_server_init(int id);
void mapif_server_destroy(int id);
void mapif_server_reset(int id);
void mapif_on_disconnect(int id);
void mapif_on_parse_accinfo(int account_id, int u_fd, int u_aid, int u_group, int map_fd);
void mapif_char_ban(int char_id, time_t timestamp);
int mapif_sendall(unsigned char *buf, unsigned int len);
int mapif_sendallwos(int sfd, unsigned char *buf, unsigned int len);
int mapif_send(int fd, unsigned char *buf, unsigned int len);
void mapif_send_users_count(int users);
void mapif_auction_message(int char_id, unsigned char result);
void mapif_auction_sendlist(int fd, int char_id, short count, short pages, unsigned char *buf);
void mapif_parse_auction_requestlist(int fd);
void mapif_auction_register(int fd, struct auction_data *auction);
void mapif_parse_auction_register(int fd);
void mapif_auction_cancel(int fd, int char_id, unsigned char result);
void mapif_parse_auction_cancel(int fd);
void mapif_auction_close(int fd, int char_id, unsigned char result);
void mapif_parse_auction_close(int fd);
void mapif_auction_bid(int fd, int char_id, int bid, unsigned char result);
void mapif_parse_auction_bid(int fd);
bool mapif_elemental_save(struct s_elemental* ele);
bool mapif_elemental_load(int ele_id, int char_id, struct s_elemental *ele);
bool mapif_elemental_delete(int ele_id);
void mapif_elemental_send(int fd, struct s_elemental *ele, unsigned char flag);
void mapif_parse_elemental_create(int fd, struct s_elemental* ele);
void mapif_parse_elemental_load(int fd, int ele_id, int char_id);
void mapif_elemental_deleted(int fd, unsigned char flag);
void mapif_parse_elemental_delete(int fd, int ele_id);
void mapif_elemental_saved(int fd, unsigned char flag);
void mapif_parse_elemental_save(int fd, struct s_elemental* ele);
int mapif_guild_created(int fd, int account_id, struct guild *g);
int mapif_guild_noinfo(int fd, int guild_id);
int mapif_guild_info(int fd, struct guild *g);
int mapif_guild_memberadded(int fd, int guild_id, int account_id, int char_id, int flag);
int mapif_guild_withdraw(int guild_id, int account_id, int char_id, int flag, const char *name, const char *mes);
int mapif_guild_memberinfoshort(struct guild *g, int idx);
int mapif_guild_broken(int guild_id, int flag);
int mapif_guild_message(int guild_id,int account_id,char *mes,int len, int sfd);
int mapif_guild_basicinfochanged(int guild_id, int type, const void *data, int len);
int mapif_guild_memberinfochanged(int guild_id, int account_id, int char_id, int type, const void *data, int len);
int mapif_guild_skillupack(int guild_id, uint16 skill_id, int account_id);
int mapif_guild_alliance(int guild_id1, int guild_id2, int account_id1, int account_id2, int flag, const char *name1, const char *name2);
int mapif_guild_position(struct guild *g, int idx);
int mapif_guild_notice(struct guild *g);
int mapif_guild_emblem(struct guild *g);
int mapif_guild_master_changed(struct guild *g, int aid, int cid);
int mapif_guild_castle_dataload(int fd, int sz, int *castle_ids);
int mapif_parse_CreateGuild(int fd,int account_id,char *name,struct guild_member *master);
int mapif_parse_GuildInfo(int fd, int guild_id);
int mapif_parse_GuildAddMember(int fd, int guild_id, struct guild_member *m);
int mapif_parse_GuildLeave(int fd, int guild_id, int account_id, int char_id, int flag, const char *mes);
int mapif_parse_GuildChangeMemberInfoShort(int fd, int guild_id, int account_id, int char_id, int online, int lv, int class_);
int mapif_parse_BreakGuild(int fd, int guild_id);
int mapif_parse_GuildMessage(int fd, int guild_id, int account_id, char *mes, int len);
int mapif_parse_GuildBasicInfoChange(int fd, int guild_id, int type, const void *data, int len);
int mapif_parse_GuildMemberInfoChange(int fd, int guild_id, int account_id, int char_id, int type, const char *data, int len);
int mapif_parse_GuildPosition(int fd, int guild_id, int idx, struct guild_position *p);
int mapif_parse_GuildSkillUp(int fd, int guild_id, uint16 skill_id, int account_id, int max);
int mapif_parse_GuildDeleteAlliance(struct guild *g, int guild_id, int account_id1, int account_id2, int flag);
int mapif_parse_GuildAlliance(int fd, int guild_id1, int guild_id2, int account_id1, int account_id2, int flag);
int mapif_parse_GuildNotice(int fd, int guild_id, const char *mes1, const char *mes2);
int mapif_parse_GuildEmblem(int fd, int len, int guild_id, int dummy, const char *data);
int mapif_parse_GuildCastleDataLoad(int fd, int len, int *castle_ids);
int mapif_parse_GuildCastleDataSave(int fd, int castle_id, int index, int value);
int mapif_parse_GuildMasterChange(int fd, int guild_id, const char* name, int len);
void mapif_homunculus_created(int fd, int account_id, struct s_homunculus *sh, unsigned char flag);
void mapif_homunculus_deleted(int fd, int flag);
void mapif_homunculus_loaded(int fd, int account_id, struct s_homunculus *hd);
void mapif_homunculus_saved(int fd, int account_id, bool flag);
void mapif_homunculus_renamed(int fd, int account_id, int char_id, unsigned char flag, char* name);
bool mapif_homunculus_save(struct s_homunculus* hd);
bool mapif_homunculus_load(int homun_id, struct s_homunculus* hd);
bool mapif_homunculus_delete(int homun_id);
bool mapif_homunculus_rename(char *name);
void mapif_parse_homunculus_create(int fd, int len, int account_id, struct s_homunculus* phd);
void mapif_parse_homunculus_delete(int fd, int homun_id);
void mapif_parse_homunculus_load(int fd, int account_id, int homun_id);
void mapif_parse_homunculus_save(int fd, int len, int account_id, struct s_homunculus* phd);
void mapif_parse_homunculus_rename(int fd, int account_id, int char_id, char* name);

void mapif_defaults(void) {
	mapif = &mapif_s;

	mapif->ban = mapif_ban;
	mapif->server_init = mapif_server_init;
	mapif->server_destroy = mapif_server_destroy;
	mapif->server_reset = mapif_server_reset;
	mapif->on_disconnect = mapif_on_disconnect;
	mapif->on_parse_accinfo = mapif_on_parse_accinfo;
	mapif->char_ban = mapif_char_ban;
	mapif->sendall = mapif_sendall;
	mapif->sendallwos = mapif_sendallwos;
	mapif->send = mapif_send;
	mapif->send_users_count = mapif_send_users_count;
	mapif->auction_message = mapif_auction_message;
	mapif->auction_sendlist = mapif_auction_sendlist;
	mapif->parse_auction_requestlist = mapif_parse_auction_requestlist;
	mapif->auction_register = mapif_auction_register;
	mapif->parse_auction_register = mapif_parse_auction_register;
	mapif->auction_cancel = mapif_auction_cancel;
	mapif->parse_auction_cancel = mapif_parse_auction_cancel;
	mapif->auction_close = mapif_auction_close;
	mapif->parse_auction_close = mapif_parse_auction_close;
	mapif->auction_bid = mapif_auction_bid;
	mapif->parse_auction_bid = mapif_parse_auction_bid;
	mapif->elemental_save = mapif_elemental_save;
	mapif->elemental_load = mapif_elemental_load;
	mapif->elemental_delete = mapif_elemental_delete;
	mapif->elemental_send = mapif_elemental_send;
	mapif->parse_elemental_create = mapif_parse_elemental_create;
	mapif->parse_elemental_load = mapif_parse_elemental_load;
	mapif->elemental_deleted = mapif_elemental_deleted;
	mapif->parse_elemental_delete = mapif_parse_elemental_delete;
	mapif->elemental_saved = mapif_elemental_saved;
	mapif->parse_elemental_save = mapif_parse_elemental_save;
	mapif->guild_created = mapif_guild_created;
	mapif->guild_noinfo = mapif_guild_noinfo;
	mapif->guild_info = mapif_guild_info;
	mapif->guild_memberadded = mapif_guild_memberadded;
	mapif->guild_withdraw = mapif_guild_withdraw;
	mapif->guild_memberinfoshort = mapif_guild_memberinfoshort;
	mapif->guild_broken = mapif_guild_broken;
	mapif->guild_message = mapif_guild_message;
	mapif->guild_basicinfochanged = mapif_guild_basicinfochanged;
	mapif->guild_memberinfochanged = mapif_guild_memberinfochanged;
	mapif->guild_skillupack = mapif_guild_skillupack;
	mapif->guild_alliance = mapif_guild_alliance;
	mapif->guild_position = mapif_guild_position;
	mapif->guild_notice = mapif_guild_notice;
	mapif->guild_emblem = mapif_guild_emblem;
	mapif->guild_master_changed = mapif_guild_master_changed;
	mapif->guild_castle_dataload = mapif_guild_castle_dataload;
	mapif->parse_CreateGuild = mapif_parse_CreateGuild;
	mapif->parse_GuildInfo = mapif_parse_GuildInfo;
	mapif->parse_GuildAddMember = mapif_parse_GuildAddMember;
	mapif->parse_GuildLeave = mapif_parse_GuildLeave;
	mapif->parse_GuildChangeMemberInfoShort = mapif_parse_GuildChangeMemberInfoShort;
	mapif->parse_BreakGuild = mapif_parse_BreakGuild;
	mapif->parse_GuildMessage = mapif_parse_GuildMessage;
	mapif->parse_GuildBasicInfoChange = mapif_parse_GuildBasicInfoChange;
	mapif->parse_GuildMemberInfoChange = mapif_parse_GuildMemberInfoChange;
	mapif->parse_GuildPosition = mapif_parse_GuildPosition;
	mapif->parse_GuildSkillUp = mapif_parse_GuildSkillUp;
	mapif->parse_GuildDeleteAlliance = mapif_parse_GuildDeleteAlliance;
	mapif->parse_GuildAlliance = mapif_parse_GuildAlliance;
	mapif->parse_GuildNotice = mapif_parse_GuildNotice;
	mapif->parse_GuildEmblem = mapif_parse_GuildEmblem;
	mapif->parse_GuildCastleDataLoad = mapif_parse_GuildCastleDataLoad;
	mapif->parse_GuildCastleDataSave = mapif_parse_GuildCastleDataSave;
	mapif->parse_GuildMasterChange = mapif_parse_GuildMasterChange;
	mapif->homunculus_created = mapif_homunculus_created;
	mapif->homunculus_deleted = mapif_homunculus_deleted;
	mapif->homunculus_loaded = mapif_homunculus_loaded;
	mapif->homunculus_saved = mapif_homunculus_saved;
	mapif->homunculus_renamed = mapif_homunculus_renamed;
	mapif->homunculus_save = mapif_homunculus_save;
	mapif->homunculus_load = mapif_homunculus_load;
	mapif->homunculus_delete = mapif_homunculus_delete;
	mapif->homunculus_rename = mapif_homunculus_rename;
	mapif->parse_homunculus_create = mapif_parse_homunculus_create;
	mapif->parse_homunculus_delete = mapif_parse_homunculus_delete;
	mapif->parse_homunculus_load = mapif_parse_homunculus_load;
	mapif->parse_homunculus_save = mapif_parse_homunculus_save;
	mapif->parse_homunculus_rename = mapif_parse_homunculus_rename;
}
