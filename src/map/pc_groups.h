// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef _PC_GROUPS_H_
#define _PC_GROUPS_H_

/// PC permissions
enum e_pc_permission {
	PC_PERM_NONE                = 0,        //  #0
	PC_PERM_TRADE               = 0x000001, //  #1
	PC_PERM_PARTY               = 0x000002,
	PC_PERM_ALL_SKILL           = 0x000004,
	PC_PERM_USE_ALL_EQUIPMENT   = 0x000008,
	PC_PERM_SKILL_UNCONDITIONAL = 0x000010,
	PC_PERM_JOIN_ALL_CHAT       = 0x000020,
	PC_PERM_NO_CHAT_KICK        = 0x000040,
	PC_PERM_HIDE_SESSION        = 0x000080,
	PC_PERM_WHO_DISPLAY_AID     = 0x000100,
	PC_PERM_RECEIVE_HACK_INFO   = 0x000200, // #10
	PC_PERM_WARP_ANYWHERE       = 0x000400,
	PC_PERM_VIEW_HPMETER        = 0x000800,
	PC_PERM_VIEW_EQUIPMENT      = 0x001000,
	PC_PERM_USE_CHECK           = 0x002000,
	PC_PERM_USE_CHANGEMAPTYPE   = 0x004000,
	PC_PERM_USE_ALL_COMMANDS    = 0x008000,
	PC_PERM_RECEIVE_REQUESTS    = 0x010000,
	PC_PERM_SHOW_BOSS           = 0x020000,
	PC_PERM_DISABLE_PVM         = 0x040000,
	PC_PERM_DISABLE_PVP         = 0x080000, // #20
	PC_PERM_DISABLE_CMD_DEAD    = 0x100000,
	PC_PERM_HCHSYS_ADMIN        = 0x200000,
};

/// Total number of PC permissions (without PC_PERM_NONE).
/// This is manifest constant for the size of pc_g_permission_name array,
/// so it's possible to apply sizeof to it [C-FAQ 1.24]
/// Whenever adding new permission: 1. add enum entry above, 2. add entry into
/// pc_g_permission_name (in pc.c), 3. increase NUM_PC_PERM below by 1.
#define NUM_PC_PERM 22

struct pc_permission_name_table {
	const char *name;
	enum e_pc_permission permission;
};

/// Name <-> enum table for PC permissions
extern const struct pc_permission_name_table pc_g_permission_name[NUM_PC_PERM];

typedef struct GroupSettings GroupSettings;

GroupSettings* pc_group_get_dummy_group(void);
bool pc_group_exists(int group_id);
GroupSettings* pc_group_id2group(int group_id);
bool pc_group_has_permission(GroupSettings *group, enum e_pc_permission permission);
bool pc_group_should_log_commands(GroupSettings *group);
const char* pc_group_get_name(GroupSettings *group);
int pc_group_get_level(GroupSettings *group);
int pc_group_get_idx(GroupSettings *group);

void do_init_pc_groups(void);
void do_final_pc_groups(void);
void pc_groups_reload(void);

#endif // _PC_GROUPS_H_
