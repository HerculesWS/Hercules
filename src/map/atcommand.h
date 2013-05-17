// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef _ATCOMMAND_H_
#define _ATCOMMAND_H_

#include "../common/db.h"

/**
 * Declarations
 **/
struct map_session_data;
struct AtCommandInfo;

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
	unsigned int *group_ids;
	/* other vars */
	DBMap* db; //name -> AtCommandInfo
	DBMap* alias_db; //alias -> AtCommandInfo
	/* */
	void (*init) (void);
	void (*final) (void);
	/* */
	bool (*parse) (const int fd, struct map_session_data* sd, const char* message, int type);
	bool (*create) (char *name, AtCommandFunc func);
	bool (*can_use) (struct map_session_data *sd, const char *command);
	bool (*can_use2) (struct map_session_data *sd, const char *command, AtCommandType type);
	void (*load_groups) (void);
	AtCommandInfo* (*exists) (const char* name);
	int (*msg_read) (const char* cfgName);
	void (*final_msg) (void);
	/* atcommand binding */
	struct atcmd_binding_data* (*get_bind_byname) (const char* name);
} atcommand_s;

struct atcommand_interface *atcommand;

const char* msg_txt(int msg_number);
void atcommand_defaults(void);
/* stay here */
#define ACMD(x) static bool atcommand_ ## x (const int fd, struct map_session_data* sd, const char* command, const char* message, struct AtCommandInfo *info)
#define ACMD_A(x) atcommand_ ## x

#endif /* _ATCOMMAND_H_ */
