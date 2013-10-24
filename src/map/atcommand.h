// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef _ATCOMMAND_H_
#define _ATCOMMAND_H_

#include "../common/conf.h"
#include "../common/db.h"
#include "pc_groups.h"

/**
 * Declarations
 **/
struct map_session_data;
struct AtCommandInfo;
struct block_list;

/**
 * Defines
 **/
#define ATCOMMAND_LENGTH 50
#define MAX_MSG 1500

/**
 * Enumerations
 **/
typedef enum {
	COMMAND_ATCOMMAND = 1,
	COMMAND_CHARCOMMAND = 2,
} AtCommandType;

/**
 * Typedef
 **/
typedef bool (*AtCommandFunc)(const int fd, struct map_session_data* sd, const char* command, const char* message,struct AtCommandInfo *info);
typedef struct AtCommandInfo AtCommandInfo;
typedef struct AliasInfo AliasInfo;

/**
 * Structures
 **/
struct AliasInfo {
	AtCommandInfo *command;
	char alias[ATCOMMAND_LENGTH];
};

struct AtCommandInfo {
	char command[ATCOMMAND_LENGTH];
	AtCommandFunc func;
	char *at_groups;/* quick @commands "can-use" lookup */
	char *char_groups;/* quick @charcommands "can-use" lookup */
	char *help;/* quick access to this @command's help string */
	bool log;/* whether to log this command or not, regardless of group settings */
};

struct atcmd_binding_data {
	char command[ATCOMMAND_LENGTH];
	char npc_event[ATCOMMAND_LENGTH];
	int group_lv;
	int group_lv_char;
	bool log;
};

/**
 * Interface
 **/
struct atcommand_interface {
	unsigned char at_symbol;
	unsigned char char_symbol;
	/* atcommand binding */
	struct atcmd_binding_data** binding;
	int binding_count;
	/* other vars */
	DBMap* db; //name -> AtCommandInfo
	DBMap* alias_db; //alias -> AtCommandInfo
	/* */
	char* msg_table[MAX_MSG]; // Server messages (0-499 reserved for GM commands, 500-999 reserved for others)
	/* */
	void (*init) (void);
	void (*final) (void);
	/* */
	bool (*parse) (const int fd, struct map_session_data* sd, const char* message, int type);
	bool (*create) (char *name, AtCommandFunc func);
	bool (*can_use) (struct map_session_data *sd, const char *command);
	bool (*can_use2) (struct map_session_data *sd, const char *command, AtCommandType type);
	void (*load_groups) (GroupSettings **groups, config_setting_t **commands_, size_t sz);
	AtCommandInfo* (*exists) (const char* name);
	int (*msg_read) (const char* cfgName);
	void (*final_msg) (void);
	/* atcommand binding */
	struct atcmd_binding_data* (*get_bind_byname) (const char* name);
	/* */
	AtCommandInfo* (*get_info_byname) (const char *name); // @help
	const char* (*check_alias) (const char *aliasname); // @help
	void (*get_suggestions) (struct map_session_data* sd, const char *name, bool is_atcmd_cmd); // @help
	void (*config_read) (const char* config_filename);
	/* command-specific subs */
	int (*stopattack) (struct block_list *bl,va_list ap);
	int (*pvpoff_sub) (struct block_list *bl,va_list ap);
	int (*pvpon_sub) (struct block_list *bl,va_list ap);
	int (*atkillmonster_sub) (struct block_list *bl, va_list ap);
	void (*raise_sub) (struct map_session_data* sd);
	void (*get_jail_time) (int jailtime, int* year, int* month, int* day, int* hour, int* minute);
	int (*cleanfloor_sub) (struct block_list *bl, va_list ap);
	int (*mutearea_sub) (struct block_list *bl,va_list ap);
	/* */
	void (*commands_sub) (struct map_session_data* sd, const int fd, AtCommandType type);
	void (*cmd_db_clear) (void);
	int (*cmd_db_clear_sub) (DBKey key, DBData *data, va_list args);
	void (*doload) (void);
	void (*base_commands) (void);
	bool (*add) (char *name, AtCommandFunc func);
};

struct atcommand_interface *atcommand;

/* will remain outside for a while, we have plans for this little fellow */
const char* msg_txt(int msg_number);

void atcommand_defaults(void);

/* stay here */
#define ACMD(x) static bool atcommand_ ## x (const int fd, struct map_session_data* sd, const char* command, const char* message, struct AtCommandInfo *info)
#define ACMD_A(x) atcommand_ ## x

#endif /* _ATCOMMAND_H_ */
