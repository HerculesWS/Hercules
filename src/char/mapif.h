/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2023 Hercules Dev Team
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
#ifndef CHAR_MAPIF_H
#define CHAR_MAPIF_H

#include "common/hercules.h"
#include "common/mmo.h"

struct rodex_item;
enum adventurer_agency_result;

typedef int (*SendAll_func)(int fd, va_list args);

/**
 * mapif interface
 **/
struct mapif_interface {
	void (*ban) (int id, unsigned int flag, int status);
	void (*server_init) (void);
	void (*server_destroy) (void);
	void (*server_reset) (void);
	void (*on_disconnect) (void);
	void (*on_parse_accinfo) (int account_id, int u_fd, int u_aid, int u_group, int map_fd);
	void (*char_ban) (int char_id, time_t timestamp);
	int (*send) (const unsigned char *buf, unsigned int len);
	void (*send_users_count) (int users);
	void (*pLoadAchievements) (int fd);
	void (*sAchievementsToMap) (int fd, int char_id, const struct char_achievements *p);
	void (*pSaveAchievements) (int fd);
	void (*achievement_load) (int fd, int char_id);
	void (*achievement_save) (int char_id, struct char_achievements *p);
	void (*auction_message) (int char_id, unsigned char result);
	void (*auction_sendlist) (int fd, int char_id, short count, short pages, unsigned char *buf);
	void (*parse_auction_requestlist) (int fd);
	void (*auction_register) (int fd, struct auction_data *auction);
	void (*parse_auction_register) (int fd);
	void (*auction_cancel) (int fd, int char_id, unsigned char result);
	void (*parse_auction_cancel) (int fd);
	void (*auction_close) (int fd, int char_id, unsigned char result);
	void (*parse_auction_close) (int fd);
	void (*auction_bid) (int fd, int char_id, int bid, unsigned char result);
	void (*parse_auction_bid) (int fd);
	void (*elemental_send) (int fd, struct s_elemental *ele, unsigned char flag);
	void (*parse_elemental_create) (int fd, const struct s_elemental *ele);
	void (*parse_elemental_load) (int fd, int ele_id, int char_id);
	void (*elemental_deleted) (int fd, unsigned char flag);
	void (*parse_elemental_delete) (int fd, int ele_id);
	void (*elemental_saved) (int fd, unsigned char flag);
	void (*parse_elemental_save) (int fd, const struct s_elemental *ele);
	int (*guild_created) (int fd, int account_id, struct guild *g);
	int (*guild_noinfo) (int guild_id);
	int (*guild_info) (const struct guild *g);
	int (*guild_info_basic) (const struct guild *g);
	int (*guild_info_emblem) (const struct guild *g);
	int (*guild_memberadded) (int guild_id, int account_id, int char_id, int flag);
	int (*guild_withdraw) (int guild_id, int account_id, int char_id, int flag, const char *name, const char *mes);
	int (*guild_memberinfoshort) (struct guild *g, int idx);
	int (*guild_broken) (int guild_id, int flag);
	int (*guild_basicinfochanged) (int guild_id, int type, const void *data, int len);
	int (*guild_memberinfochanged) (int guild_id, int account_id, int char_id, int type, const void *data, int len);
	int (*guild_skillupack) (int guild_id, uint16 skill_id, int account_id);
	int (*guild_alliance) (int guild_id1, int guild_id2, int account_id1, int account_id2, int flag, const char *name1, const char *name2);
	int (*guild_position) (struct guild *g, int idx);
	int (*guild_notice) (struct guild *g);
	int (*guild_emblem) (struct guild *g);
	int (*guild_emblem_sub) (int fd, va_list args);
	int (*guild_master_changed) (struct guild *g, int aid, int cid);
	int (*guild_castle_dataload) (int fd, int sz, const int *castle_ids);
	int (*parse_CreateGuild) (int fd, int account_id, const char *name, const struct guild_member *master);
	int (*parse_GuildInfo) (int fd, int guild_id);
	int (*parse_GuildAddMember) (int fd, int guild_id, const struct guild_member *m);
	int (*parse_GuildLeave) (int fd, int guild_id, int account_id, int char_id, int flag, const char *mes);
	int (*parse_GuildChangeMemberInfoShort) (int fd, int guild_id, int account_id, int char_id, int online, int lv, int class);
	int (*parse_BreakGuild) (int fd, int guild_id);
	int (*parse_GuildBasicInfoChange) (int fd, int guild_id, int type, const void *data, int len);
	int (*parse_GuildMemberInfoChange) (int fd, int guild_id, int account_id, int char_id, int type, const char *data, int len);
	int (*parse_GuildPosition) (int fd, int guild_id, int idx, const struct guild_position *p);
	int (*parse_GuildSkillUp) (int fd, int guild_id, uint16 skill_id, int account_id, int max);
	int (*parse_GuildAlliance) (int fd, int guild_id1, int guild_id2, int account_id1, int account_id2, int flag);
	int (*parse_GuildNotice) (int fd, int guild_id, const char *mes1, const char *mes2);
	int (*parse_GuildEmblem) (int fd);
	int (*parse_GuildCastleDataLoad) (int fd, int len, const int *castle_ids);
	int (*parse_GuildCastleDataSave) (int fd, int castle_id, int index, int value);
	int (*parse_GuildMasterChange) (int fd, int guild_id, const char* name, int len);
	void (*homunculus_created) (int fd, int account_id, const struct s_homunculus *sh, unsigned char flag);
	void (*homunculus_deleted) (int fd, int flag);
	void (*homunculus_loaded) (int fd, int account_id, struct s_homunculus *hd);
	void (*homunculus_saved) (int fd, int account_id, bool flag);
	void (*homunculus_renamed) (int fd, int account_id, int char_id, unsigned char flag, const char *name);
	void (*parse_homunculus_create) (int fd, int len, int account_id, const struct s_homunculus *phd);
	void (*parse_homunculus_delete) (int fd, int homun_id);
	void (*parse_homunculus_load) (int fd, int account_id, int homun_id);
	void (*parse_homunculus_save) (int fd, int len, int account_id, const struct s_homunculus *phd);
	void (*parse_homunculus_rename) (int fd, int account_id, int char_id, const char *name);
	void (*mail_sendinbox) (int fd, int char_id, unsigned char flag, struct mail_data *md);
	void (*parse_mail_requestinbox) (int fd);
	void (*parse_mail_read) (int fd);
	void (*mail_sendattach) (int fd, int char_id, struct mail_message *msg);
	void (*parse_mail_getattach) (int fd);
	void (*mail_delete) (int fd, int char_id, int mail_id, bool failed);
	void (*parse_mail_delete) (int fd);
	void (*mail_new) (struct mail_message *msg);
	void (*mail_return) (int fd, int char_id, int mail_id, int new_mail);
	void (*parse_mail_return) (int fd);
	void (*mail_send) (int fd, struct mail_message* msg);
	void (*parse_mail_send) (int fd);
	void (*mercenary_send) (int fd, struct s_mercenary *merc, unsigned char flag);
	void (*parse_mercenary_create) (int fd, const struct s_mercenary *merc);
	void (*parse_mercenary_load) (int fd, int merc_id, int char_id);
	void (*mercenary_deleted) (int fd, unsigned char flag);
	void (*parse_mercenary_delete) (int fd, int merc_id);
	void (*mercenary_saved) (int fd, unsigned char flag);
	void (*parse_mercenary_save) (int fd, const struct s_mercenary *merc);
	int (*party_created) (int fd, int account_id, int char_id, struct party *p);
	void (*party_noinfo) (int fd, int party_id, int char_id);
	void (*party_info) (const struct party *p, int char_id);
	int (*party_memberadded) (int fd, int party_id, int account_id, int char_id, int flag);
	int (*party_optionchanged) (const struct party *p, int account_id, int flag);
	int (*party_withdraw) (int party_id,int account_id, int char_id);
	int (*party_membermoved) (struct party *p, int idx);
	int (*party_broken) (int party_id, int flag);
	int (*parse_CreateParty) (int fd, const char *name, int item, int item2, const struct party_member *leader);
	void (*parse_PartyInfo) (int fd, int party_id, int char_id);
	int (*parse_PartyAddMember) (int fd, int party_id, const struct party_member *member);
	int (*parse_PartyChangeOption) (int fd,int party_id,int account_id,int exp,int item);
	int (*parse_PartyLeave) (int fd, int party_id, int account_id, int char_id);
	int (*parse_PartyChangeMap) (int fd, int party_id, int account_id, int char_id, unsigned short map, int online, unsigned int lv);
	int (*parse_BreakParty) (int fd, int party_id);
	int (*parse_PartyLeaderChange) (int fd, int party_id, int account_id, int char_id);
	int (*pet_created) (int fd, int account_id, struct s_pet *p);
	int (*pet_info) (int fd, int account_id, struct s_pet *p);
	int (*pet_noinfo) (int fd, int account_id);
	int (*save_pet_ack) (int fd, int account_id, int flag);
	int (*delete_pet_ack) (int fd, int flag);
	int (*save_pet) (int fd, int account_id, const struct s_pet *data);
	int (*delete_pet) (int fd, int pet_id);
	int (*parse_CreatePet) (int fd);
	int (*parse_LoadPet) (int fd);
	int (*parse_SavePet) (int fd);
	int (*parse_DeletePet) (int fd);
	void (*quest_save_ack) (int fd, int char_id, bool success);
	int (*parse_quest_save) (int fd);
	void (*send_quests) (int fd, int char_id, struct quest *tmp_questlog, int num_quests);
	int (*parse_quest_load) (int fd);
	void (*parse_rodex_requestinbox) (int fd);
	void (*rodex_sendinbox) (int fd, int char_id, int8 opentype, int8 flag, int count, int64 mail_id, struct rodex_maillist *mails);
	void (*parse_rodex_checkhasnew) (int fd);
	void (*rodex_sendhasnew) (int fd, int char_id, bool has_new);
	void (*parse_rodex_updatemail) (int fd);
	void (*parse_rodex_send) (int fd);
	void (*rodex_send) (int fd, int sender_id, int receiver_id, int receiver_accountid, bool result);
	void (*parse_rodex_checkname) (int fd);
	void (*rodex_checkname) (int fd, int reqchar_id, int target_char_id, int target_class, int target_level, char *name);
	void (*rodex_getzenyack) (int char_id, int64 mail_id, uint8 opentype, int64 zeny);
	void (*rodex_getitemsack) (int char_id, int64 mail_id, uint8 opentype, int count, const struct rodex_item *items);
	int (*load_guild_storage) (int fd, int account_id, int guild_id, char flag);
	int (*save_guild_storage_ack) (int fd, int account_id, int guild_id, int fail);
	int (*parse_LoadGuildStorage) (int fd);
	int (*parse_SaveGuildStorage) (int fd);
	int (*account_storage_load) (int fd, int account_id);
	int (*pAccountStorageLoad) (int fd);
	int (*pAccountStorageSave) (int fd);
	void (*sAccountStorageSaveAck) (int fd, int account_id, bool save);
	int (*itembound_ack) (int fd, int aid, int guild_id);
	void (*parse_ItemBoundRetrieve) (int fd);
	void (*parse_accinfo) (int fd);
	int (*account_reg_reply) (int fd,int account_id,int char_id, int type);
	int (*disconnectplayer) (int account_id, int char_id, int reason);
	int (*parse_Registry) (int fd);
	int (*parse_RegistryRequest) (int fd);
	void (*namechange_ack) (int fd, int account_id, int char_id, int type, int flag, const char *name);
	int (*parse_NameChangeRequest) (int fd);
	// Clan System
	int (*parse_ClanMemberKick) (int fd, int clan_id, int kick_interval);
	int (*parse_ClanMemberCount) (int fd, int clan_id, int kick_interval);
	void (*agency_joinPartyResult) (int fd, int char_id, enum adventurer_agency_result result);
};

#ifdef HERCULES_CORE
void mapif_defaults(void);
#endif // HERCULES_CORE

HPShared struct mapif_interface *mapif;

#endif /* CHAR_MAPIF_H */
