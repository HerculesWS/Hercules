/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2023 Hercules Dev Team
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
#define QUEST_MAPWIDE_ID 10363 // MobId handled by the client to display MapName
#define QUEST_MOBTYPE_ID 31999 // MobId handled by the client to display Mob properties

#define quest_mobtype2client(type) (\
	((type).size_enabled ? quest->mobsize2client((type).size) : 0) \
	| ((type).ele_enabled ? quest->mobele2client((type).ele) : 0) \
	| ((type).race_enabled ? quest->mobrace2client((type).race) : 0))
#define quest_mobtypeisenabled(type) ((type).size_enabled || (type).ele_enabled || (type).race_enabled)

enum quest_mobtype {
	// Monster Sizes
	QMT_SZ_SMALL         = 0x10,
	QMT_SZ_MEDIUM        = 0x20,
	QMT_SZ_LARGE         = 0x40,

	// Monster Races
	QMT_RC_DEMIHUMAN     = 0x80,
	QMT_RC_BRUTE         = 0x100,
	QMT_RC_INSECT        = 0x200,
	QMT_RC_FISH          = 0x400,
	QMT_RC_PLANT         = 0x800,
	QMT_RC_DEMON         = 0x1000,
	QMT_RC_ANGEL         = 0x2000,
	QMT_RC_UNDEAD        = 0x4000,
	QMT_RC_FORMLESS      = 0x8000,
	QMT_RC_DRAGON        = 0x10000,

	// Monster Elements
	QMT_ELE_WATER        = 0x20000,
	QMT_ELE_WIND         = 0x40000,
	QMT_ELE_EARTH        = 0x80000,
	QMT_ELE_FIRE         = 0x100000,
	QMT_ELE_DARK         = 0x200000,
	QMT_ELE_HOLY         = 0x400000,
	QMT_ELE_POISON       = 0x800000,
	QMT_ELE_GHOST        = 0x1000000,
	QMT_ELE_NEUTRAL      = 0x2000000,
	QMT_ELE_UNDEAD       = 0x4000000
};

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
	struct {
		uint8 size;
		uint8 race;
		uint8 ele;

		bool size_enabled;
		bool race_enabled;
		bool ele_enabled;
	} mobtype;
	int16 mapid;
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
	int (*setting_lookup_const) (struct config_setting_t *tt, const char *name, int *value, int quest_id, int idx, const char *kind, const char *source);

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
	enum quest_mobtype (*mobsize2client) (uint8 size);
	enum quest_mobtype (*mobele2client) (uint8 type);
	enum quest_mobtype (*mobrace2client) (uint8 type);
};

#ifdef HERCULES_CORE
void quest_defaults(void);
#endif // HERCULES_CORE

HPShared struct quest_interface *quest;

#endif /* MAP_QUEST_H */
