// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#include "../common/cbasetypes.h"
#include "../common/conf.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/nullpo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h" // strcmp

#include "pc_groups.h"
#include "atcommand.h" // atcommand->exists(), atcommand->load_groups()
#include "clif.h"      // clif->GM_kick()
#include "map.h"       // mapiterator
#include "pc.h"        // pc->set_group()

// Cached config settings for quick lookup
struct GroupSettings {
	unsigned int id; // groups.[].id
	int level; // groups.[].level
	char *name; // copy of groups.[].name
	unsigned int e_permissions; // packed groups.[].permissions
	bool log_commands; // groups.[].log_commands
	int index; // internal index of the group (contiguous range starting at 0) [Ind]
	/// Following are used/available only during config reading
	config_setting_t *commands; // groups.[].commands
	config_setting_t *permissions; // groups.[].permissions
	config_setting_t *inherit; // groups.[].inherit
	bool inheritance_done; // have all inheritance rules been evaluated?
	config_setting_t *root; // groups.[]
};

const struct pc_permission_name_table pc_g_permission_name[NUM_PC_PERM] = {
	{ "can_trade", PC_PERM_TRADE },
	{ "can_party", PC_PERM_PARTY },
	{ "all_skill", PC_PERM_ALL_SKILL },
	{ "all_equipment", PC_PERM_USE_ALL_EQUIPMENT },
	{ "skill_unconditional", PC_PERM_SKILL_UNCONDITIONAL },
	{ "join_chat", PC_PERM_JOIN_ALL_CHAT },
	{ "kick_chat", PC_PERM_NO_CHAT_KICK },
	{ "hide_session", PC_PERM_HIDE_SESSION },
	{ "who_display_aid", PC_PERM_WHO_DISPLAY_AID },
	{ "hack_info", PC_PERM_RECEIVE_HACK_INFO },
	{ "any_warp", PC_PERM_WARP_ANYWHERE },
	{ "view_hpmeter", PC_PERM_VIEW_HPMETER },
	{ "view_equipment", PC_PERM_VIEW_EQUIPMENT },
	{ "use_check", PC_PERM_USE_CHECK },
	{ "use_changemaptype", PC_PERM_USE_CHANGEMAPTYPE },
	{ "all_commands", PC_PERM_USE_ALL_COMMANDS },
	{ "receive_requests", PC_PERM_RECEIVE_REQUESTS },
	{ "show_bossmobs", PC_PERM_SHOW_BOSS },
	{ "disable_pvm", PC_PERM_DISABLE_PVM },
	{ "disable_pvp", PC_PERM_DISABLE_PVP },
	{ "disable_commands_when_dead", PC_PERM_DISABLE_CMD_DEAD },
	{ "hchsys_admin", PC_PERM_HCHSYS_ADMIN },
};

static DBMap* pc_group_db; // id -> GroupSettings
static DBMap* pc_groupname_db; // name -> GroupSettings

static GroupSettings dummy_group; ///< dummy group used in dummy map sessions @see pc_get_dummy_sd()

/**
 * Returns dummy group.
 * Used in dummy map sessions.
 * @see pc_get_dummy_sd()
 */
GroupSettings* pc_group_get_dummy_group(void)
{
	return &dummy_group;
}

/**
 * @retval NULL if not found
 * @private
 */
static inline GroupSettings* name2group(const char* group_name)
{
	return strdb_get(pc_groupname_db, group_name);
}

/**
 * Loads group configuration from config file into memory.
 * @private
 */
static void read_config(void) {
	config_t pc_group_config;
	config_setting_t *groups = NULL;
	const char *config_filename = "conf/groups.conf"; // FIXME hardcoded name
	int group_count = 0;
	
	if (conf_read_file(&pc_group_config, config_filename))
		return;

	groups = config_lookup(&pc_group_config, "groups");

	if (groups != NULL) {
		GroupSettings *group_settings = NULL;
		DBIterator *iter = NULL;
		int i, loop = 0;

		group_count = config_setting_length(groups);
		for (i = 0; i < group_count; ++i) {
			int id = 0, level = 0;
			const char *groupname = NULL;
			int log_commands = 0;
			config_setting_t *group = config_setting_get_elem(groups, i);

			if (!config_setting_lookup_int(group, "id", &id)) {
				ShowConfigWarning(group, "pc_groups:read_config: \"groups\" list member #%d has undefined id, removing...", i);
				config_setting_remove_elem(groups, i);
				--i;
				--group_count;
				continue;
			}

			if (pc_group_exists(id)) {
				ShowConfigWarning(group, "pc_groups:read_config: duplicate group id %d, removing...", i);
				config_setting_remove_elem(groups, i);
				--i;
				--group_count;
				continue;
			}

			config_setting_lookup_int(group, "level", &level);
			config_setting_lookup_bool(group, "log_commands", &log_commands);

			if (!config_setting_lookup_string(group, "name", &groupname)) {
				char temp[20];
				config_setting_t *name = NULL;
				snprintf(temp, sizeof(temp), "Group %d", id);
				if ((name = config_setting_add(group, "name", CONFIG_TYPE_STRING)) == NULL ||
				    !config_setting_set_string(name, temp)) {
					ShowError("pc_groups:read_config: failed to set missing group name, id=%d, skipping... (%s:%d)\n",
					          id, config_setting_source_file(group), config_setting_source_line(group));
					--i;
					--group_count;
					continue;
				}
				config_setting_lookup_string(group, "name", &groupname); // Retrieve the pointer
			}

			if (name2group(groupname) != NULL) {
				ShowConfigWarning(group, "pc_groups:read_config: duplicate group name %s, removing...", groupname);
				config_setting_remove_elem(groups, i);
				--i;
				--group_count;
				continue;
			}

			CREATE(group_settings, GroupSettings, 1);
			group_settings->id = id;
			group_settings->level = level;
			group_settings->name = aStrdup(groupname);
			group_settings->log_commands = (bool)log_commands;
			group_settings->inherit = config_setting_get_member(group, "inherit");
			group_settings->commands = config_setting_get_member(group, "commands");
			group_settings->permissions = config_setting_get_member(group, "permissions");
			group_settings->inheritance_done = false;
			group_settings->root = group;
			group_settings->index = i;

			strdb_put(pc_groupname_db, groupname, group_settings);
			idb_put(pc_group_db, id, group_settings);
			
		}
		group_count = config_setting_length(groups); // Save number of groups
		assert(group_count == db_size(pc_group_db));
		
		// Check if all commands and permissions exist
		iter = db_iterator(pc_group_db);
		for (group_settings = dbi_first(iter); dbi_exists(iter); group_settings = dbi_next(iter)) {
			config_setting_t *commands = group_settings->commands, *permissions = group_settings->permissions;
			int count = 0, i;

			// Make sure there is "commands" group
			if (commands == NULL)
				commands = group_settings->commands = config_setting_add(group_settings->root, "commands", CONFIG_TYPE_GROUP);
			count = config_setting_length(commands);

			for (i = 0; i < count; ++i) {
				config_setting_t *command = config_setting_get_elem(commands, i);
				const char *name = config_setting_name(command);
				if (!atcommand->exists(name)) {
					ShowConfigWarning(command, "pc_groups:read_config: non-existent command name '%s', removing...", name);
					config_setting_remove(commands, name);
					--i;
					--count;
				}
			}

			// Make sure there is "permissions" group
			if (permissions == NULL)
				permissions = group_settings->permissions = config_setting_add(group_settings->root, "permissions", CONFIG_TYPE_GROUP);
			count = config_setting_length(permissions);

			for(i = 0; i < count; ++i) {
				config_setting_t *permission = config_setting_get_elem(permissions, i);
				const char *name = config_setting_name(permission);
				int j;

				ARR_FIND(0, ARRAYLENGTH(pc_g_permission_name), j, strcmp(pc_g_permission_name[j].name, name) == 0);
				if (j == ARRAYLENGTH(pc_g_permission_name)) {
					ShowConfigWarning(permission, "pc_groups:read_config: non-existent permission name '%s', removing...", name);
					config_setting_remove(permissions, name);
					--i;
					--count;
				}
			}
		}
		dbi_destroy(iter);

		// Apply inheritance
		i = 0; // counter for processed groups
		while (i < group_count) {
			iter = db_iterator(pc_group_db);
			for (group_settings = dbi_first(iter); dbi_exists(iter); group_settings = dbi_next(iter)) {
				config_setting_t *inherit = NULL,
				                 *commands = group_settings->commands,
					             *permissions = group_settings->permissions;
				int j, inherit_count = 0, done = 0;
				
				if (group_settings->inheritance_done) // group already processed
					continue; 

				if ((inherit = group_settings->inherit) == NULL ||
				    (inherit_count = config_setting_length(inherit)) <= 0) { // this group does not inherit from others
					++i;
					group_settings->inheritance_done = true;
					continue;
				}
				
				for (j = 0; j < inherit_count; ++j) {
					GroupSettings *inherited_group = NULL;
					const char *groupname = config_setting_get_string_elem(inherit, j);

					if (groupname == NULL) {
						ShowConfigWarning(inherit, "pc_groups:read_config: \"inherit\" array member #%d is not a name, removing...", j);
						config_setting_remove_elem(inherit,j);
						continue;
					}
					if ((inherited_group = name2group(groupname)) == NULL) {
						ShowConfigWarning(inherit, "pc_groups:read_config: non-existent group name \"%s\", removing...", groupname);
						config_setting_remove_elem(inherit,j);
						continue;
					}
					if (!inherited_group->inheritance_done)
						continue; // we need to do that group first

					// Copy settings (commands/permissions) that are not defined yet
					if (inherited_group->commands != NULL) {
						int i = 0, commands_count = config_setting_length(inherited_group->commands);
						for (i = 0; i < commands_count; ++i)
							config_setting_copy(commands, config_setting_get_elem(inherited_group->commands, i));
					}

					if (inherited_group->permissions != NULL) {
						int i = 0, permissions_count = config_setting_length(inherited_group->permissions);
						for (i = 0; i < permissions_count; ++i)
							config_setting_copy(permissions, config_setting_get_elem(inherited_group->permissions, i));
					}

					++done; // copied commands and permissions from one of inherited groups
				}
				
				if (done == inherit_count) { // copied commands from all of inherited groups
					++i;
					group_settings->inheritance_done = true; // we're done with this group
				}
			}
			dbi_destroy(iter);

			if (++loop > group_count) {
				ShowWarning("pc_groups:read_config: Could not process inheritance rules, check your config '%s' for cycles...\n",
				            config_filename);
				break;
			}
		} // while(i < group_count)
		
		// Pack permissions into GroupSettings.e_permissions for faster checking
		iter = db_iterator(pc_group_db);
		for (group_settings = dbi_first(iter); dbi_exists(iter); group_settings = dbi_next(iter)) {
			config_setting_t *permissions = group_settings->permissions;
			int i, count = config_setting_length(permissions);

			for (i = 0; i < count; ++i) {
				config_setting_t *perm = config_setting_get_elem(permissions, i);
				const char *name = config_setting_name(perm);
				int val = config_setting_get_bool(perm);
				int j;

				if (val == 0) // does not have this permission
					continue;
				ARR_FIND(0, ARRAYLENGTH(pc_g_permission_name), j, strcmp(pc_g_permission_name[j].name, name) == 0);
				group_settings->e_permissions |= pc_g_permission_name[j].permission;
			}
		}
		dbi_destroy(iter);

		// Atcommand permissions are processed by atcommand module.
		// Fetch all groups and relevant config setting and send them
		// to atcommand->load_group() for processing.
		if (group_count > 0) {
			int i = 0;
			GroupSettings **groups = NULL;
			config_setting_t **commands = NULL;
			CREATE(groups, GroupSettings*, group_count);
			CREATE(commands, config_setting_t*, group_count);
			iter = db_iterator(pc_group_db);
			for (group_settings = dbi_first(iter); dbi_exists(iter); group_settings = dbi_next(iter)) {
				groups[i] = group_settings;
				commands[i] = group_settings->commands;
				i++;
			}
			atcommand->load_groups(groups, commands, group_count);
			dbi_destroy(iter);
			aFree(groups);
			aFree(commands);
		}
	}

	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' groups in '"CL_WHITE"%s"CL_RESET"'.\n", group_count, config_filename);

	// All data is loaded now, discard config
	config_destroy(&pc_group_config);
}

/**
 * Checks if player group has a permission
 * @param group group
 * @param permission permission to check
 */
bool pc_group_has_permission(GroupSettings *group, enum e_pc_permission permission)
{
	return ((group->e_permissions&permission) != 0);
}

/**
 * Checks if commands used by player group should be logged
 * @param group group
 */
bool pc_group_should_log_commands(GroupSettings *group)
{
	return group->log_commands;
}

/**
 * Checks if player group with given ID exists.
 * @param group_id group id
 * @returns true if group exists, false otherwise
 */
bool pc_group_exists(int group_id)
{
	return idb_exists(pc_group_db, group_id);
}

/**
 * @retval NULL if not found
 */
GroupSettings* pc_group_id2group(int group_id)
{
	return idb_get(pc_group_db, group_id);
}

/**
 * Group name lookup. Used only in @who atcommands.
 * @param group group
 * @return group name
 * @public
 */
const char* pc_group_get_name(GroupSettings *group)
{
	return group->name;
}

/**
 * Group level lookup. A way to provide backward compatibility with GM level system.
 * @param group group
 * @return group level
 * @public
 */
int pc_group_get_level(GroupSettings *group)
{
	return group->level;
}

/**
 * Group -> index lookup.
 * @param group group
 * @return group index
 * @public
 */
int pc_group_get_idx(GroupSettings *group)
{
	return group->index;
}

/**
 * Initialize PC Groups: allocate DBMaps and read config.
 * @public
 */
void do_init_pc_groups(void)
{
	pc_group_db = idb_alloc(DB_OPT_RELEASE_DATA);
	pc_groupname_db = stridb_alloc(DB_OPT_DUP_KEY, 0);
	read_config();
}

/**
 * @see DBApply
 */
static int group_db_clear_sub(DBKey key, DBData *data, va_list args)
{
	GroupSettings *group = DB->data2ptr(data);
	if (group->name)
		aFree(group->name);
	return 0;
}

/**
 * Finalize PC Groups: free DBMaps and config.
 * @public
 */
void do_final_pc_groups(void)
{
	if (pc_group_db != NULL)
		pc_group_db->destroy(pc_group_db, group_db_clear_sub);
	if (pc_groupname_db != NULL)
		db_destroy(pc_groupname_db);
}

/**
 * Reload PC Groups
 * Used in @reloadatcommand
 * @public
 */
void pc_groups_reload(void) {
	struct map_session_data *sd = NULL;
	struct s_mapiterator *iter;

	do_final_pc_groups();
	do_init_pc_groups();
	
	/* refresh online users permissions */
	iter = mapit_getallusers();
	for (sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); sd = (TBL_PC*)mapit->next(iter)) {
		if (pc->set_group(sd, sd->group_id) != 0) {
			ShowWarning("pc_groups_reload: %s (AID:%d) has unknown group id (%d)! kicking...\n",
				sd->status.name, sd->status.account_id, pc_get_group_id(sd));
			clif->GM_kick(NULL, sd);
		}
	}
	mapit->free(iter);
}
