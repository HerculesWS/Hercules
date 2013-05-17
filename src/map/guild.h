// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef _GUILD_H_
#define _GUILD_H_

//#include "../common/mmo.h"
struct guild;
struct guild_member;
struct guild_position;
struct guild_castle;
#include "map.h" // NAME_LENGTH
struct map_session_data;
struct mob_data;

//For quick linking to a guardian's info. [Skotlex]
struct guardian_data {
	int number; //0-MAX_GUARDIANS-1 = Guardians. MAX_GUARDIANS = Emperium.
	int guild_id;
	int emblem_id;
	int guardup_lv; //Level of GD_GUARDUP skill.
	char guild_name[NAME_LENGTH];
	struct guild_castle* castle;
};

struct guild_interface {
	void (*init) (void);
	void (*final) (void);
	/* */
	int (*skill_get_max) (int id);
	/* */
	int (*checkskill) (struct guild *g,int id);
	int (*check_skill_require) (struct guild *g,int id); // [Komurka]
	int (*checkcastles) (struct guild *g); // [MouseJstr]
	bool (*isallied) (int guild_id, int guild_id2); //Checks alliance based on guild Ids. [Skotlex]
	/* */
	struct guild *(*search) (int guild_id);
	struct guild *(*searchname) (char *str);
	struct guild_castle *(*castle_search) (int gcid);
	/* */
	struct guild_castle *(*mapname2gc) (const char* mapname);
	struct guild_castle *(*mapindex2gc) (short mapindex);
	/* */
	struct map_session_data *(*getavailablesd) (struct guild *g);
	int (*getindex) (struct guild *g,int account_id,int char_id);
	int (*getposition) (struct guild *g, struct map_session_data *sd);
	unsigned int (*payexp) (struct map_session_data *sd,unsigned int exp);
	int (*getexp) (struct map_session_data *sd,int exp); // [Celest]
	/* */
	int (*create) (struct map_session_data *sd, const char *name);
	int (*created) (int account_id,int guild_id);
	int (*request_info) (int guild_id);
	int (*recv_noinfo) (int guild_id);
	int (*recv_info) (struct guild *sg);
	int (*npc_request_info) (int guild_id,const char *ev);
	int (*invite) (struct map_session_data *sd,struct map_session_data *tsd);
	int (*reply_invite) (struct map_session_data *sd,int guild_id,int flag);
	void (*member_joined) (struct map_session_data *sd);
	int (*member_added) (int guild_id,int account_id,int char_id,int flag);
	int (*leave) (struct map_session_data *sd,int guild_id,int account_id,int char_id,const char *mes);
	int (*member_withdraw) (int guild_id,int account_id,int char_id,int flag,const char *name,const char *mes);
	int (*expulsion) (struct map_session_data *sd,int guild_id,int account_id,int char_id,const char *mes);
	int (*skillup) (struct map_session_data* sd, uint16 skill_id);
	void (*block_skill) (struct map_session_data *sd, int time);
	int (*reqalliance) (struct map_session_data *sd,struct map_session_data *tsd);
	int (*reply_reqalliance) (struct map_session_data *sd,int account_id,int flag);
	int (*allianceack) (int guild_id1,int guild_id2,int account_id1,int account_id2,int flag,const char *name1,const char *name2);
	int (*delalliance) (struct map_session_data *sd,int guild_id,int flag);
	int (*opposition) (struct map_session_data *sd,struct map_session_data *tsd);
	int (*check_alliance) (int guild_id1, int guild_id2, int flag);
	/* */
	int (*send_memberinfoshort) (struct map_session_data *sd,int online);
	int (*recv_memberinfoshort) (int guild_id,int account_id,int char_id,int online,int lv,int class_);
	int (*change_memberposition) (int guild_id,int account_id,int char_id,short idx);
	int (*memberposition_changed) (struct guild *g,int idx,int pos);
	int (*change_position) (int guild_id,int idx,int mode,int exp_mode,const char *name);
	int (*position_changed) (int guild_id,int idx,struct guild_position *p);
	int (*change_notice) (struct map_session_data *sd,int guild_id,const char *mes1,const char *mes2);
	int (*notice_changed) (int guild_id,const char *mes1,const char *mes2);
	int (*change_emblem) (struct map_session_data *sd,int len,const char *data);
	int (*emblem_changed) (int len,int guild_id,int emblem_id,const char *data);
	int (*send_message) (struct map_session_data *sd,const char *mes,int len);
	int (*recv_message) (int guild_id,int account_id,const char *mes,int len);
	int (*send_dot_remove) (struct map_session_data *sd);
	int (*skillupack) (int guild_id,uint16 skill_id,int account_id);
	int (*dobreak) (struct map_session_data *sd,char *name);
	int (*broken) (int guild_id,int flag);
	int (*gm_change) (int guild_id, struct map_session_data *sd);
	int (*gm_changed) (int guild_id, int account_id, int char_id);
	/* */
	void (*castle_map_init) (void);
	int (*castledatasave) (int castle_id,int index,int value);
	int (*castledataloadack) (int len, struct guild_castle *gc);
	void (*castle_reconnect) (int castle_id, int index, int value);
	/* */
	void (*agit_start) (void);
	void (*agit_end) (void);
	void (*agit2_start) (void);
	void (*agit2_end) (void);
	/* guild flag cachin */
	void (*flag_add) (struct npc_data *nd);
	void (*flag_remove) (struct npc_data *nd);
	void (*flags_clear) (void);
	/* guild aura */
	void (*aura_refresh) (struct map_session_data *sd, uint16 skill_id, uint16 skill_lv);
} guild_s;

struct guild_interface *guild;

void guild_defaults(void);

#endif /* _GUILD_H_ */
