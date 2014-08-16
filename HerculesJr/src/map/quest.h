// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef MAP_QUEST_H
#define MAP_QUEST_H

#include "map.h" // TBL_PC
#include "../common/cbasetypes.h"
#include "../common/mmo.h" // MAX_QUEST_OBJECTIVES

#define MAX_QUEST_DB (60355+1) // Highest quest ID + 1

struct quest_db {
	int id;
	unsigned int time;
	int mob[MAX_QUEST_OBJECTIVES];
	int count[MAX_QUEST_OBJECTIVES];
	int num_objectives;
	//char name[NAME_LENGTH];
};

// Questlog check types
enum quest_check_type {
	HAVEQUEST, ///< Query the state of the given quest
	PLAYTIME,  ///< Check if the given quest has been completed or has yet to expire
	HUNTING,   ///< Check if the given hunting quest's requirements have been met
};

struct quest_interface {
	struct quest_db *db_data[MAX_QUEST_DB]; ///< Quest database
	struct quest_db dummy;                  ///< Dummy entry for invalid quest lookups
	/* */
	void (*init) (bool minimal);
	void (*final) (void);
	void (*reload) (void);
	/* */
	struct quest_db *(*db) (int quest_id);
	int (*pc_login) (TBL_PC *sd);
	int (*add) (TBL_PC *sd, int quest_id);
	int (*change) (TBL_PC *sd, int qid1, int qid2);
	int (*delete) (TBL_PC *sd, int quest_id);
	int (*update_objective_sub) (struct block_list *bl, va_list ap);
	void (*update_objective) (TBL_PC *sd, int mob_id);
	int (*update_status) (TBL_PC *sd, int quest_id, enum quest_state qs);
	int (*check) (TBL_PC *sd, int quest_id, enum quest_check_type type);
	void (*clear) (void);
	int (*read_db) (void);
};

struct quest_interface *quest;

void quest_defaults(void);

#endif /* MAP_QUEST_H */
