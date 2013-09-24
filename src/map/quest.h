// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef _QUEST_H_
#define _QUEST_H_

struct s_quest_db {
	int id;
	unsigned int time;
	int mob[MAX_QUEST_OBJECTIVES];
	int count[MAX_QUEST_OBJECTIVES];
	int num_objectives;
	//char name[NAME_LENGTH];
};

typedef enum quest_check_type { HAVEQUEST, PLAYTIME, HUNTING } quest_check_type;

struct quest_interface {
	struct s_quest_db db[MAX_QUEST_DB];
	/* */
	void (*init) (void);
	void (*reload) (void);
	/* */
	int (*search_db) (int quest_id);
	int (*pc_login) (TBL_PC *sd);
	int (*add) (TBL_PC *sd, int quest_id);
	int (*change) (TBL_PC *sd, int qid1, int qid2);
	int (*delete) (TBL_PC *sd, int quest_id);
	int (*update_objective_sub) (struct block_list *bl, va_list ap);
	void (*update_objective) (TBL_PC *sd, int mob_id);
	int (*update_status) (TBL_PC *sd, int quest_id, quest_state status);
	int (*check) (TBL_PC *sd, int quest_id, quest_check_type type);
	int (*read_db) (void);
};

struct quest_interface *quest;

void quest_defaults(void);

#endif
