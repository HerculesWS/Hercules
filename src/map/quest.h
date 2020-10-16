/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2021 Hercules Dev Team
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
#ifndef MAP_QUEST_H
#define MAP_QUEST_H

#include "common/hercules.h"
#include "common/mmo.h" // enum quest_state

/* Forward Declarations */
struct block_list;
struct config_setting_t;
struct map_session_data;
struct questinfo;
struct mob_data;

#define MAX_QUEST_DB (60355+1) // Highest quest ID + 1

struct quest_dropitem {
	int mob_id;
	int nameid;
	int rate;
};

struct quest_objective {
	int mob;
	int count;
	struct {
		int min;
		int max;
	} level;
};

struct quest_db {
	int id;
	unsigned int time;
	int objectives_count;
	struct quest_objective *objectives;
	int dropitem_count;
	struct quest_dropitem *dropitem;
	//char name[NAME_LENGTH];
};

// Questlog check types
enum quest_check_type {
	HAVEQUEST, ///< Query the state of the given quest
	PLAYTIME,  ///< Check if the given quest has been completed or has yet to expire
	HUNTING,   ///< Check if the given hunting quest's requirements have been met
};

struct questinfo_qreq {
	int id;
	int state;
};

struct questinfo_itemreq {
	int nameid;
	int min;
	int max;
};

struct questinfo {
	unsigned short icon;
	unsigned char color;
	bool hasJob;
	unsigned int job;/* perhaps a mapid mask would be most flexible? */
	bool sex_enabled;
	int sex;
	struct {
		int min;
		int max;
	} base_level;
	struct {
		int min;
		int max;
	} job_level;
	VECTOR_DECL(struct questinfo_itemreq) items;
	struct s_homunculus homunculus;
	int homunculus_type;
	VECTOR_DECL(struct questinfo_qreq) quest_requirement;
	int mercenary_class;
};

struct quest_interface {
	struct quest_db **db_data; ///< Quest database
	struct quest_db dummy;                  ///< Dummy entry for invalid quest lookups
	/* */
	void (*init) (bool minimal);
	void (*final) (void);
	void (*reload) (void);
	/* */
	struct quest_db *(*db) (int quest_id);
	int (*pc_login) (struct map_session_data *sd);
	int (*add) (struct map_session_data *sd, int quest_id, unsigned int time_limit);
	int (*change) (struct map_session_data *sd, int qid1, int qid2);
	int (*delete) (struct map_session_data *sd, int quest_id);
	int (*update_objective_sub) (struct block_list *bl, va_list ap);
	void (*update_objective) (struct map_session_data *sd, const struct mob_data *md);
	int (*update_status) (struct map_session_data *sd, int quest_id, enum quest_state qs);
	int (*check) (struct map_session_data *sd, int quest_id, enum quest_check_type type);
	void (*clear) (void);
	int (*read_db) (void);
	struct quest_db *(*read_db_sub) (struct config_setting_t *cs, int n, const char *source);

	int (*questinfo_validate_icon) (int icon);
	void (*questinfo_refresh) (struct map_session_data *sd);
	bool (*questinfo_validate) (struct map_session_data *sd, struct questinfo *qi);
	bool (*questinfo_validate_job) (struct map_session_data *sd, struct questinfo *qi);
	bool (*questinfo_validate_sex) (struct map_session_data *sd, struct questinfo *qi);
	bool (*questinfo_validate_baselevel) (struct map_session_data *sd, struct questinfo *qi);
	bool (*questinfo_validate_joblevel) (struct map_session_data *sd, struct questinfo *qi);
	bool (*questinfo_validate_items) (struct map_session_data *sd, struct questinfo *qi);
	bool (*questinfo_validate_homunculus_level) (struct map_session_data *sd, struct questinfo *qi);
	bool (*questinfo_validate_homunculus_type) (struct map_session_data *sd, struct questinfo *qi);
	bool (*questinfo_validate_quests) (struct map_session_data *sd, struct questinfo *qi);
	bool (*questinfo_validate_mercenary_class) (struct map_session_data *sd, struct questinfo *qi);
};

#ifdef HERCULES_CORE
void quest_defaults(void);
#endif // HERCULES_CORE

HPShared struct quest_interface *quest;

#endif /* MAP_QUEST_H */
