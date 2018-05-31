/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2016  Hercules Dev Team
 * Copyright (C)  Athena Dev Teams
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
#define HERCULES_CORE

#include "pc_groups.h"

#include "map/atcommand.h" // atcommand-"exists(), atcommand-"load_groups()
#include "map/clif.h"      // clif-"GM_kick()
#include "map/map.h"       // mapiterator
#include "map/pc.h"        // pc-"set_group()
#include "common/cbasetypes.h"
#include "common/conf.h"
#include "common/db.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/strlib.h" // strcmp

static GroupSettings dummy_group; ///< dummy group used in dummy map sessions @see pc_get_dummy_sd()

struct pc_groups_interface pcg_s;
struct pc_groups_interface *pcg;

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
	return strdb_get(pcg->name_db, group_name);
}

/**
 * Loads group configuration from config file into memory.
 * @private
 */
static void read_config(void) {
	struct config_t pc_group_config;
	struct config_setting_t *groups = NULL;
	const char *config_filename = "conf/groups.conf"; // FIXME hardcoded name
	int group_count = 0;

	if (!libconfig->load_file(&pc_group_config, config_filename))
		return;

	groups = libconfig->lookup(&pc_group_config, "groups");

	if (groups != NULL) {
		GroupSettings *group_settings = NULL;
		struct DBIterator *iter = NULL;
		int i, loop = 0;

		group_count = libconfig->setting_length(groups);
		for (i = 0; i < group_count; ++i) {
			int id = 0, level = 0;
			const char *groupname = NULL;
			int log_commands = 0;
			struct config_setting_t *group = libconfig->setting_get_elem(groups, i);

			if (!libconfig->setting_lookup_int(group, "id", &id)) {
				ShowConfigWarning(group, "pc_groups:read_config: \"groups\" list member #%d has undefined id, removing...", i);
				libconfig->setting_remove_elem(groups, i);
				--i;
				--group_count;
				continue;
			}

			if (pcg->exists(id)) {
				ShowConfigWarning(group, "pc_groups:read_config: duplicate group id %d, removing...", i);
				libconfig->setting_remove_elem(groups, i);
				--i;
				--group_count;
				continue;
			}

			libconfig->setting_lookup_int(group, "level", &level);
			libconfig->setting_lookup_bool(group, "log_commands", &log_commands);

			if (!libconfig->setting_lookup_string(group, "name", &groupname)) {
				char temp[20];
				struct config_setting_t *name = NULL;
				snprintf(temp, sizeof(temp), "Group %d", id);
				if ((name = config_setting_add(group, "name", CONFIG_TYPE_STRING)) == NULL ||
				    !config_setting_set_string(name, temp)) {
					ShowError("pc_groups:read_config: failed to set missing group name, id=%d, skipping... (%s:%u)\n",
					          id, config_setting_source_file(group), config_setting_source_line(group));
					--i;
					--group_count;
					continue;
				}
				libconfig->setting_lookup_string(group, "name", &groupname); // Retrieve the pointer
			}

			if (name2group(groupname) != NULL) {
				ShowConfigWarning(group, "pc_groups:read_config: duplicate group name %s, removing...", groupname);
				libconfig->setting_remove_elem(groups, i);
				--i;
				--group_count;
				continue;
			}

			CREATE(group_settings, GroupSettings, 1);
			group_settings->id = id;
			group_settings->level = level;
			group_settings->name = aStrdup(groupname);
			group_settings->log_commands = (bool)log_commands;
			group_settings->inherit = libconfig->setting_get_member(group, "inherit");
			group_settings->commands = libconfig->setting_get_member(group, "commands");
			group_settings->permissions = libconfig->setting_get_member(group, "permissions");
			group_settings->inheritance_done = false;
			group_settings->root = group;
			group_settings->index = i;

			strdb_put(pcg->name_db, groupname, group_settings);
			idb_put(pcg->db, id, group_settings);
		}
		group_count = libconfig->setting_length(groups); // Save number of groups
		assert(group_count == db_size(pcg->db));

		// Check if all commands and permissions exist
		iter = db_iterator(pcg->db);
		for (group_settings = dbi_first(iter); dbi_exists(iter); group_settings = dbi_next(iter)) {
			struct config_setting_t *commands = group_settings->commands, *permissions = group_settings->permissions;
			int count = 0;

			// Make sure there is "commands" group
			if (commands == NULL)
				commands = group_settings->commands = libconfig->setting_add(group_settings->root, "commands", CONFIG_TYPE_GROUP);
			count = libconfig->setting_length(commands);

			for (i = 0; i < count; ++i) {
				struct config_setting_t *command = libconfig->setting_get_elem(commands, i);
				const char *name = config_setting_name(command);
				if (!atcommand->exists(name)) {
					ShowConfigWarning(command, "pc_groups:read_config: non-existent command name '%s', removing...", name);
					libconfig->setting_remove(commands, name);
					--i;
					--count;
				}
			}

			// Make sure there is "permissions" group
			if (permissions == NULL)
				permissions = group_settings->permissions = libconfig->setting_add(group_settings->root, "permissions", CONFIG_TYPE_GROUP);
			count = libconfig->setting_length(permissions);

			for(i = 0; i < count; ++i) {
				struct config_setting_t *permission = libconfig->setting_get_elem(permissions, i);
				const char *name = config_setting_name(permission);
				int j;

				ARR_FIND(0, pcg->permission_count, j, strcmp(pcg->permissions[j].name, name) == 0);
				if (j == pcg->permission_count) {
					ShowConfigWarning(permission, "pc_groups:read_config: non-existent permission name '%s', removing...", name);
					libconfig->setting_remove(permissions, name);
					--i;
					--count;
				}
			}
		}
		dbi_destroy(iter);

		// Apply inheritance
		i = 0; // counter for processed groups
		while (i < group_count) {
			iter = db_iterator(pcg->db);
			for (group_settings = dbi_first(iter); dbi_exists(iter); group_settings = dbi_next(iter)) {
				struct config_setting_t *inherit = NULL,
				                 *commands = group_settings->commands,
					             *permissions = group_settings->permissions;
				int j, inherit_count = 0, done = 0;

				if (group_settings->inheritance_done) // group already processed
					continue;

				if ((inherit = group_settings->inherit) == NULL ||
				    (inherit_count = libconfig->setting_length(inherit)) <= 0) { // this group does not inherit from others
					++i;
					group_settings->inheritance_done = true;
					continue;
				}

				for (j = 0; j < inherit_count; ++j) {
					GroupSettings *inherited_group = NULL;
					const char *groupname = libconfig->setting_get_string_elem(inherit, j);

					if (groupname == NULL) {
						ShowConfigWarning(inherit, "pc_groups:read_config: \"inherit\" array member #%d is not a name, removing...", j);
						libconfig->setting_remove_elem(inherit,j);
						continue;
					}
					if ((inherited_group = name2group(groupname)) == NULL) {
						ShowConfigWarning(inherit, "pc_groups:read_config: non-existent group name \"%s\", removing...", groupname);
						libconfig->setting_remove_elem(inherit,j);
						continue;
					}
					if (!inherited_group->inheritance_done)
						continue; // we need to do that group first

					// Copy settings (commands/permissions) that are not defined yet
					if (inherited_group->commands != NULL) {
						int k = 0, commands_count = libconfig->setting_length(inherited_group->commands);
						for (k = 0; k < commands_count; ++k)
							libconfig->setting_copy(commands, libconfig->setting_get_elem(inherited_group->commands, k));
					}

					if (inherited_group->permissions != NULL) {
						int k = 0, permissions_count = libconfig->setting_length(inherited_group->permissions);
						for (k = 0; k < permissions_count; ++k)
							libconfig->setting_copy(permissions, libconfig->setting_get_elem(inherited_group->permissions, k));
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
		iter = db_iterator(pcg->db);
		for (group_settings = dbi_first(iter); dbi_exists(iter); group_settings = dbi_next(iter)) {
			struct config_setting_t *permissions = group_settings->permissions;
			int count = libconfig->setting_length(permissions);

			for (i = 0; i < count; ++i) {
				struct config_setting_t *perm = libconfig->setting_get_elem(permissions, i);
				const char *name = config_setting_name(perm);
				int val = libconfig->setting_get_bool(perm);
				int j;

				if (val == 0) // does not have this permission
					continue;
				ARR_FIND(0, pcg->permission_count, j, strcmp(pcg->permissions[j].name, name) == 0);
				group_settings->e_permissions |= pcg->permissions[j].permission;
			}
		}
		dbi_destroy(iter);

		// Atcommand permissions are processed by atcommand module.
		// Fetch all groups and relevant config setting and send them
		// to atcommand->load_group() for processing.
		if (group_count > 0) {
			GroupSettings **pc_groups = NULL;
			struct config_setting_t **commands = NULL;
			CREATE(pc_groups, GroupSettings*, group_count);
			CREATE(commands, struct config_setting_t*, group_count);
			i = 0;
			iter = db_iterator(pcg->db);
			for (group_settings = dbi_first(iter); dbi_exists(iter); group_settings = dbi_next(iter)) {
				pc_groups[i] = group_settings;
				commands[i] = group_settings->commands;
				i++;
			}
			atcommand->load_groups(pc_groups, commands, group_count);
			dbi_destroy(iter);
			aFree(pc_groups);
			aFree(commands);
		}
	}

	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' groups in '"CL_WHITE"%s"CL_RESET"'.\n", group_count, config_filename);

	// All data is loaded now, discard config
	libconfig->destroy(&pc_group_config);
}

/**
 * Checks if player group has a permission
 * @param group group
 * @param permission permission to check
 */
bool pc_group_has_permission(GroupSettings *group, unsigned int permission)
{
	nullpo_retr(false, group);
	return ((group->e_permissions&permission) != 0);
}

/**
 * Checks if commands used by player group should be logged
 * @param group group
 */
bool pc_group_should_log_commands(GroupSettings *group)
{
	nullpo_retr(true, group);
	return group->log_commands;
}

/**
 * Checks if player group with given ID exists.
 * @param group_id group id
 * @returns true if group exists, false otherwise
 */
bool pc_group_exists(int group_id)
{
	return idb_exists(pcg->db, group_id);
}

/**
 * @retval NULL if not found
 */
GroupSettings* pc_group_id2group(int group_id)
{
	return idb_get(pcg->db, group_id);
}

/**
 * Group name lookup. Used only in @who atcommands.
 * @param group group
 * @return group name
 * @public
 */
const char* pc_group_get_name(GroupSettings *group)
{
	nullpo_retr(NULL, group);
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
	nullpo_ret(group);
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
	nullpo_ret(group);
	return group->index;
}

/**
 * Insert a new permission
 * @return inserted key or 0 upon failure.
 **/
unsigned int pc_groups_add_permission(const char *name) {
	uint64 key = 0x1;
	unsigned char i;
	nullpo_ret(name);

	for(i = 0; i < pcg->permission_count; i++) {
		if( strcmpi(name,pcg->permissions[i].name) == 0 ) {
			ShowError("pc_groups_add_permission(%s): failed! duplicate permission name!\n",name);
			return 0;
		}
	}

	if( i != 0 )
		key = (uint64)pcg->permissions[i - 1].permission << 1;

	if( key >= UINT_MAX ) {
		ShowError("pc_groups_add_permission(%s): failed! not enough room, too many permissions!\n",name);
		return 0;
	}

	i = pcg->permission_count;
	RECREATE(pcg->permissions, struct pc_groups_permission_table, ++pcg->permission_count);

	pcg->permissions[i].name = aStrdup(name);
	pcg->permissions[i].permission = (unsigned int)key;

	return (unsigned int)key;
}
/**
 * Initialize PC Groups: allocate DBMaps and read config.
 * @public
 */
void do_init_pc_groups(void) {
	const struct {
		const char *name;
		unsigned int permission;
	} pc_g_defaults[] = {
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
		{ "can_trade_bound", PC_PERM_TRADE_BOUND },
		{ "disable_pickup", PC_PERM_DISABLE_PICK_UP },
		{ "disable_store", PC_PERM_DISABLE_STORE },
		{ "disable_exp", PC_PERM_DISABLE_EXP },
		{ "disable_skill_usage", PC_PERM_DISABLE_SKILL_USAGE },
	};
	unsigned char i, len = ARRAYLENGTH(pc_g_defaults);

	for(i = 0; i < len; i++) {
		unsigned int p;
		if( ( p = pc_groups_add_permission(pc_g_defaults[i].name) ) != pc_g_defaults[i].permission )
			ShowError("do_init_pc_groups: %s error : %u != %u\n", pc_g_defaults[i].name, p, pc_g_defaults[i].permission);
	}

	/**
	 * Handle plugin-provided permissions
	 **/
	for(i = 0; i < pcg->HPMpermissions_count; i++) {
		*pcg->HPMpermissions[i].mask = pc_groups_add_permission(pcg->HPMpermissions[i].name);
	}

	pcg->db = idb_alloc(DB_OPT_RELEASE_DATA);
	pcg->name_db = stridb_alloc(DB_OPT_DUP_KEY, 0);

	read_config();
}

/**
 * @see DBApply
 */
static int group_db_clear_sub(union DBKey key, struct DBData *data, va_list args)
{
	GroupSettings *group = DB->data2ptr(data);
	nullpo_ret(group);
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
	if (pcg->db != NULL)
		pcg->db->destroy(pcg->db, group_db_clear_sub);
	if (pcg->name_db != NULL)
		db_destroy(pcg->name_db);

	if(pcg->permissions != NULL) {
		unsigned char i;
		for(i = 0; i < pcg->permission_count; i++)
			aFree(pcg->permissions[i].name);
		aFree(pcg->permissions);
		pcg->permissions = NULL;
	}
	pcg->permission_count = 0;
}

/**
 * Reload PC Groups
 * Used in @reloadatcommand
 * @public
 */
void pc_groups_reload(void) {
	struct map_session_data *sd = NULL;
	struct s_mapiterator *iter;

	pcg->final();
	pcg->init();

	/* refresh online users permissions */
	iter = mapit_getallusers();
	for (sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); sd = BL_UCAST(BL_PC, mapit->next(iter))) {
		if (pc->set_group(sd, sd->group_id) != 0) {
			ShowWarning("pc_groups_reload: %s (AID:%d) has unknown group id (%d)! kicking...\n",
				sd->status.name, sd->status.account_id, pc_get_group_id(sd));
			clif->GM_kick(NULL, sd);
		}
	}
	mapit->free(iter);
}

/**
 * Connect Interface
 **/
void pc_groups_defaults(void) {
	pcg = &pcg_s;
	/* */
	pcg->db = NULL;
	pcg->name_db = NULL;
	/* */
	pcg->permissions = NULL;
	pcg->permission_count = 0;
	/* */
	pcg->HPMpermissions = NULL;
	pcg->HPMpermissions_count = 0;
	/* */
	pcg->init = do_init_pc_groups;
	pcg->final = do_final_pc_groups;
	pcg->reload = pc_groups_reload;
	/* */
	pcg->get_dummy_group = pc_group_get_dummy_group;
	pcg->exists = pc_group_exists;
	pcg->id2group = pc_group_id2group;
	pcg->has_permission = pc_group_has_permission;
	pcg->should_log_commands = pc_group_should_log_commands;
	pcg->get_name = pc_group_get_name;
	pcg->get_level = pc_group_get_level;
	pcg->get_idx = pc_group_get_idx;
}
