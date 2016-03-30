/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-2016  Hercules Dev Team
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

/*
 * NOTE: This file was auto-generated and should never be manually edited,
 *       as it will get overwritten.
 */

/* HCache_interface */
void HP_HCache_init(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_HCache_init_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_HCache_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_HCache_init_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.HCache.init();
	}
	if( HPMHooks.count.HP_HCache_init_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_HCache_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_HCache_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
bool HP_HCache_check(const char *file) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_HCache_check_pre ) {
		bool (*preHookFunc) (const char *file);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_HCache_check_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_HCache_check_pre[hIndex].func;
			retVal___ = preHookFunc(file);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.HCache.check(file);
	}
	if( HPMHooks.count.HP_HCache_check_post ) {
		bool (*postHookFunc) (bool retVal___, const char *file);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_HCache_check_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_HCache_check_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, file);
		}
	}
	return retVal___;
}
FILE* HP_HCache_open(const char *file, const char *opt) {
	int hIndex = 0;
	FILE* retVal___ = NULL;
	if( HPMHooks.count.HP_HCache_open_pre ) {
		FILE* (*preHookFunc) (const char *file, const char *opt);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_HCache_open_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_HCache_open_pre[hIndex].func;
			retVal___ = preHookFunc(file, opt);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.HCache.open(file, opt);
	}
	if( HPMHooks.count.HP_HCache_open_post ) {
		FILE* (*postHookFunc) (FILE* retVal___, const char *file, const char *opt);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_HCache_open_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_HCache_open_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, file, opt);
		}
	}
	return retVal___;
}
/* cmdline_interface */
void HP_cmdline_init(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_cmdline_init_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_cmdline_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_cmdline_init_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.cmdline.init();
	}
	if( HPMHooks.count.HP_cmdline_init_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_cmdline_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_cmdline_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_cmdline_final(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_cmdline_final_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_cmdline_final_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_cmdline_final_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.cmdline.final();
	}
	if( HPMHooks.count.HP_cmdline_final_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_cmdline_final_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_cmdline_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
bool HP_cmdline_arg_add(unsigned int pluginID, const char *name, char shortname, CmdlineExecFunc func, const char *help, unsigned int options) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_cmdline_arg_add_pre ) {
		bool (*preHookFunc) (unsigned int *pluginID, const char *name, char *shortname, CmdlineExecFunc *func, const char *help, unsigned int *options);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_cmdline_arg_add_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_cmdline_arg_add_pre[hIndex].func;
			retVal___ = preHookFunc(&pluginID, name, &shortname, &func, help, &options);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.cmdline.arg_add(pluginID, name, shortname, func, help, options);
	}
	if( HPMHooks.count.HP_cmdline_arg_add_post ) {
		bool (*postHookFunc) (bool retVal___, unsigned int *pluginID, const char *name, char *shortname, CmdlineExecFunc *func, const char *help, unsigned int *options);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_cmdline_arg_add_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_cmdline_arg_add_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &pluginID, name, &shortname, &func, help, &options);
		}
	}
	return retVal___;
}
int HP_cmdline_exec(int argc, char **argv, unsigned int options) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_cmdline_exec_pre ) {
		int (*preHookFunc) (int *argc, char **argv, unsigned int *options);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_cmdline_exec_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_cmdline_exec_pre[hIndex].func;
			retVal___ = preHookFunc(&argc, argv, &options);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.cmdline.exec(argc, argv, options);
	}
	if( HPMHooks.count.HP_cmdline_exec_post ) {
		int (*postHookFunc) (int retVal___, int *argc, char **argv, unsigned int *options);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_cmdline_exec_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_cmdline_exec_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &argc, argv, &options);
		}
	}
	return retVal___;
}
bool HP_cmdline_arg_next_value(const char *name, int current_arg, int argc) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_cmdline_arg_next_value_pre ) {
		bool (*preHookFunc) (const char *name, int *current_arg, int *argc);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_cmdline_arg_next_value_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_cmdline_arg_next_value_pre[hIndex].func;
			retVal___ = preHookFunc(name, &current_arg, &argc);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.cmdline.arg_next_value(name, current_arg, argc);
	}
	if( HPMHooks.count.HP_cmdline_arg_next_value_post ) {
		bool (*postHookFunc) (bool retVal___, const char *name, int *current_arg, int *argc);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_cmdline_arg_next_value_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_cmdline_arg_next_value_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, name, &current_arg, &argc);
		}
	}
	return retVal___;
}
const char* HP_cmdline_arg_source(struct CmdlineArgData *arg) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if( HPMHooks.count.HP_cmdline_arg_source_pre ) {
		const char* (*preHookFunc) (struct CmdlineArgData *arg);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_cmdline_arg_source_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_cmdline_arg_source_pre[hIndex].func;
			retVal___ = preHookFunc(arg);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.cmdline.arg_source(arg);
	}
	if( HPMHooks.count.HP_cmdline_arg_source_post ) {
		const char* (*postHookFunc) (const char* retVal___, struct CmdlineArgData *arg);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_cmdline_arg_source_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_cmdline_arg_source_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, arg);
		}
	}
	return retVal___;
}
/* console_interface */
void HP_console_init(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_console_init_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_console_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_console_init_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.console.init();
	}
	if( HPMHooks.count.HP_console_init_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_console_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_console_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_console_final(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_console_final_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_console_final_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_console_final_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.console.final();
	}
	if( HPMHooks.count.HP_console_final_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_console_final_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_console_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_console_display_title(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_console_display_title_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_console_display_title_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_console_display_title_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.console.display_title();
	}
	if( HPMHooks.count.HP_console_display_title_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_console_display_title_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_console_display_title_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_console_display_gplnotice(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_console_display_gplnotice_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_console_display_gplnotice_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_console_display_gplnotice_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.console.display_gplnotice();
	}
	if( HPMHooks.count.HP_console_display_gplnotice_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_console_display_gplnotice_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_console_display_gplnotice_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
/* core_interface */
void HP_core_shutdown_callback(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_core_shutdown_callback_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_core_shutdown_callback_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_core_shutdown_callback_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.core.shutdown_callback();
	}
	if( HPMHooks.count.HP_core_shutdown_callback_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_core_shutdown_callback_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_core_shutdown_callback_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
/* db_interface */
enum DBOptions HP_DB_fix_options(enum DBType type, enum DBOptions options) {
	int hIndex = 0;
	enum DBOptions retVal___ = DB_OPT_BASE;
	if( HPMHooks.count.HP_DB_fix_options_pre ) {
		enum DBOptions (*preHookFunc) (enum DBType *type, enum DBOptions *options);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_fix_options_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_DB_fix_options_pre[hIndex].func;
			retVal___ = preHookFunc(&type, &options);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.fix_options(type, options);
	}
	if( HPMHooks.count.HP_DB_fix_options_post ) {
		enum DBOptions (*postHookFunc) (enum DBOptions retVal___, enum DBType *type, enum DBOptions *options);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_fix_options_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_DB_fix_options_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &type, &options);
		}
	}
	return retVal___;
}
DBComparator HP_DB_default_cmp(enum DBType type) {
	int hIndex = 0;
	DBComparator retVal___ = NULL;
	if( HPMHooks.count.HP_DB_default_cmp_pre ) {
		DBComparator (*preHookFunc) (enum DBType *type);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_default_cmp_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_DB_default_cmp_pre[hIndex].func;
			retVal___ = preHookFunc(&type);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.default_cmp(type);
	}
	if( HPMHooks.count.HP_DB_default_cmp_post ) {
		DBComparator (*postHookFunc) (DBComparator retVal___, enum DBType *type);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_default_cmp_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_DB_default_cmp_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &type);
		}
	}
	return retVal___;
}
DBHasher HP_DB_default_hash(enum DBType type) {
	int hIndex = 0;
	DBHasher retVal___ = NULL;
	if( HPMHooks.count.HP_DB_default_hash_pre ) {
		DBHasher (*preHookFunc) (enum DBType *type);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_default_hash_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_DB_default_hash_pre[hIndex].func;
			retVal___ = preHookFunc(&type);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.default_hash(type);
	}
	if( HPMHooks.count.HP_DB_default_hash_post ) {
		DBHasher (*postHookFunc) (DBHasher retVal___, enum DBType *type);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_default_hash_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_DB_default_hash_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &type);
		}
	}
	return retVal___;
}
DBReleaser HP_DB_default_release(enum DBType type, enum DBOptions options) {
	int hIndex = 0;
	DBReleaser retVal___ = NULL;
	if( HPMHooks.count.HP_DB_default_release_pre ) {
		DBReleaser (*preHookFunc) (enum DBType *type, enum DBOptions *options);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_default_release_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_DB_default_release_pre[hIndex].func;
			retVal___ = preHookFunc(&type, &options);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.default_release(type, options);
	}
	if( HPMHooks.count.HP_DB_default_release_post ) {
		DBReleaser (*postHookFunc) (DBReleaser retVal___, enum DBType *type, enum DBOptions *options);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_default_release_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_DB_default_release_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &type, &options);
		}
	}
	return retVal___;
}
DBReleaser HP_DB_custom_release(enum DBReleaseOption which) {
	int hIndex = 0;
	DBReleaser retVal___ = NULL;
	if( HPMHooks.count.HP_DB_custom_release_pre ) {
		DBReleaser (*preHookFunc) (enum DBReleaseOption *which);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_custom_release_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_DB_custom_release_pre[hIndex].func;
			retVal___ = preHookFunc(&which);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.custom_release(which);
	}
	if( HPMHooks.count.HP_DB_custom_release_post ) {
		DBReleaser (*postHookFunc) (DBReleaser retVal___, enum DBReleaseOption *which);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_custom_release_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_DB_custom_release_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &which);
		}
	}
	return retVal___;
}
struct DBMap* HP_DB_alloc(const char *file, const char *func, int line, enum DBType type, enum DBOptions options, unsigned short maxlen) {
	int hIndex = 0;
	struct DBMap* retVal___ = NULL;
	if( HPMHooks.count.HP_DB_alloc_pre ) {
		struct DBMap* (*preHookFunc) (const char *file, const char *func, int *line, enum DBType *type, enum DBOptions *options, unsigned short *maxlen);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_alloc_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_DB_alloc_pre[hIndex].func;
			retVal___ = preHookFunc(file, func, &line, &type, &options, &maxlen);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.alloc(file, func, line, type, options, maxlen);
	}
	if( HPMHooks.count.HP_DB_alloc_post ) {
		struct DBMap* (*postHookFunc) (struct DBMap* retVal___, const char *file, const char *func, int *line, enum DBType *type, enum DBOptions *options, unsigned short *maxlen);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_alloc_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_DB_alloc_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, file, func, &line, &type, &options, &maxlen);
		}
	}
	return retVal___;
}
union DBKey HP_DB_i2key(int key) {
	int hIndex = 0;
	union DBKey retVal___;
	memset(&retVal___, '\0', sizeof(union DBKey));
	if( HPMHooks.count.HP_DB_i2key_pre ) {
		union DBKey (*preHookFunc) (int *key);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_i2key_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_DB_i2key_pre[hIndex].func;
			retVal___ = preHookFunc(&key);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.i2key(key);
	}
	if( HPMHooks.count.HP_DB_i2key_post ) {
		union DBKey (*postHookFunc) (union DBKey retVal___, int *key);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_i2key_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_DB_i2key_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &key);
		}
	}
	return retVal___;
}
union DBKey HP_DB_ui2key(unsigned int key) {
	int hIndex = 0;
	union DBKey retVal___;
	memset(&retVal___, '\0', sizeof(union DBKey));
	if( HPMHooks.count.HP_DB_ui2key_pre ) {
		union DBKey (*preHookFunc) (unsigned int *key);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_ui2key_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_DB_ui2key_pre[hIndex].func;
			retVal___ = preHookFunc(&key);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.ui2key(key);
	}
	if( HPMHooks.count.HP_DB_ui2key_post ) {
		union DBKey (*postHookFunc) (union DBKey retVal___, unsigned int *key);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_ui2key_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_DB_ui2key_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &key);
		}
	}
	return retVal___;
}
union DBKey HP_DB_str2key(const char *key) {
	int hIndex = 0;
	union DBKey retVal___;
	memset(&retVal___, '\0', sizeof(union DBKey));
	if( HPMHooks.count.HP_DB_str2key_pre ) {
		union DBKey (*preHookFunc) (const char *key);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_str2key_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_DB_str2key_pre[hIndex].func;
			retVal___ = preHookFunc(key);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.str2key(key);
	}
	if( HPMHooks.count.HP_DB_str2key_post ) {
		union DBKey (*postHookFunc) (union DBKey retVal___, const char *key);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_str2key_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_DB_str2key_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, key);
		}
	}
	return retVal___;
}
union DBKey HP_DB_i642key(int64 key) {
	int hIndex = 0;
	union DBKey retVal___;
	memset(&retVal___, '\0', sizeof(union DBKey));
	if( HPMHooks.count.HP_DB_i642key_pre ) {
		union DBKey (*preHookFunc) (int64 *key);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_i642key_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_DB_i642key_pre[hIndex].func;
			retVal___ = preHookFunc(&key);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.i642key(key);
	}
	if( HPMHooks.count.HP_DB_i642key_post ) {
		union DBKey (*postHookFunc) (union DBKey retVal___, int64 *key);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_i642key_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_DB_i642key_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &key);
		}
	}
	return retVal___;
}
union DBKey HP_DB_ui642key(uint64 key) {
	int hIndex = 0;
	union DBKey retVal___;
	memset(&retVal___, '\0', sizeof(union DBKey));
	if( HPMHooks.count.HP_DB_ui642key_pre ) {
		union DBKey (*preHookFunc) (uint64 *key);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_ui642key_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_DB_ui642key_pre[hIndex].func;
			retVal___ = preHookFunc(&key);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.ui642key(key);
	}
	if( HPMHooks.count.HP_DB_ui642key_post ) {
		union DBKey (*postHookFunc) (union DBKey retVal___, uint64 *key);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_ui642key_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_DB_ui642key_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &key);
		}
	}
	return retVal___;
}
struct DBData HP_DB_i2data(int data) {
	int hIndex = 0;
	struct DBData retVal___;
	memset(&retVal___, '\0', sizeof(struct DBData));
	if( HPMHooks.count.HP_DB_i2data_pre ) {
		struct DBData (*preHookFunc) (int *data);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_i2data_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_DB_i2data_pre[hIndex].func;
			retVal___ = preHookFunc(&data);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.i2data(data);
	}
	if( HPMHooks.count.HP_DB_i2data_post ) {
		struct DBData (*postHookFunc) (struct DBData retVal___, int *data);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_i2data_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_DB_i2data_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &data);
		}
	}
	return retVal___;
}
struct DBData HP_DB_ui2data(unsigned int data) {
	int hIndex = 0;
	struct DBData retVal___;
	memset(&retVal___, '\0', sizeof(struct DBData));
	if( HPMHooks.count.HP_DB_ui2data_pre ) {
		struct DBData (*preHookFunc) (unsigned int *data);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_ui2data_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_DB_ui2data_pre[hIndex].func;
			retVal___ = preHookFunc(&data);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.ui2data(data);
	}
	if( HPMHooks.count.HP_DB_ui2data_post ) {
		struct DBData (*postHookFunc) (struct DBData retVal___, unsigned int *data);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_ui2data_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_DB_ui2data_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &data);
		}
	}
	return retVal___;
}
struct DBData HP_DB_ptr2data(void *data) {
	int hIndex = 0;
	struct DBData retVal___;
	memset(&retVal___, '\0', sizeof(struct DBData));
	if( HPMHooks.count.HP_DB_ptr2data_pre ) {
		struct DBData (*preHookFunc) (void *data);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_ptr2data_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_DB_ptr2data_pre[hIndex].func;
			retVal___ = preHookFunc(data);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.ptr2data(data);
	}
	if( HPMHooks.count.HP_DB_ptr2data_post ) {
		struct DBData (*postHookFunc) (struct DBData retVal___, void *data);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_ptr2data_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_DB_ptr2data_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, data);
		}
	}
	return retVal___;
}
int HP_DB_data2i(struct DBData *data) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_DB_data2i_pre ) {
		int (*preHookFunc) (struct DBData *data);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_data2i_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_DB_data2i_pre[hIndex].func;
			retVal___ = preHookFunc(data);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.data2i(data);
	}
	if( HPMHooks.count.HP_DB_data2i_post ) {
		int (*postHookFunc) (int retVal___, struct DBData *data);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_data2i_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_DB_data2i_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, data);
		}
	}
	return retVal___;
}
unsigned int HP_DB_data2ui(struct DBData *data) {
	int hIndex = 0;
	unsigned int retVal___ = 0;
	if( HPMHooks.count.HP_DB_data2ui_pre ) {
		unsigned int (*preHookFunc) (struct DBData *data);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_data2ui_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_DB_data2ui_pre[hIndex].func;
			retVal___ = preHookFunc(data);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.data2ui(data);
	}
	if( HPMHooks.count.HP_DB_data2ui_post ) {
		unsigned int (*postHookFunc) (unsigned int retVal___, struct DBData *data);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_data2ui_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_DB_data2ui_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, data);
		}
	}
	return retVal___;
}
void* HP_DB_data2ptr(struct DBData *data) {
	int hIndex = 0;
	void* retVal___ = NULL;
	if( HPMHooks.count.HP_DB_data2ptr_pre ) {
		void* (*preHookFunc) (struct DBData *data);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_data2ptr_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_DB_data2ptr_pre[hIndex].func;
			retVal___ = preHookFunc(data);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.data2ptr(data);
	}
	if( HPMHooks.count.HP_DB_data2ptr_post ) {
		void* (*postHookFunc) (void* retVal___, struct DBData *data);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_data2ptr_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_DB_data2ptr_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, data);
		}
	}
	return retVal___;
}
void HP_DB_init(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_DB_init_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_DB_init_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.DB.init();
	}
	if( HPMHooks.count.HP_DB_init_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_DB_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_DB_final(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_DB_final_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_final_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_DB_final_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.DB.final();
	}
	if( HPMHooks.count.HP_DB_final_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_DB_final_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_DB_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
/* libconfig_interface */
int HP_libconfig_read(struct config_t *config, FILE *stream) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_read_pre ) {
		int (*preHookFunc) (struct config_t *config, FILE *stream);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_read_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_read_pre[hIndex].func;
			retVal___ = preHookFunc(config, stream);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.read(config, stream);
	}
	if( HPMHooks.count.HP_libconfig_read_post ) {
		int (*postHookFunc) (int retVal___, struct config_t *config, FILE *stream);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_read_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_read_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config, stream);
		}
	}
	return retVal___;
}
void HP_libconfig_write(const struct config_t *config, FILE *stream) {
	int hIndex = 0;
	if( HPMHooks.count.HP_libconfig_write_pre ) {
		void (*preHookFunc) (const struct config_t *config, FILE *stream);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_write_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_write_pre[hIndex].func;
			preHookFunc(config, stream);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.libconfig.write(config, stream);
	}
	if( HPMHooks.count.HP_libconfig_write_post ) {
		void (*postHookFunc) (const struct config_t *config, FILE *stream);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_write_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_write_post[hIndex].func;
			postHookFunc(config, stream);
		}
	}
	return;
}
void HP_libconfig_set_options(struct config_t *config, int options) {
	int hIndex = 0;
	if( HPMHooks.count.HP_libconfig_set_options_pre ) {
		void (*preHookFunc) (struct config_t *config, int *options);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_set_options_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_set_options_pre[hIndex].func;
			preHookFunc(config, &options);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.libconfig.set_options(config, options);
	}
	if( HPMHooks.count.HP_libconfig_set_options_post ) {
		void (*postHookFunc) (struct config_t *config, int *options);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_set_options_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_set_options_post[hIndex].func;
			postHookFunc(config, &options);
		}
	}
	return;
}
int HP_libconfig_get_options(const struct config_t *config) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_get_options_pre ) {
		int (*preHookFunc) (const struct config_t *config);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_get_options_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_get_options_pre[hIndex].func;
			retVal___ = preHookFunc(config);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.get_options(config);
	}
	if( HPMHooks.count.HP_libconfig_get_options_post ) {
		int (*postHookFunc) (int retVal___, const struct config_t *config);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_get_options_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_get_options_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config);
		}
	}
	return retVal___;
}
int HP_libconfig_read_string(struct config_t *config, const char *str) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_read_string_pre ) {
		int (*preHookFunc) (struct config_t *config, const char *str);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_read_string_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_read_string_pre[hIndex].func;
			retVal___ = preHookFunc(config, str);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.read_string(config, str);
	}
	if( HPMHooks.count.HP_libconfig_read_string_post ) {
		int (*postHookFunc) (int retVal___, struct config_t *config, const char *str);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_read_string_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_read_string_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config, str);
		}
	}
	return retVal___;
}
int HP_libconfig_read_file_src(struct config_t *config, const char *filename) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_read_file_src_pre ) {
		int (*preHookFunc) (struct config_t *config, const char *filename);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_read_file_src_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_read_file_src_pre[hIndex].func;
			retVal___ = preHookFunc(config, filename);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.read_file_src(config, filename);
	}
	if( HPMHooks.count.HP_libconfig_read_file_src_post ) {
		int (*postHookFunc) (int retVal___, struct config_t *config, const char *filename);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_read_file_src_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_read_file_src_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config, filename);
		}
	}
	return retVal___;
}
int HP_libconfig_write_file(struct config_t *config, const char *filename) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_write_file_pre ) {
		int (*preHookFunc) (struct config_t *config, const char *filename);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_write_file_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_write_file_pre[hIndex].func;
			retVal___ = preHookFunc(config, filename);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.write_file(config, filename);
	}
	if( HPMHooks.count.HP_libconfig_write_file_post ) {
		int (*postHookFunc) (int retVal___, struct config_t *config, const char *filename);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_write_file_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_write_file_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config, filename);
		}
	}
	return retVal___;
}
void HP_libconfig_set_destructor(struct config_t *config, void ( *destructor ) (void *)) {
	int hIndex = 0;
	if( HPMHooks.count.HP_libconfig_set_destructor_pre ) {
		void (*preHookFunc) (struct config_t *config, void ( *destructor ) (void *));
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_set_destructor_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_set_destructor_pre[hIndex].func;
			preHookFunc(config, destructor);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.libconfig.set_destructor(config, destructor);
	}
	if( HPMHooks.count.HP_libconfig_set_destructor_post ) {
		void (*postHookFunc) (struct config_t *config, void ( *destructor ) (void *));
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_set_destructor_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_set_destructor_post[hIndex].func;
			postHookFunc(config, destructor);
		}
	}
	return;
}
void HP_libconfig_set_include_dir(struct config_t *config, const char *include_dir) {
	int hIndex = 0;
	if( HPMHooks.count.HP_libconfig_set_include_dir_pre ) {
		void (*preHookFunc) (struct config_t *config, const char *include_dir);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_set_include_dir_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_set_include_dir_pre[hIndex].func;
			preHookFunc(config, include_dir);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.libconfig.set_include_dir(config, include_dir);
	}
	if( HPMHooks.count.HP_libconfig_set_include_dir_post ) {
		void (*postHookFunc) (struct config_t *config, const char *include_dir);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_set_include_dir_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_set_include_dir_post[hIndex].func;
			postHookFunc(config, include_dir);
		}
	}
	return;
}
void HP_libconfig_init(struct config_t *config) {
	int hIndex = 0;
	if( HPMHooks.count.HP_libconfig_init_pre ) {
		void (*preHookFunc) (struct config_t *config);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_init_pre[hIndex].func;
			preHookFunc(config);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.libconfig.init(config);
	}
	if( HPMHooks.count.HP_libconfig_init_post ) {
		void (*postHookFunc) (struct config_t *config);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_init_post[hIndex].func;
			postHookFunc(config);
		}
	}
	return;
}
void HP_libconfig_destroy(struct config_t *config) {
	int hIndex = 0;
	if( HPMHooks.count.HP_libconfig_destroy_pre ) {
		void (*preHookFunc) (struct config_t *config);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_destroy_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_destroy_pre[hIndex].func;
			preHookFunc(config);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.libconfig.destroy(config);
	}
	if( HPMHooks.count.HP_libconfig_destroy_post ) {
		void (*postHookFunc) (struct config_t *config);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_destroy_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_destroy_post[hIndex].func;
			postHookFunc(config);
		}
	}
	return;
}
int HP_libconfig_setting_get_int(const struct config_setting_t *setting) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_get_int_pre ) {
		int (*preHookFunc) (const struct config_setting_t *setting);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_int_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_int_pre[hIndex].func;
			retVal___ = preHookFunc(setting);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_int(setting);
	}
	if( HPMHooks.count.HP_libconfig_setting_get_int_post ) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_int_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_int_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting);
		}
	}
	return retVal___;
}
long long HP_libconfig_setting_get_int64(const struct config_setting_t *setting) {
	int hIndex = 0;
	long long retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_get_int64_pre ) {
		long long (*preHookFunc) (const struct config_setting_t *setting);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_int64_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_int64_pre[hIndex].func;
			retVal___ = preHookFunc(setting);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_int64(setting);
	}
	if( HPMHooks.count.HP_libconfig_setting_get_int64_post ) {
		long long (*postHookFunc) (long long retVal___, const struct config_setting_t *setting);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_int64_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_int64_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting);
		}
	}
	return retVal___;
}
double HP_libconfig_setting_get_float(const struct config_setting_t *setting) {
	int hIndex = 0;
	double retVal___ = 0.;
	if( HPMHooks.count.HP_libconfig_setting_get_float_pre ) {
		double (*preHookFunc) (const struct config_setting_t *setting);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_float_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_float_pre[hIndex].func;
			retVal___ = preHookFunc(setting);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_float(setting);
	}
	if( HPMHooks.count.HP_libconfig_setting_get_float_post ) {
		double (*postHookFunc) (double retVal___, const struct config_setting_t *setting);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_float_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_float_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_get_bool(const struct config_setting_t *setting) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_get_bool_pre ) {
		int (*preHookFunc) (const struct config_setting_t *setting);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_bool_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_bool_pre[hIndex].func;
			retVal___ = preHookFunc(setting);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_bool(setting);
	}
	if( HPMHooks.count.HP_libconfig_setting_get_bool_post ) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_bool_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_bool_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting);
		}
	}
	return retVal___;
}
const char* HP_libconfig_setting_get_string(const struct config_setting_t *setting) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if( HPMHooks.count.HP_libconfig_setting_get_string_pre ) {
		const char* (*preHookFunc) (const struct config_setting_t *setting);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_string_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_string_pre[hIndex].func;
			retVal___ = preHookFunc(setting);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_string(setting);
	}
	if( HPMHooks.count.HP_libconfig_setting_get_string_post ) {
		const char* (*postHookFunc) (const char* retVal___, const struct config_setting_t *setting);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_string_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_string_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting);
		}
	}
	return retVal___;
}
struct config_setting_t* HP_libconfig_setting_lookup(struct config_setting_t *setting, const char *name) {
	int hIndex = 0;
	struct config_setting_t* retVal___ = NULL;
	if( HPMHooks.count.HP_libconfig_setting_lookup_pre ) {
		struct config_setting_t* (*preHookFunc) (struct config_setting_t *setting, const char *name);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_pre[hIndex].func;
			retVal___ = preHookFunc(setting, name);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_lookup(setting, name);
	}
	if( HPMHooks.count.HP_libconfig_setting_lookup_post ) {
		struct config_setting_t* (*postHookFunc) (struct config_setting_t* retVal___, struct config_setting_t *setting, const char *name);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, name);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_lookup_int(const struct config_setting_t *setting, const char *name, int *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_lookup_int_pre ) {
		int (*preHookFunc) (const struct config_setting_t *setting, const char *name, int *value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_int_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_int_pre[hIndex].func;
			retVal___ = preHookFunc(setting, name, value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_lookup_int(setting, name, value);
	}
	if( HPMHooks.count.HP_libconfig_setting_lookup_int_post ) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting, const char *name, int *value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_int_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_int_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, name, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_lookup_int64(const struct config_setting_t *setting, const char *name, long long *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_lookup_int64_pre ) {
		int (*preHookFunc) (const struct config_setting_t *setting, const char *name, long long *value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_int64_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_int64_pre[hIndex].func;
			retVal___ = preHookFunc(setting, name, value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_lookup_int64(setting, name, value);
	}
	if( HPMHooks.count.HP_libconfig_setting_lookup_int64_post ) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting, const char *name, long long *value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_int64_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_int64_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, name, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_lookup_float(const struct config_setting_t *setting, const char *name, double *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_lookup_float_pre ) {
		int (*preHookFunc) (const struct config_setting_t *setting, const char *name, double *value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_float_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_float_pre[hIndex].func;
			retVal___ = preHookFunc(setting, name, value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_lookup_float(setting, name, value);
	}
	if( HPMHooks.count.HP_libconfig_setting_lookup_float_post ) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting, const char *name, double *value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_float_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_float_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, name, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_lookup_bool(const struct config_setting_t *setting, const char *name, int *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_lookup_bool_pre ) {
		int (*preHookFunc) (const struct config_setting_t *setting, const char *name, int *value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_bool_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_bool_pre[hIndex].func;
			retVal___ = preHookFunc(setting, name, value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_lookup_bool(setting, name, value);
	}
	if( HPMHooks.count.HP_libconfig_setting_lookup_bool_post ) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting, const char *name, int *value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_bool_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_bool_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, name, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_lookup_string(const struct config_setting_t *setting, const char *name, const char **value) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_lookup_string_pre ) {
		int (*preHookFunc) (const struct config_setting_t *setting, const char *name, const char **value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_string_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_string_pre[hIndex].func;
			retVal___ = preHookFunc(setting, name, value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_lookup_string(setting, name, value);
	}
	if( HPMHooks.count.HP_libconfig_setting_lookup_string_post ) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting, const char *name, const char **value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_string_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_string_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, name, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_set_int(struct config_setting_t *setting, int value) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_set_int_pre ) {
		int (*preHookFunc) (struct config_setting_t *setting, int *value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_int_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_set_int_pre[hIndex].func;
			retVal___ = preHookFunc(setting, &value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_set_int(setting, value);
	}
	if( HPMHooks.count.HP_libconfig_setting_set_int_post ) {
		int (*postHookFunc) (int retVal___, struct config_setting_t *setting, int *value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_int_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_set_int_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, &value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_set_int64(struct config_setting_t *setting, long long value) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_set_int64_pre ) {
		int (*preHookFunc) (struct config_setting_t *setting, long long *value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_int64_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_set_int64_pre[hIndex].func;
			retVal___ = preHookFunc(setting, &value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_set_int64(setting, value);
	}
	if( HPMHooks.count.HP_libconfig_setting_set_int64_post ) {
		int (*postHookFunc) (int retVal___, struct config_setting_t *setting, long long *value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_int64_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_set_int64_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, &value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_set_float(struct config_setting_t *setting, double value) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_set_float_pre ) {
		int (*preHookFunc) (struct config_setting_t *setting, double *value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_float_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_set_float_pre[hIndex].func;
			retVal___ = preHookFunc(setting, &value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_set_float(setting, value);
	}
	if( HPMHooks.count.HP_libconfig_setting_set_float_post ) {
		int (*postHookFunc) (int retVal___, struct config_setting_t *setting, double *value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_float_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_set_float_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, &value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_set_bool(struct config_setting_t *setting, int value) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_set_bool_pre ) {
		int (*preHookFunc) (struct config_setting_t *setting, int *value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_bool_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_set_bool_pre[hIndex].func;
			retVal___ = preHookFunc(setting, &value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_set_bool(setting, value);
	}
	if( HPMHooks.count.HP_libconfig_setting_set_bool_post ) {
		int (*postHookFunc) (int retVal___, struct config_setting_t *setting, int *value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_bool_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_set_bool_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, &value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_set_string(struct config_setting_t *setting, const char *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_set_string_pre ) {
		int (*preHookFunc) (struct config_setting_t *setting, const char *value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_string_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_set_string_pre[hIndex].func;
			retVal___ = preHookFunc(setting, value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_set_string(setting, value);
	}
	if( HPMHooks.count.HP_libconfig_setting_set_string_post ) {
		int (*postHookFunc) (int retVal___, struct config_setting_t *setting, const char *value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_string_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_set_string_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_set_format(struct config_setting_t *setting, short format) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_set_format_pre ) {
		int (*preHookFunc) (struct config_setting_t *setting, short *format);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_format_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_set_format_pre[hIndex].func;
			retVal___ = preHookFunc(setting, &format);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_set_format(setting, format);
	}
	if( HPMHooks.count.HP_libconfig_setting_set_format_post ) {
		int (*postHookFunc) (int retVal___, struct config_setting_t *setting, short *format);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_format_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_set_format_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, &format);
		}
	}
	return retVal___;
}
short HP_libconfig_setting_get_format(const struct config_setting_t *setting) {
	int hIndex = 0;
	short retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_get_format_pre ) {
		short (*preHookFunc) (const struct config_setting_t *setting);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_format_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_format_pre[hIndex].func;
			retVal___ = preHookFunc(setting);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_format(setting);
	}
	if( HPMHooks.count.HP_libconfig_setting_get_format_post ) {
		short (*postHookFunc) (short retVal___, const struct config_setting_t *setting);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_format_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_format_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_get_int_elem(const struct config_setting_t *setting, int idx) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_get_int_elem_pre ) {
		int (*preHookFunc) (const struct config_setting_t *setting, int *idx);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_int_elem_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_int_elem_pre[hIndex].func;
			retVal___ = preHookFunc(setting, &idx);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_int_elem(setting, idx);
	}
	if( HPMHooks.count.HP_libconfig_setting_get_int_elem_post ) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting, int *idx);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_int_elem_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_int_elem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, &idx);
		}
	}
	return retVal___;
}
long long HP_libconfig_setting_get_int64_elem(const struct config_setting_t *setting, int idx) {
	int hIndex = 0;
	long long retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_get_int64_elem_pre ) {
		long long (*preHookFunc) (const struct config_setting_t *setting, int *idx);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_int64_elem_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_int64_elem_pre[hIndex].func;
			retVal___ = preHookFunc(setting, &idx);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_int64_elem(setting, idx);
	}
	if( HPMHooks.count.HP_libconfig_setting_get_int64_elem_post ) {
		long long (*postHookFunc) (long long retVal___, const struct config_setting_t *setting, int *idx);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_int64_elem_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_int64_elem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, &idx);
		}
	}
	return retVal___;
}
double HP_libconfig_setting_get_float_elem(const struct config_setting_t *setting, int idx) {
	int hIndex = 0;
	double retVal___ = 0.;
	if( HPMHooks.count.HP_libconfig_setting_get_float_elem_pre ) {
		double (*preHookFunc) (const struct config_setting_t *setting, int *idx);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_float_elem_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_float_elem_pre[hIndex].func;
			retVal___ = preHookFunc(setting, &idx);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_float_elem(setting, idx);
	}
	if( HPMHooks.count.HP_libconfig_setting_get_float_elem_post ) {
		double (*postHookFunc) (double retVal___, const struct config_setting_t *setting, int *idx);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_float_elem_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_float_elem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, &idx);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_get_bool_elem(const struct config_setting_t *setting, int idx) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_get_bool_elem_pre ) {
		int (*preHookFunc) (const struct config_setting_t *setting, int *idx);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_bool_elem_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_bool_elem_pre[hIndex].func;
			retVal___ = preHookFunc(setting, &idx);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_bool_elem(setting, idx);
	}
	if( HPMHooks.count.HP_libconfig_setting_get_bool_elem_post ) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting, int *idx);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_bool_elem_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_bool_elem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, &idx);
		}
	}
	return retVal___;
}
const char* HP_libconfig_setting_get_string_elem(const struct config_setting_t *setting, int idx) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if( HPMHooks.count.HP_libconfig_setting_get_string_elem_pre ) {
		const char* (*preHookFunc) (const struct config_setting_t *setting, int *idx);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_string_elem_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_string_elem_pre[hIndex].func;
			retVal___ = preHookFunc(setting, &idx);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_string_elem(setting, idx);
	}
	if( HPMHooks.count.HP_libconfig_setting_get_string_elem_post ) {
		const char* (*postHookFunc) (const char* retVal___, const struct config_setting_t *setting, int *idx);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_string_elem_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_string_elem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, &idx);
		}
	}
	return retVal___;
}
struct config_setting_t* HP_libconfig_setting_set_int_elem(struct config_setting_t *setting, int idx, int value) {
	int hIndex = 0;
	struct config_setting_t* retVal___ = NULL;
	if( HPMHooks.count.HP_libconfig_setting_set_int_elem_pre ) {
		struct config_setting_t* (*preHookFunc) (struct config_setting_t *setting, int *idx, int *value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_int_elem_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_set_int_elem_pre[hIndex].func;
			retVal___ = preHookFunc(setting, &idx, &value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_set_int_elem(setting, idx, value);
	}
	if( HPMHooks.count.HP_libconfig_setting_set_int_elem_post ) {
		struct config_setting_t* (*postHookFunc) (struct config_setting_t* retVal___, struct config_setting_t *setting, int *idx, int *value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_int_elem_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_set_int_elem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, &idx, &value);
		}
	}
	return retVal___;
}
struct config_setting_t* HP_libconfig_setting_set_int64_elem(struct config_setting_t *setting, int idx, long long value) {
	int hIndex = 0;
	struct config_setting_t* retVal___ = NULL;
	if( HPMHooks.count.HP_libconfig_setting_set_int64_elem_pre ) {
		struct config_setting_t* (*preHookFunc) (struct config_setting_t *setting, int *idx, long long *value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_int64_elem_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_set_int64_elem_pre[hIndex].func;
			retVal___ = preHookFunc(setting, &idx, &value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_set_int64_elem(setting, idx, value);
	}
	if( HPMHooks.count.HP_libconfig_setting_set_int64_elem_post ) {
		struct config_setting_t* (*postHookFunc) (struct config_setting_t* retVal___, struct config_setting_t *setting, int *idx, long long *value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_int64_elem_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_set_int64_elem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, &idx, &value);
		}
	}
	return retVal___;
}
struct config_setting_t* HP_libconfig_setting_set_float_elem(struct config_setting_t *setting, int idx, double value) {
	int hIndex = 0;
	struct config_setting_t* retVal___ = NULL;
	if( HPMHooks.count.HP_libconfig_setting_set_float_elem_pre ) {
		struct config_setting_t* (*preHookFunc) (struct config_setting_t *setting, int *idx, double *value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_float_elem_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_set_float_elem_pre[hIndex].func;
			retVal___ = preHookFunc(setting, &idx, &value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_set_float_elem(setting, idx, value);
	}
	if( HPMHooks.count.HP_libconfig_setting_set_float_elem_post ) {
		struct config_setting_t* (*postHookFunc) (struct config_setting_t* retVal___, struct config_setting_t *setting, int *idx, double *value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_float_elem_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_set_float_elem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, &idx, &value);
		}
	}
	return retVal___;
}
struct config_setting_t* HP_libconfig_setting_set_bool_elem(struct config_setting_t *setting, int idx, int value) {
	int hIndex = 0;
	struct config_setting_t* retVal___ = NULL;
	if( HPMHooks.count.HP_libconfig_setting_set_bool_elem_pre ) {
		struct config_setting_t* (*preHookFunc) (struct config_setting_t *setting, int *idx, int *value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_bool_elem_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_set_bool_elem_pre[hIndex].func;
			retVal___ = preHookFunc(setting, &idx, &value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_set_bool_elem(setting, idx, value);
	}
	if( HPMHooks.count.HP_libconfig_setting_set_bool_elem_post ) {
		struct config_setting_t* (*postHookFunc) (struct config_setting_t* retVal___, struct config_setting_t *setting, int *idx, int *value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_bool_elem_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_set_bool_elem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, &idx, &value);
		}
	}
	return retVal___;
}
struct config_setting_t* HP_libconfig_setting_set_string_elem(struct config_setting_t *setting, int idx, const char *value) {
	int hIndex = 0;
	struct config_setting_t* retVal___ = NULL;
	if( HPMHooks.count.HP_libconfig_setting_set_string_elem_pre ) {
		struct config_setting_t* (*preHookFunc) (struct config_setting_t *setting, int *idx, const char *value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_string_elem_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_set_string_elem_pre[hIndex].func;
			retVal___ = preHookFunc(setting, &idx, value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_set_string_elem(setting, idx, value);
	}
	if( HPMHooks.count.HP_libconfig_setting_set_string_elem_post ) {
		struct config_setting_t* (*postHookFunc) (struct config_setting_t* retVal___, struct config_setting_t *setting, int *idx, const char *value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_string_elem_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_set_string_elem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, &idx, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_index(const struct config_setting_t *setting) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_index_pre ) {
		int (*preHookFunc) (const struct config_setting_t *setting);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_index_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_index_pre[hIndex].func;
			retVal___ = preHookFunc(setting);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_index(setting);
	}
	if( HPMHooks.count.HP_libconfig_setting_index_post ) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_index_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_index_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_length(const struct config_setting_t *setting) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_length_pre ) {
		int (*preHookFunc) (const struct config_setting_t *setting);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_length_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_length_pre[hIndex].func;
			retVal___ = preHookFunc(setting);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_length(setting);
	}
	if( HPMHooks.count.HP_libconfig_setting_length_post ) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_length_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_length_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting);
		}
	}
	return retVal___;
}
struct config_setting_t* HP_libconfig_setting_get_elem(const struct config_setting_t *setting, unsigned int idx) {
	int hIndex = 0;
	struct config_setting_t* retVal___ = NULL;
	if( HPMHooks.count.HP_libconfig_setting_get_elem_pre ) {
		struct config_setting_t* (*preHookFunc) (const struct config_setting_t *setting, unsigned int *idx);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_elem_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_elem_pre[hIndex].func;
			retVal___ = preHookFunc(setting, &idx);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_elem(setting, idx);
	}
	if( HPMHooks.count.HP_libconfig_setting_get_elem_post ) {
		struct config_setting_t* (*postHookFunc) (struct config_setting_t* retVal___, const struct config_setting_t *setting, unsigned int *idx);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_elem_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_elem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, &idx);
		}
	}
	return retVal___;
}
struct config_setting_t* HP_libconfig_setting_get_member(const struct config_setting_t *setting, const char *name) {
	int hIndex = 0;
	struct config_setting_t* retVal___ = NULL;
	if( HPMHooks.count.HP_libconfig_setting_get_member_pre ) {
		struct config_setting_t* (*preHookFunc) (const struct config_setting_t *setting, const char *name);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_member_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_member_pre[hIndex].func;
			retVal___ = preHookFunc(setting, name);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_member(setting, name);
	}
	if( HPMHooks.count.HP_libconfig_setting_get_member_post ) {
		struct config_setting_t* (*postHookFunc) (struct config_setting_t* retVal___, const struct config_setting_t *setting, const char *name);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_member_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_member_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, name);
		}
	}
	return retVal___;
}
struct config_setting_t* HP_libconfig_setting_add(struct config_setting_t *parent, const char *name, int type) {
	int hIndex = 0;
	struct config_setting_t* retVal___ = NULL;
	if( HPMHooks.count.HP_libconfig_setting_add_pre ) {
		struct config_setting_t* (*preHookFunc) (struct config_setting_t *parent, const char *name, int *type);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_add_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_add_pre[hIndex].func;
			retVal___ = preHookFunc(parent, name, &type);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_add(parent, name, type);
	}
	if( HPMHooks.count.HP_libconfig_setting_add_post ) {
		struct config_setting_t* (*postHookFunc) (struct config_setting_t* retVal___, struct config_setting_t *parent, const char *name, int *type);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_add_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_add_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent, name, &type);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_remove(struct config_setting_t *parent, const char *name) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_remove_pre ) {
		int (*preHookFunc) (struct config_setting_t *parent, const char *name);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_remove_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_remove_pre[hIndex].func;
			retVal___ = preHookFunc(parent, name);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_remove(parent, name);
	}
	if( HPMHooks.count.HP_libconfig_setting_remove_post ) {
		int (*postHookFunc) (int retVal___, struct config_setting_t *parent, const char *name);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_remove_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_remove_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent, name);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_remove_elem(struct config_setting_t *parent, unsigned int idx) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_remove_elem_pre ) {
		int (*preHookFunc) (struct config_setting_t *parent, unsigned int *idx);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_remove_elem_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_remove_elem_pre[hIndex].func;
			retVal___ = preHookFunc(parent, &idx);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_remove_elem(parent, idx);
	}
	if( HPMHooks.count.HP_libconfig_setting_remove_elem_post ) {
		int (*postHookFunc) (int retVal___, struct config_setting_t *parent, unsigned int *idx);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_remove_elem_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_remove_elem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent, &idx);
		}
	}
	return retVal___;
}
void HP_libconfig_setting_set_hook(struct config_setting_t *setting, void *hook) {
	int hIndex = 0;
	if( HPMHooks.count.HP_libconfig_setting_set_hook_pre ) {
		void (*preHookFunc) (struct config_setting_t *setting, void *hook);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_hook_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_set_hook_pre[hIndex].func;
			preHookFunc(setting, hook);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.libconfig.setting_set_hook(setting, hook);
	}
	if( HPMHooks.count.HP_libconfig_setting_set_hook_post ) {
		void (*postHookFunc) (struct config_setting_t *setting, void *hook);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_hook_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_set_hook_post[hIndex].func;
			postHookFunc(setting, hook);
		}
	}
	return;
}
struct config_setting_t* HP_libconfig_lookup(const struct config_t *config, const char *filepath) {
	int hIndex = 0;
	struct config_setting_t* retVal___ = NULL;
	if( HPMHooks.count.HP_libconfig_lookup_pre ) {
		struct config_setting_t* (*preHookFunc) (const struct config_t *config, const char *filepath);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_lookup_pre[hIndex].func;
			retVal___ = preHookFunc(config, filepath);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.lookup(config, filepath);
	}
	if( HPMHooks.count.HP_libconfig_lookup_post ) {
		struct config_setting_t* (*postHookFunc) (struct config_setting_t* retVal___, const struct config_t *config, const char *filepath);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_lookup_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config, filepath);
		}
	}
	return retVal___;
}
int HP_libconfig_lookup_int(const struct config_t *config, const char *filepath, int *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_lookup_int_pre ) {
		int (*preHookFunc) (const struct config_t *config, const char *filepath, int *value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_int_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_lookup_int_pre[hIndex].func;
			retVal___ = preHookFunc(config, filepath, value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.lookup_int(config, filepath, value);
	}
	if( HPMHooks.count.HP_libconfig_lookup_int_post ) {
		int (*postHookFunc) (int retVal___, const struct config_t *config, const char *filepath, int *value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_int_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_lookup_int_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config, filepath, value);
		}
	}
	return retVal___;
}
int HP_libconfig_lookup_int64(const struct config_t *config, const char *filepath, long long *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_lookup_int64_pre ) {
		int (*preHookFunc) (const struct config_t *config, const char *filepath, long long *value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_int64_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_lookup_int64_pre[hIndex].func;
			retVal___ = preHookFunc(config, filepath, value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.lookup_int64(config, filepath, value);
	}
	if( HPMHooks.count.HP_libconfig_lookup_int64_post ) {
		int (*postHookFunc) (int retVal___, const struct config_t *config, const char *filepath, long long *value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_int64_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_lookup_int64_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config, filepath, value);
		}
	}
	return retVal___;
}
int HP_libconfig_lookup_float(const struct config_t *config, const char *filepath, double *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_lookup_float_pre ) {
		int (*preHookFunc) (const struct config_t *config, const char *filepath, double *value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_float_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_lookup_float_pre[hIndex].func;
			retVal___ = preHookFunc(config, filepath, value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.lookup_float(config, filepath, value);
	}
	if( HPMHooks.count.HP_libconfig_lookup_float_post ) {
		int (*postHookFunc) (int retVal___, const struct config_t *config, const char *filepath, double *value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_float_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_lookup_float_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config, filepath, value);
		}
	}
	return retVal___;
}
int HP_libconfig_lookup_bool(const struct config_t *config, const char *filepath, int *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_lookup_bool_pre ) {
		int (*preHookFunc) (const struct config_t *config, const char *filepath, int *value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_bool_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_lookup_bool_pre[hIndex].func;
			retVal___ = preHookFunc(config, filepath, value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.lookup_bool(config, filepath, value);
	}
	if( HPMHooks.count.HP_libconfig_lookup_bool_post ) {
		int (*postHookFunc) (int retVal___, const struct config_t *config, const char *filepath, int *value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_bool_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_lookup_bool_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config, filepath, value);
		}
	}
	return retVal___;
}
int HP_libconfig_lookup_string(const struct config_t *config, const char *filepath, const char **value) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_lookup_string_pre ) {
		int (*preHookFunc) (const struct config_t *config, const char *filepath, const char **value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_string_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_lookup_string_pre[hIndex].func;
			retVal___ = preHookFunc(config, filepath, value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.lookup_string(config, filepath, value);
	}
	if( HPMHooks.count.HP_libconfig_lookup_string_post ) {
		int (*postHookFunc) (int retVal___, const struct config_t *config, const char *filepath, const char **value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_string_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_lookup_string_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config, filepath, value);
		}
	}
	return retVal___;
}
int HP_libconfig_load_file(struct config_t *config, const char *config_filename) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_load_file_pre ) {
		int (*preHookFunc) (struct config_t *config, const char *config_filename);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_load_file_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_load_file_pre[hIndex].func;
			retVal___ = preHookFunc(config, config_filename);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.load_file(config, config_filename);
	}
	if( HPMHooks.count.HP_libconfig_load_file_post ) {
		int (*postHookFunc) (int retVal___, struct config_t *config, const char *config_filename);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_load_file_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_load_file_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config, config_filename);
		}
	}
	return retVal___;
}
void HP_libconfig_setting_copy_simple(struct config_setting_t *parent, const struct config_setting_t *src) {
	int hIndex = 0;
	if( HPMHooks.count.HP_libconfig_setting_copy_simple_pre ) {
		void (*preHookFunc) (struct config_setting_t *parent, const struct config_setting_t *src);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_copy_simple_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_copy_simple_pre[hIndex].func;
			preHookFunc(parent, src);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.libconfig.setting_copy_simple(parent, src);
	}
	if( HPMHooks.count.HP_libconfig_setting_copy_simple_post ) {
		void (*postHookFunc) (struct config_setting_t *parent, const struct config_setting_t *src);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_copy_simple_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_copy_simple_post[hIndex].func;
			postHookFunc(parent, src);
		}
	}
	return;
}
void HP_libconfig_setting_copy_elem(struct config_setting_t *parent, const struct config_setting_t *src) {
	int hIndex = 0;
	if( HPMHooks.count.HP_libconfig_setting_copy_elem_pre ) {
		void (*preHookFunc) (struct config_setting_t *parent, const struct config_setting_t *src);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_copy_elem_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_copy_elem_pre[hIndex].func;
			preHookFunc(parent, src);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.libconfig.setting_copy_elem(parent, src);
	}
	if( HPMHooks.count.HP_libconfig_setting_copy_elem_post ) {
		void (*postHookFunc) (struct config_setting_t *parent, const struct config_setting_t *src);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_copy_elem_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_copy_elem_post[hIndex].func;
			postHookFunc(parent, src);
		}
	}
	return;
}
void HP_libconfig_setting_copy_aggregate(struct config_setting_t *parent, const struct config_setting_t *src) {
	int hIndex = 0;
	if( HPMHooks.count.HP_libconfig_setting_copy_aggregate_pre ) {
		void (*preHookFunc) (struct config_setting_t *parent, const struct config_setting_t *src);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_copy_aggregate_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_copy_aggregate_pre[hIndex].func;
			preHookFunc(parent, src);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.libconfig.setting_copy_aggregate(parent, src);
	}
	if( HPMHooks.count.HP_libconfig_setting_copy_aggregate_post ) {
		void (*postHookFunc) (struct config_setting_t *parent, const struct config_setting_t *src);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_copy_aggregate_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_copy_aggregate_post[hIndex].func;
			postHookFunc(parent, src);
		}
	}
	return;
}
int HP_libconfig_setting_copy(struct config_setting_t *parent, const struct config_setting_t *src) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_copy_pre ) {
		int (*preHookFunc) (struct config_setting_t *parent, const struct config_setting_t *src);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_copy_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_copy_pre[hIndex].func;
			retVal___ = preHookFunc(parent, src);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_copy(parent, src);
	}
	if( HPMHooks.count.HP_libconfig_setting_copy_post ) {
		int (*postHookFunc) (int retVal___, struct config_setting_t *parent, const struct config_setting_t *src);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_copy_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_copy_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent, src);
		}
	}
	return retVal___;
}
bool HP_libconfig_setting_get_bool_real(const struct config_setting_t *setting) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_libconfig_setting_get_bool_real_pre ) {
		bool (*preHookFunc) (const struct config_setting_t *setting);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_bool_real_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_bool_real_pre[hIndex].func;
			retVal___ = preHookFunc(setting);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_bool_real(setting);
	}
	if( HPMHooks.count.HP_libconfig_setting_get_bool_real_post ) {
		bool (*postHookFunc) (bool retVal___, const struct config_setting_t *setting);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_bool_real_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_bool_real_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting);
		}
	}
	return retVal___;
}
uint32 HP_libconfig_setting_get_uint32(const struct config_setting_t *setting) {
	int hIndex = 0;
	uint32 retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_get_uint32_pre ) {
		uint32 (*preHookFunc) (const struct config_setting_t *setting);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_uint32_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_uint32_pre[hIndex].func;
			retVal___ = preHookFunc(setting);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_uint32(setting);
	}
	if( HPMHooks.count.HP_libconfig_setting_get_uint32_post ) {
		uint32 (*postHookFunc) (uint32 retVal___, const struct config_setting_t *setting);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_uint32_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_uint32_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting);
		}
	}
	return retVal___;
}
uint16 HP_libconfig_setting_get_uint16(const struct config_setting_t *setting) {
	int hIndex = 0;
	uint16 retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_get_uint16_pre ) {
		uint16 (*preHookFunc) (const struct config_setting_t *setting);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_uint16_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_uint16_pre[hIndex].func;
			retVal___ = preHookFunc(setting);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_uint16(setting);
	}
	if( HPMHooks.count.HP_libconfig_setting_get_uint16_post ) {
		uint16 (*postHookFunc) (uint16 retVal___, const struct config_setting_t *setting);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_uint16_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_uint16_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting);
		}
	}
	return retVal___;
}
int16 HP_libconfig_setting_get_int16(const struct config_setting_t *setting) {
	int hIndex = 0;
	int16 retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_get_int16_pre ) {
		int16 (*preHookFunc) (const struct config_setting_t *setting);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_int16_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_int16_pre[hIndex].func;
			retVal___ = preHookFunc(setting);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_int16(setting);
	}
	if( HPMHooks.count.HP_libconfig_setting_get_int16_post ) {
		int16 (*postHookFunc) (int16 retVal___, const struct config_setting_t *setting);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_int16_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_int16_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_lookup_bool_real(const struct config_setting_t *setting, const char *name, bool *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_lookup_bool_real_pre ) {
		int (*preHookFunc) (const struct config_setting_t *setting, const char *name, bool *value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_bool_real_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_bool_real_pre[hIndex].func;
			retVal___ = preHookFunc(setting, name, value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_lookup_bool_real(setting, name, value);
	}
	if( HPMHooks.count.HP_libconfig_setting_lookup_bool_real_post ) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting, const char *name, bool *value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_bool_real_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_bool_real_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, name, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_lookup_uint32(const struct config_setting_t *setting, const char *name, uint32 *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_lookup_uint32_pre ) {
		int (*preHookFunc) (const struct config_setting_t *setting, const char *name, uint32 *value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_uint32_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_uint32_pre[hIndex].func;
			retVal___ = preHookFunc(setting, name, value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_lookup_uint32(setting, name, value);
	}
	if( HPMHooks.count.HP_libconfig_setting_lookup_uint32_post ) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting, const char *name, uint32 *value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_uint32_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_uint32_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, name, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_lookup_uint16(const struct config_setting_t *setting, const char *name, uint16 *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_lookup_uint16_pre ) {
		int (*preHookFunc) (const struct config_setting_t *setting, const char *name, uint16 *value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_uint16_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_uint16_pre[hIndex].func;
			retVal___ = preHookFunc(setting, name, value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_lookup_uint16(setting, name, value);
	}
	if( HPMHooks.count.HP_libconfig_setting_lookup_uint16_post ) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting, const char *name, uint16 *value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_uint16_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_uint16_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, name, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_lookup_int16(const struct config_setting_t *setting, const char *name, int16 *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_lookup_int16_pre ) {
		int (*preHookFunc) (const struct config_setting_t *setting, const char *name, int16 *value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_int16_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_int16_pre[hIndex].func;
			retVal___ = preHookFunc(setting, name, value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_lookup_int16(setting, name, value);
	}
	if( HPMHooks.count.HP_libconfig_setting_lookup_int16_post ) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting, const char *name, int16 *value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_int16_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_int16_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, name, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_lookup_mutable_string(const struct config_setting_t *setting, const char *name, char *out, size_t out_size) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_setting_lookup_mutable_string_pre ) {
		int (*preHookFunc) (const struct config_setting_t *setting, const char *name, char *out, size_t *out_size);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_mutable_string_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_mutable_string_pre[hIndex].func;
			retVal___ = preHookFunc(setting, name, out, &out_size);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_lookup_mutable_string(setting, name, out, out_size);
	}
	if( HPMHooks.count.HP_libconfig_setting_lookup_mutable_string_post ) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting, const char *name, char *out, size_t *out_size);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_mutable_string_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_mutable_string_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, name, out, &out_size);
		}
	}
	return retVal___;
}
int HP_libconfig_lookup_mutable_string(const struct config_t *config, const char *name, char *out, size_t out_size) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_libconfig_lookup_mutable_string_pre ) {
		int (*preHookFunc) (const struct config_t *config, const char *name, char *out, size_t *out_size);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_mutable_string_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_libconfig_lookup_mutable_string_pre[hIndex].func;
			retVal___ = preHookFunc(config, name, out, &out_size);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.lookup_mutable_string(config, name, out, out_size);
	}
	if( HPMHooks.count.HP_libconfig_lookup_mutable_string_post ) {
		int (*postHookFunc) (int retVal___, const struct config_t *config, const char *name, char *out, size_t *out_size);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_mutable_string_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_libconfig_lookup_mutable_string_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config, name, out, &out_size);
		}
	}
	return retVal___;
}
/* login_interface */
int HP_login_mmo_auth(struct login_session_data *sd, bool isServer) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_login_mmo_auth_pre ) {
		int (*preHookFunc) (struct login_session_data *sd, bool *isServer);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_mmo_auth_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_mmo_auth_pre[hIndex].func;
			retVal___ = preHookFunc(sd, &isServer);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.login.mmo_auth(sd, isServer);
	}
	if( HPMHooks.count.HP_login_mmo_auth_post ) {
		int (*postHookFunc) (int retVal___, struct login_session_data *sd, bool *isServer);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_mmo_auth_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_mmo_auth_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, sd, &isServer);
		}
	}
	return retVal___;
}
int HP_login_mmo_auth_new(const char *userid, const char *pass, const char sex, const char *last_ip) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_login_mmo_auth_new_pre ) {
		int (*preHookFunc) (const char *userid, const char *pass, const char *sex, const char *last_ip);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_mmo_auth_new_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_mmo_auth_new_pre[hIndex].func;
			retVal___ = preHookFunc(userid, pass, &sex, last_ip);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.login.mmo_auth_new(userid, pass, sex, last_ip);
	}
	if( HPMHooks.count.HP_login_mmo_auth_new_post ) {
		int (*postHookFunc) (int retVal___, const char *userid, const char *pass, const char *sex, const char *last_ip);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_mmo_auth_new_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_mmo_auth_new_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, userid, pass, &sex, last_ip);
		}
	}
	return retVal___;
}
int HP_login_waiting_disconnect_timer(int tid, int64 tick, int id, intptr_t data) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_login_waiting_disconnect_timer_pre ) {
		int (*preHookFunc) (int *tid, int64 *tick, int *id, intptr_t *data);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_waiting_disconnect_timer_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_waiting_disconnect_timer_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &tick, &id, &data);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.login.waiting_disconnect_timer(tid, tick, id, data);
	}
	if( HPMHooks.count.HP_login_waiting_disconnect_timer_post ) {
		int (*postHookFunc) (int retVal___, int *tid, int64 *tick, int *id, intptr_t *data);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_waiting_disconnect_timer_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_waiting_disconnect_timer_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &tid, &tick, &id, &data);
		}
	}
	return retVal___;
}
struct DBData HP_login_create_online_user(union DBKey key, va_list args) {
	int hIndex = 0;
	struct DBData retVal___;
	memset(&retVal___, '\0', sizeof(struct DBData));
	if( HPMHooks.count.HP_login_create_online_user_pre ) {
		struct DBData (*preHookFunc) (union DBKey *key, va_list args);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_create_online_user_pre; hIndex++ ) {
			va_list args___copy; va_copy(args___copy, args);
			preHookFunc = HPMHooks.list.HP_login_create_online_user_pre[hIndex].func;
			retVal___ = preHookFunc(&key, args___copy);
			va_end(args___copy);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		va_list args___copy; va_copy(args___copy, args);
		retVal___ = HPMHooks.source.login.create_online_user(key, args___copy);
		va_end(args___copy);
	}
	if( HPMHooks.count.HP_login_create_online_user_post ) {
		struct DBData (*postHookFunc) (struct DBData retVal___, union DBKey *key, va_list args);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_create_online_user_post; hIndex++ ) {
			va_list args___copy; va_copy(args___copy, args);
			postHookFunc = HPMHooks.list.HP_login_create_online_user_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &key, args___copy);
			va_end(args___copy);
		}
	}
	return retVal___;
}
struct online_login_data* HP_login_add_online_user(int char_server, int account_id) {
	int hIndex = 0;
	struct online_login_data* retVal___ = NULL;
	if( HPMHooks.count.HP_login_add_online_user_pre ) {
		struct online_login_data* (*preHookFunc) (int *char_server, int *account_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_add_online_user_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_add_online_user_pre[hIndex].func;
			retVal___ = preHookFunc(&char_server, &account_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.login.add_online_user(char_server, account_id);
	}
	if( HPMHooks.count.HP_login_add_online_user_post ) {
		struct online_login_data* (*postHookFunc) (struct online_login_data* retVal___, int *char_server, int *account_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_add_online_user_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_add_online_user_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &char_server, &account_id);
		}
	}
	return retVal___;
}
void HP_login_remove_online_user(int account_id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_remove_online_user_pre ) {
		void (*preHookFunc) (int *account_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_remove_online_user_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_remove_online_user_pre[hIndex].func;
			preHookFunc(&account_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.remove_online_user(account_id);
	}
	if( HPMHooks.count.HP_login_remove_online_user_post ) {
		void (*postHookFunc) (int *account_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_remove_online_user_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_remove_online_user_post[hIndex].func;
			postHookFunc(&account_id);
		}
	}
	return;
}
int HP_login_online_db_setoffline(union DBKey key, struct DBData *data, va_list ap) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_login_online_db_setoffline_pre ) {
		int (*preHookFunc) (union DBKey *key, struct DBData *data, va_list ap);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_online_db_setoffline_pre; hIndex++ ) {
			va_list ap___copy; va_copy(ap___copy, ap);
			preHookFunc = HPMHooks.list.HP_login_online_db_setoffline_pre[hIndex].func;
			retVal___ = preHookFunc(&key, data, ap___copy);
			va_end(ap___copy);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		va_list ap___copy; va_copy(ap___copy, ap);
		retVal___ = HPMHooks.source.login.online_db_setoffline(key, data, ap___copy);
		va_end(ap___copy);
	}
	if( HPMHooks.count.HP_login_online_db_setoffline_post ) {
		int (*postHookFunc) (int retVal___, union DBKey *key, struct DBData *data, va_list ap);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_online_db_setoffline_post; hIndex++ ) {
			va_list ap___copy; va_copy(ap___copy, ap);
			postHookFunc = HPMHooks.list.HP_login_online_db_setoffline_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &key, data, ap___copy);
			va_end(ap___copy);
		}
	}
	return retVal___;
}
int HP_login_online_data_cleanup_sub(union DBKey key, struct DBData *data, va_list ap) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_login_online_data_cleanup_sub_pre ) {
		int (*preHookFunc) (union DBKey *key, struct DBData *data, va_list ap);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_online_data_cleanup_sub_pre; hIndex++ ) {
			va_list ap___copy; va_copy(ap___copy, ap);
			preHookFunc = HPMHooks.list.HP_login_online_data_cleanup_sub_pre[hIndex].func;
			retVal___ = preHookFunc(&key, data, ap___copy);
			va_end(ap___copy);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		va_list ap___copy; va_copy(ap___copy, ap);
		retVal___ = HPMHooks.source.login.online_data_cleanup_sub(key, data, ap___copy);
		va_end(ap___copy);
	}
	if( HPMHooks.count.HP_login_online_data_cleanup_sub_post ) {
		int (*postHookFunc) (int retVal___, union DBKey *key, struct DBData *data, va_list ap);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_online_data_cleanup_sub_post; hIndex++ ) {
			va_list ap___copy; va_copy(ap___copy, ap);
			postHookFunc = HPMHooks.list.HP_login_online_data_cleanup_sub_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &key, data, ap___copy);
			va_end(ap___copy);
		}
	}
	return retVal___;
}
int HP_login_online_data_cleanup(int tid, int64 tick, int id, intptr_t data) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_login_online_data_cleanup_pre ) {
		int (*preHookFunc) (int *tid, int64 *tick, int *id, intptr_t *data);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_online_data_cleanup_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_online_data_cleanup_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &tick, &id, &data);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.login.online_data_cleanup(tid, tick, id, data);
	}
	if( HPMHooks.count.HP_login_online_data_cleanup_post ) {
		int (*postHookFunc) (int retVal___, int *tid, int64 *tick, int *id, intptr_t *data);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_online_data_cleanup_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_online_data_cleanup_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &tid, &tick, &id, &data);
		}
	}
	return retVal___;
}
int HP_login_sync_ip_addresses(int tid, int64 tick, int id, intptr_t data) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_login_sync_ip_addresses_pre ) {
		int (*preHookFunc) (int *tid, int64 *tick, int *id, intptr_t *data);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_sync_ip_addresses_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_sync_ip_addresses_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &tick, &id, &data);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.login.sync_ip_addresses(tid, tick, id, data);
	}
	if( HPMHooks.count.HP_login_sync_ip_addresses_post ) {
		int (*postHookFunc) (int retVal___, int *tid, int64 *tick, int *id, intptr_t *data);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_sync_ip_addresses_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_sync_ip_addresses_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &tid, &tick, &id, &data);
		}
	}
	return retVal___;
}
bool HP_login_check_encrypted(const char *str1, const char *str2, const char *passwd) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_login_check_encrypted_pre ) {
		bool (*preHookFunc) (const char *str1, const char *str2, const char *passwd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_check_encrypted_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_check_encrypted_pre[hIndex].func;
			retVal___ = preHookFunc(str1, str2, passwd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.login.check_encrypted(str1, str2, passwd);
	}
	if( HPMHooks.count.HP_login_check_encrypted_post ) {
		bool (*postHookFunc) (bool retVal___, const char *str1, const char *str2, const char *passwd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_check_encrypted_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_check_encrypted_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, str1, str2, passwd);
		}
	}
	return retVal___;
}
bool HP_login_check_password(const char *md5key, int passwdenc, const char *passwd, const char *refpass) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_login_check_password_pre ) {
		bool (*preHookFunc) (const char *md5key, int *passwdenc, const char *passwd, const char *refpass);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_check_password_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_check_password_pre[hIndex].func;
			retVal___ = preHookFunc(md5key, &passwdenc, passwd, refpass);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.login.check_password(md5key, passwdenc, passwd, refpass);
	}
	if( HPMHooks.count.HP_login_check_password_post ) {
		bool (*postHookFunc) (bool retVal___, const char *md5key, int *passwdenc, const char *passwd, const char *refpass);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_check_password_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_check_password_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, md5key, &passwdenc, passwd, refpass);
		}
	}
	return retVal___;
}
uint32 HP_login_lan_subnet_check(uint32 ip) {
	int hIndex = 0;
	uint32 retVal___ = 0;
	if( HPMHooks.count.HP_login_lan_subnet_check_pre ) {
		uint32 (*preHookFunc) (uint32 *ip);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_lan_subnet_check_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_lan_subnet_check_pre[hIndex].func;
			retVal___ = preHookFunc(&ip);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.login.lan_subnet_check(ip);
	}
	if( HPMHooks.count.HP_login_lan_subnet_check_post ) {
		uint32 (*postHookFunc) (uint32 retVal___, uint32 *ip);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_lan_subnet_check_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_lan_subnet_check_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &ip);
		}
	}
	return retVal___;
}
void HP_login_fromchar_accinfo(int fd, int account_id, int u_fd, int u_aid, int u_group, int map_fd, struct mmo_account *acc) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_accinfo_pre ) {
		void (*preHookFunc) (int *fd, int *account_id, int *u_fd, int *u_aid, int *u_group, int *map_fd, struct mmo_account *acc);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_accinfo_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_accinfo_pre[hIndex].func;
			preHookFunc(&fd, &account_id, &u_fd, &u_aid, &u_group, &map_fd, acc);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_accinfo(fd, account_id, u_fd, u_aid, u_group, map_fd, acc);
	}
	if( HPMHooks.count.HP_login_fromchar_accinfo_post ) {
		void (*postHookFunc) (int *fd, int *account_id, int *u_fd, int *u_aid, int *u_group, int *map_fd, struct mmo_account *acc);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_accinfo_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_accinfo_post[hIndex].func;
			postHookFunc(&fd, &account_id, &u_fd, &u_aid, &u_group, &map_fd, acc);
		}
	}
	return;
}
void HP_login_fromchar_account(int fd, int account_id, struct mmo_account *acc) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_account_pre ) {
		void (*preHookFunc) (int *fd, int *account_id, struct mmo_account *acc);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_account_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_account_pre[hIndex].func;
			preHookFunc(&fd, &account_id, acc);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_account(fd, account_id, acc);
	}
	if( HPMHooks.count.HP_login_fromchar_account_post ) {
		void (*postHookFunc) (int *fd, int *account_id, struct mmo_account *acc);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_account_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_account_post[hIndex].func;
			postHookFunc(&fd, &account_id, acc);
		}
	}
	return;
}
void HP_login_fromchar_account_update_other(int account_id, unsigned int state) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_account_update_other_pre ) {
		void (*preHookFunc) (int *account_id, unsigned int *state);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_account_update_other_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_account_update_other_pre[hIndex].func;
			preHookFunc(&account_id, &state);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_account_update_other(account_id, state);
	}
	if( HPMHooks.count.HP_login_fromchar_account_update_other_post ) {
		void (*postHookFunc) (int *account_id, unsigned int *state);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_account_update_other_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_account_update_other_post[hIndex].func;
			postHookFunc(&account_id, &state);
		}
	}
	return;
}
void HP_login_fromchar_auth_ack(int fd, int account_id, uint32 login_id1, uint32 login_id2, uint8 sex, int request_id, struct login_auth_node *node) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_auth_ack_pre ) {
		void (*preHookFunc) (int *fd, int *account_id, uint32 *login_id1, uint32 *login_id2, uint8 *sex, int *request_id, struct login_auth_node *node);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_auth_ack_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_auth_ack_pre[hIndex].func;
			preHookFunc(&fd, &account_id, &login_id1, &login_id2, &sex, &request_id, node);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_auth_ack(fd, account_id, login_id1, login_id2, sex, request_id, node);
	}
	if( HPMHooks.count.HP_login_fromchar_auth_ack_post ) {
		void (*postHookFunc) (int *fd, int *account_id, uint32 *login_id1, uint32 *login_id2, uint8 *sex, int *request_id, struct login_auth_node *node);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_auth_ack_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_auth_ack_post[hIndex].func;
			postHookFunc(&fd, &account_id, &login_id1, &login_id2, &sex, &request_id, node);
		}
	}
	return;
}
void HP_login_fromchar_ban(int account_id, time_t timestamp) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_ban_pre ) {
		void (*preHookFunc) (int *account_id, time_t *timestamp);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_ban_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_ban_pre[hIndex].func;
			preHookFunc(&account_id, &timestamp);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_ban(account_id, timestamp);
	}
	if( HPMHooks.count.HP_login_fromchar_ban_post ) {
		void (*postHookFunc) (int *account_id, time_t *timestamp);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_ban_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_ban_post[hIndex].func;
			postHookFunc(&account_id, &timestamp);
		}
	}
	return;
}
void HP_login_fromchar_change_sex_other(int account_id, char sex) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_change_sex_other_pre ) {
		void (*preHookFunc) (int *account_id, char *sex);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_change_sex_other_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_change_sex_other_pre[hIndex].func;
			preHookFunc(&account_id, &sex);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_change_sex_other(account_id, sex);
	}
	if( HPMHooks.count.HP_login_fromchar_change_sex_other_post ) {
		void (*postHookFunc) (int *account_id, char *sex);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_change_sex_other_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_change_sex_other_post[hIndex].func;
			postHookFunc(&account_id, &sex);
		}
	}
	return;
}
void HP_login_fromchar_pong(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_pong_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_pong_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_pong_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_pong(fd);
	}
	if( HPMHooks.count.HP_login_fromchar_pong_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_pong_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_pong_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_login_fromchar_parse_auth(int fd, int id, const char *ip) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_parse_auth_pre ) {
		void (*preHookFunc) (int *fd, int *id, const char *ip);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_auth_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_parse_auth_pre[hIndex].func;
			preHookFunc(&fd, &id, ip);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_parse_auth(fd, id, ip);
	}
	if( HPMHooks.count.HP_login_fromchar_parse_auth_post ) {
		void (*postHookFunc) (int *fd, int *id, const char *ip);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_auth_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_parse_auth_post[hIndex].func;
			postHookFunc(&fd, &id, ip);
		}
	}
	return;
}
void HP_login_fromchar_parse_update_users(int fd, int id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_parse_update_users_pre ) {
		void (*preHookFunc) (int *fd, int *id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_update_users_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_parse_update_users_pre[hIndex].func;
			preHookFunc(&fd, &id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_parse_update_users(fd, id);
	}
	if( HPMHooks.count.HP_login_fromchar_parse_update_users_post ) {
		void (*postHookFunc) (int *fd, int *id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_update_users_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_parse_update_users_post[hIndex].func;
			postHookFunc(&fd, &id);
		}
	}
	return;
}
void HP_login_fromchar_parse_request_change_email(int fd, int id, const char *ip) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_parse_request_change_email_pre ) {
		void (*preHookFunc) (int *fd, int *id, const char *ip);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_request_change_email_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_parse_request_change_email_pre[hIndex].func;
			preHookFunc(&fd, &id, ip);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_parse_request_change_email(fd, id, ip);
	}
	if( HPMHooks.count.HP_login_fromchar_parse_request_change_email_post ) {
		void (*postHookFunc) (int *fd, int *id, const char *ip);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_request_change_email_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_parse_request_change_email_post[hIndex].func;
			postHookFunc(&fd, &id, ip);
		}
	}
	return;
}
void HP_login_fromchar_parse_account_data(int fd, int id, const char *ip) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_parse_account_data_pre ) {
		void (*preHookFunc) (int *fd, int *id, const char *ip);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_account_data_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_parse_account_data_pre[hIndex].func;
			preHookFunc(&fd, &id, ip);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_parse_account_data(fd, id, ip);
	}
	if( HPMHooks.count.HP_login_fromchar_parse_account_data_post ) {
		void (*postHookFunc) (int *fd, int *id, const char *ip);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_account_data_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_parse_account_data_post[hIndex].func;
			postHookFunc(&fd, &id, ip);
		}
	}
	return;
}
void HP_login_fromchar_parse_ping(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_parse_ping_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_ping_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_parse_ping_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_parse_ping(fd);
	}
	if( HPMHooks.count.HP_login_fromchar_parse_ping_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_ping_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_parse_ping_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_login_fromchar_parse_change_email(int fd, int id, const char *ip) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_parse_change_email_pre ) {
		void (*preHookFunc) (int *fd, int *id, const char *ip);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_change_email_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_parse_change_email_pre[hIndex].func;
			preHookFunc(&fd, &id, ip);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_parse_change_email(fd, id, ip);
	}
	if( HPMHooks.count.HP_login_fromchar_parse_change_email_post ) {
		void (*postHookFunc) (int *fd, int *id, const char *ip);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_change_email_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_parse_change_email_post[hIndex].func;
			postHookFunc(&fd, &id, ip);
		}
	}
	return;
}
void HP_login_fromchar_parse_account_update(int fd, int id, const char *ip) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_parse_account_update_pre ) {
		void (*preHookFunc) (int *fd, int *id, const char *ip);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_account_update_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_parse_account_update_pre[hIndex].func;
			preHookFunc(&fd, &id, ip);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_parse_account_update(fd, id, ip);
	}
	if( HPMHooks.count.HP_login_fromchar_parse_account_update_post ) {
		void (*postHookFunc) (int *fd, int *id, const char *ip);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_account_update_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_parse_account_update_post[hIndex].func;
			postHookFunc(&fd, &id, ip);
		}
	}
	return;
}
void HP_login_fromchar_parse_ban(int fd, int id, const char *ip) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_parse_ban_pre ) {
		void (*preHookFunc) (int *fd, int *id, const char *ip);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_ban_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_parse_ban_pre[hIndex].func;
			preHookFunc(&fd, &id, ip);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_parse_ban(fd, id, ip);
	}
	if( HPMHooks.count.HP_login_fromchar_parse_ban_post ) {
		void (*postHookFunc) (int *fd, int *id, const char *ip);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_ban_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_parse_ban_post[hIndex].func;
			postHookFunc(&fd, &id, ip);
		}
	}
	return;
}
void HP_login_fromchar_parse_change_sex(int fd, int id, const char *ip) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_parse_change_sex_pre ) {
		void (*preHookFunc) (int *fd, int *id, const char *ip);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_change_sex_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_parse_change_sex_pre[hIndex].func;
			preHookFunc(&fd, &id, ip);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_parse_change_sex(fd, id, ip);
	}
	if( HPMHooks.count.HP_login_fromchar_parse_change_sex_post ) {
		void (*postHookFunc) (int *fd, int *id, const char *ip);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_change_sex_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_parse_change_sex_post[hIndex].func;
			postHookFunc(&fd, &id, ip);
		}
	}
	return;
}
void HP_login_fromchar_parse_account_reg2(int fd, int id, const char *ip) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_parse_account_reg2_pre ) {
		void (*preHookFunc) (int *fd, int *id, const char *ip);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_account_reg2_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_parse_account_reg2_pre[hIndex].func;
			preHookFunc(&fd, &id, ip);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_parse_account_reg2(fd, id, ip);
	}
	if( HPMHooks.count.HP_login_fromchar_parse_account_reg2_post ) {
		void (*postHookFunc) (int *fd, int *id, const char *ip);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_account_reg2_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_parse_account_reg2_post[hIndex].func;
			postHookFunc(&fd, &id, ip);
		}
	}
	return;
}
void HP_login_fromchar_parse_unban(int fd, int id, const char *ip) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_parse_unban_pre ) {
		void (*preHookFunc) (int *fd, int *id, const char *ip);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_unban_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_parse_unban_pre[hIndex].func;
			preHookFunc(&fd, &id, ip);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_parse_unban(fd, id, ip);
	}
	if( HPMHooks.count.HP_login_fromchar_parse_unban_post ) {
		void (*postHookFunc) (int *fd, int *id, const char *ip);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_unban_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_parse_unban_post[hIndex].func;
			postHookFunc(&fd, &id, ip);
		}
	}
	return;
}
void HP_login_fromchar_parse_account_online(int fd, int id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_parse_account_online_pre ) {
		void (*preHookFunc) (int *fd, int *id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_account_online_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_parse_account_online_pre[hIndex].func;
			preHookFunc(&fd, &id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_parse_account_online(fd, id);
	}
	if( HPMHooks.count.HP_login_fromchar_parse_account_online_post ) {
		void (*postHookFunc) (int *fd, int *id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_account_online_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_parse_account_online_post[hIndex].func;
			postHookFunc(&fd, &id);
		}
	}
	return;
}
void HP_login_fromchar_parse_account_offline(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_parse_account_offline_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_account_offline_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_parse_account_offline_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_parse_account_offline(fd);
	}
	if( HPMHooks.count.HP_login_fromchar_parse_account_offline_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_account_offline_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_parse_account_offline_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_login_fromchar_parse_online_accounts(int fd, int id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_parse_online_accounts_pre ) {
		void (*preHookFunc) (int *fd, int *id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_online_accounts_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_parse_online_accounts_pre[hIndex].func;
			preHookFunc(&fd, &id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_parse_online_accounts(fd, id);
	}
	if( HPMHooks.count.HP_login_fromchar_parse_online_accounts_post ) {
		void (*postHookFunc) (int *fd, int *id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_online_accounts_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_parse_online_accounts_post[hIndex].func;
			postHookFunc(&fd, &id);
		}
	}
	return;
}
void HP_login_fromchar_parse_request_account_reg2(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_parse_request_account_reg2_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_request_account_reg2_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_parse_request_account_reg2_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_parse_request_account_reg2(fd);
	}
	if( HPMHooks.count.HP_login_fromchar_parse_request_account_reg2_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_request_account_reg2_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_parse_request_account_reg2_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_login_fromchar_parse_update_wan_ip(int fd, int id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_parse_update_wan_ip_pre ) {
		void (*preHookFunc) (int *fd, int *id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_update_wan_ip_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_parse_update_wan_ip_pre[hIndex].func;
			preHookFunc(&fd, &id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_parse_update_wan_ip(fd, id);
	}
	if( HPMHooks.count.HP_login_fromchar_parse_update_wan_ip_post ) {
		void (*postHookFunc) (int *fd, int *id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_update_wan_ip_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_parse_update_wan_ip_post[hIndex].func;
			postHookFunc(&fd, &id);
		}
	}
	return;
}
void HP_login_fromchar_parse_all_offline(int fd, int id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_parse_all_offline_pre ) {
		void (*preHookFunc) (int *fd, int *id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_all_offline_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_parse_all_offline_pre[hIndex].func;
			preHookFunc(&fd, &id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_parse_all_offline(fd, id);
	}
	if( HPMHooks.count.HP_login_fromchar_parse_all_offline_post ) {
		void (*postHookFunc) (int *fd, int *id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_all_offline_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_parse_all_offline_post[hIndex].func;
			postHookFunc(&fd, &id);
		}
	}
	return;
}
void HP_login_fromchar_parse_change_pincode(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_parse_change_pincode_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_change_pincode_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_parse_change_pincode_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_parse_change_pincode(fd);
	}
	if( HPMHooks.count.HP_login_fromchar_parse_change_pincode_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_change_pincode_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_parse_change_pincode_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
bool HP_login_fromchar_parse_wrong_pincode(int fd) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_login_fromchar_parse_wrong_pincode_pre ) {
		bool (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_wrong_pincode_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_parse_wrong_pincode_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.login.fromchar_parse_wrong_pincode(fd);
	}
	if( HPMHooks.count.HP_login_fromchar_parse_wrong_pincode_post ) {
		bool (*postHookFunc) (bool retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_wrong_pincode_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_parse_wrong_pincode_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
void HP_login_fromchar_parse_accinfo(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_fromchar_parse_accinfo_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_accinfo_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_fromchar_parse_accinfo_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.fromchar_parse_accinfo(fd);
	}
	if( HPMHooks.count.HP_login_fromchar_parse_accinfo_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_fromchar_parse_accinfo_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_fromchar_parse_accinfo_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
int HP_login_parse_fromchar(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_login_parse_fromchar_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_parse_fromchar_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_parse_fromchar_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.login.parse_fromchar(fd);
	}
	if( HPMHooks.count.HP_login_parse_fromchar_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_parse_fromchar_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_parse_fromchar_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
void HP_login_connection_problem(int fd, uint8 status) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_connection_problem_pre ) {
		void (*preHookFunc) (int *fd, uint8 *status);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_connection_problem_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_connection_problem_pre[hIndex].func;
			preHookFunc(&fd, &status);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.connection_problem(fd, status);
	}
	if( HPMHooks.count.HP_login_connection_problem_post ) {
		void (*postHookFunc) (int *fd, uint8 *status);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_connection_problem_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_connection_problem_post[hIndex].func;
			postHookFunc(&fd, &status);
		}
	}
	return;
}
void HP_login_kick(struct login_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_kick_pre ) {
		void (*preHookFunc) (struct login_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_kick_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_kick_pre[hIndex].func;
			preHookFunc(sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.kick(sd);
	}
	if( HPMHooks.count.HP_login_kick_post ) {
		void (*postHookFunc) (struct login_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_kick_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_kick_post[hIndex].func;
			postHookFunc(sd);
		}
	}
	return;
}
void HP_login_auth_ok(struct login_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_auth_ok_pre ) {
		void (*preHookFunc) (struct login_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_auth_ok_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_auth_ok_pre[hIndex].func;
			preHookFunc(sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.auth_ok(sd);
	}
	if( HPMHooks.count.HP_login_auth_ok_post ) {
		void (*postHookFunc) (struct login_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_auth_ok_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_auth_ok_post[hIndex].func;
			postHookFunc(sd);
		}
	}
	return;
}
void HP_login_auth_failed(struct login_session_data *sd, int result) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_auth_failed_pre ) {
		void (*preHookFunc) (struct login_session_data *sd, int *result);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_auth_failed_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_auth_failed_pre[hIndex].func;
			preHookFunc(sd, &result);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.auth_failed(sd, result);
	}
	if( HPMHooks.count.HP_login_auth_failed_post ) {
		void (*postHookFunc) (struct login_session_data *sd, int *result);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_auth_failed_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_auth_failed_post[hIndex].func;
			postHookFunc(sd, &result);
		}
	}
	return;
}
void HP_login_login_error(int fd, uint8 status) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_login_error_pre ) {
		void (*preHookFunc) (int *fd, uint8 *status);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_login_error_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_login_error_pre[hIndex].func;
			preHookFunc(&fd, &status);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.login_error(fd, status);
	}
	if( HPMHooks.count.HP_login_login_error_post ) {
		void (*postHookFunc) (int *fd, uint8 *status);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_login_error_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_login_error_post[hIndex].func;
			postHookFunc(&fd, &status);
		}
	}
	return;
}
void HP_login_parse_ping(int fd, struct login_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_parse_ping_pre ) {
		void (*preHookFunc) (int *fd, struct login_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_parse_ping_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_parse_ping_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.parse_ping(fd, sd);
	}
	if( HPMHooks.count.HP_login_parse_ping_post ) {
		void (*postHookFunc) (int *fd, struct login_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_parse_ping_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_parse_ping_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
void HP_login_parse_client_md5(int fd, struct login_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_parse_client_md5_pre ) {
		void (*preHookFunc) (int *fd, struct login_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_parse_client_md5_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_parse_client_md5_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.parse_client_md5(fd, sd);
	}
	if( HPMHooks.count.HP_login_parse_client_md5_post ) {
		void (*postHookFunc) (int *fd, struct login_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_parse_client_md5_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_parse_client_md5_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
bool HP_login_parse_client_login(int fd, struct login_session_data *sd, const char *ip) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_login_parse_client_login_pre ) {
		bool (*preHookFunc) (int *fd, struct login_session_data *sd, const char *ip);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_parse_client_login_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_parse_client_login_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, sd, ip);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.login.parse_client_login(fd, sd, ip);
	}
	if( HPMHooks.count.HP_login_parse_client_login_post ) {
		bool (*postHookFunc) (bool retVal___, int *fd, struct login_session_data *sd, const char *ip);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_parse_client_login_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_parse_client_login_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, sd, ip);
		}
	}
	return retVal___;
}
void HP_login_send_coding_key(int fd, struct login_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_send_coding_key_pre ) {
		void (*preHookFunc) (int *fd, struct login_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_send_coding_key_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_send_coding_key_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.send_coding_key(fd, sd);
	}
	if( HPMHooks.count.HP_login_send_coding_key_post ) {
		void (*postHookFunc) (int *fd, struct login_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_send_coding_key_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_send_coding_key_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
void HP_login_parse_request_coding_key(int fd, struct login_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_parse_request_coding_key_pre ) {
		void (*preHookFunc) (int *fd, struct login_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_parse_request_coding_key_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_parse_request_coding_key_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.parse_request_coding_key(fd, sd);
	}
	if( HPMHooks.count.HP_login_parse_request_coding_key_post ) {
		void (*postHookFunc) (int *fd, struct login_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_parse_request_coding_key_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_parse_request_coding_key_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
void HP_login_char_server_connection_status(int fd, struct login_session_data *sd, uint8 status) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_char_server_connection_status_pre ) {
		void (*preHookFunc) (int *fd, struct login_session_data *sd, uint8 *status);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_char_server_connection_status_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_char_server_connection_status_pre[hIndex].func;
			preHookFunc(&fd, sd, &status);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.char_server_connection_status(fd, sd, status);
	}
	if( HPMHooks.count.HP_login_char_server_connection_status_post ) {
		void (*postHookFunc) (int *fd, struct login_session_data *sd, uint8 *status);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_char_server_connection_status_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_char_server_connection_status_post[hIndex].func;
			postHookFunc(&fd, sd, &status);
		}
	}
	return;
}
void HP_login_parse_request_connection(int fd, struct login_session_data *sd, const char *ip, uint32 ipl) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_parse_request_connection_pre ) {
		void (*preHookFunc) (int *fd, struct login_session_data *sd, const char *ip, uint32 *ipl);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_parse_request_connection_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_parse_request_connection_pre[hIndex].func;
			preHookFunc(&fd, sd, ip, &ipl);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.parse_request_connection(fd, sd, ip, ipl);
	}
	if( HPMHooks.count.HP_login_parse_request_connection_post ) {
		void (*postHookFunc) (int *fd, struct login_session_data *sd, const char *ip, uint32 *ipl);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_parse_request_connection_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_parse_request_connection_post[hIndex].func;
			postHookFunc(&fd, sd, ip, &ipl);
		}
	}
	return;
}
int HP_login_parse_login(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_login_parse_login_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_parse_login_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_parse_login_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.login.parse_login(fd);
	}
	if( HPMHooks.count.HP_login_parse_login_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_parse_login_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_parse_login_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
void HP_login_config_set_defaults(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_config_set_defaults_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_config_set_defaults_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_config_set_defaults_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.config_set_defaults();
	}
	if( HPMHooks.count.HP_login_config_set_defaults_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_config_set_defaults_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_config_set_defaults_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_login_config_read(const char *cfgName) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_login_config_read_pre ) {
		int (*preHookFunc) (const char *cfgName);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_config_read_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_config_read_pre[hIndex].func;
			retVal___ = preHookFunc(cfgName);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.login.config_read(cfgName);
	}
	if( HPMHooks.count.HP_login_config_read_post ) {
		int (*postHookFunc) (int retVal___, const char *cfgName);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_config_read_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_config_read_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, cfgName);
		}
	}
	return retVal___;
}
/* malloc_interface */
void HP_iMalloc_init(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_iMalloc_init_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_iMalloc_init_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.iMalloc.init();
	}
	if( HPMHooks.count.HP_iMalloc_init_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_iMalloc_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_iMalloc_final(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_iMalloc_final_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_final_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_iMalloc_final_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.iMalloc.final();
	}
	if( HPMHooks.count.HP_iMalloc_final_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_final_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_iMalloc_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void* HP_iMalloc_malloc(size_t size, const char *file, int line, const char *func) {
	int hIndex = 0;
	void* retVal___ = NULL;
	if( HPMHooks.count.HP_iMalloc_malloc_pre ) {
		void* (*preHookFunc) (size_t *size, const char *file, int *line, const char *func);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_malloc_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_iMalloc_malloc_pre[hIndex].func;
			retVal___ = preHookFunc(&size, file, &line, func);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.iMalloc.malloc(size, file, line, func);
	}
	if( HPMHooks.count.HP_iMalloc_malloc_post ) {
		void* (*postHookFunc) (void* retVal___, size_t *size, const char *file, int *line, const char *func);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_malloc_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_iMalloc_malloc_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &size, file, &line, func);
		}
	}
	return retVal___;
}
void* HP_iMalloc_calloc(size_t num, size_t size, const char *file, int line, const char *func) {
	int hIndex = 0;
	void* retVal___ = NULL;
	if( HPMHooks.count.HP_iMalloc_calloc_pre ) {
		void* (*preHookFunc) (size_t *num, size_t *size, const char *file, int *line, const char *func);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_calloc_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_iMalloc_calloc_pre[hIndex].func;
			retVal___ = preHookFunc(&num, &size, file, &line, func);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.iMalloc.calloc(num, size, file, line, func);
	}
	if( HPMHooks.count.HP_iMalloc_calloc_post ) {
		void* (*postHookFunc) (void* retVal___, size_t *num, size_t *size, const char *file, int *line, const char *func);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_calloc_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_iMalloc_calloc_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &num, &size, file, &line, func);
		}
	}
	return retVal___;
}
void* HP_iMalloc_realloc(void *p, size_t size, const char *file, int line, const char *func) {
	int hIndex = 0;
	void* retVal___ = NULL;
	if( HPMHooks.count.HP_iMalloc_realloc_pre ) {
		void* (*preHookFunc) (void *p, size_t *size, const char *file, int *line, const char *func);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_realloc_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_iMalloc_realloc_pre[hIndex].func;
			retVal___ = preHookFunc(p, &size, file, &line, func);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.iMalloc.realloc(p, size, file, line, func);
	}
	if( HPMHooks.count.HP_iMalloc_realloc_post ) {
		void* (*postHookFunc) (void* retVal___, void *p, size_t *size, const char *file, int *line, const char *func);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_realloc_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_iMalloc_realloc_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, p, &size, file, &line, func);
		}
	}
	return retVal___;
}
void* HP_iMalloc_reallocz(void *p, size_t size, const char *file, int line, const char *func) {
	int hIndex = 0;
	void* retVal___ = NULL;
	if( HPMHooks.count.HP_iMalloc_reallocz_pre ) {
		void* (*preHookFunc) (void *p, size_t *size, const char *file, int *line, const char *func);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_reallocz_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_iMalloc_reallocz_pre[hIndex].func;
			retVal___ = preHookFunc(p, &size, file, &line, func);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.iMalloc.reallocz(p, size, file, line, func);
	}
	if( HPMHooks.count.HP_iMalloc_reallocz_post ) {
		void* (*postHookFunc) (void* retVal___, void *p, size_t *size, const char *file, int *line, const char *func);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_reallocz_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_iMalloc_reallocz_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, p, &size, file, &line, func);
		}
	}
	return retVal___;
}
char* HP_iMalloc_astrdup(const char *p, const char *file, int line, const char *func) {
	int hIndex = 0;
	char* retVal___ = NULL;
	if( HPMHooks.count.HP_iMalloc_astrdup_pre ) {
		char* (*preHookFunc) (const char *p, const char *file, int *line, const char *func);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_astrdup_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_iMalloc_astrdup_pre[hIndex].func;
			retVal___ = preHookFunc(p, file, &line, func);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.iMalloc.astrdup(p, file, line, func);
	}
	if( HPMHooks.count.HP_iMalloc_astrdup_post ) {
		char* (*postHookFunc) (char* retVal___, const char *p, const char *file, int *line, const char *func);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_astrdup_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_iMalloc_astrdup_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, p, file, &line, func);
		}
	}
	return retVal___;
}
char* HP_iMalloc_astrndup(const char *p, size_t size, const char *file, int line, const char *func) {
	int hIndex = 0;
	char* retVal___ = NULL;
	if( HPMHooks.count.HP_iMalloc_astrndup_pre ) {
		char* (*preHookFunc) (const char *p, size_t *size, const char *file, int *line, const char *func);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_astrndup_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_iMalloc_astrndup_pre[hIndex].func;
			retVal___ = preHookFunc(p, &size, file, &line, func);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.iMalloc.astrndup(p, size, file, line, func);
	}
	if( HPMHooks.count.HP_iMalloc_astrndup_post ) {
		char* (*postHookFunc) (char* retVal___, const char *p, size_t *size, const char *file, int *line, const char *func);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_astrndup_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_iMalloc_astrndup_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, p, &size, file, &line, func);
		}
	}
	return retVal___;
}
void HP_iMalloc_free(void *p, const char *file, int line, const char *func) {
	int hIndex = 0;
	if( HPMHooks.count.HP_iMalloc_free_pre ) {
		void (*preHookFunc) (void *p, const char *file, int *line, const char *func);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_free_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_iMalloc_free_pre[hIndex].func;
			preHookFunc(p, file, &line, func);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.iMalloc.free(p, file, line, func);
	}
	if( HPMHooks.count.HP_iMalloc_free_post ) {
		void (*postHookFunc) (void *p, const char *file, int *line, const char *func);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_free_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_iMalloc_free_post[hIndex].func;
			postHookFunc(p, file, &line, func);
		}
	}
	return;
}
void HP_iMalloc_memory_check(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_iMalloc_memory_check_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_memory_check_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_iMalloc_memory_check_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.iMalloc.memory_check();
	}
	if( HPMHooks.count.HP_iMalloc_memory_check_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_memory_check_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_iMalloc_memory_check_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
bool HP_iMalloc_verify_ptr(void *ptr) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_iMalloc_verify_ptr_pre ) {
		bool (*preHookFunc) (void *ptr);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_verify_ptr_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_iMalloc_verify_ptr_pre[hIndex].func;
			retVal___ = preHookFunc(ptr);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.iMalloc.verify_ptr(ptr);
	}
	if( HPMHooks.count.HP_iMalloc_verify_ptr_post ) {
		bool (*postHookFunc) (bool retVal___, void *ptr);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_verify_ptr_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_iMalloc_verify_ptr_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, ptr);
		}
	}
	return retVal___;
}
size_t HP_iMalloc_usage(void) {
	int hIndex = 0;
	size_t retVal___ = 0;
	if( HPMHooks.count.HP_iMalloc_usage_pre ) {
		size_t (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_usage_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_iMalloc_usage_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.iMalloc.usage();
	}
	if( HPMHooks.count.HP_iMalloc_usage_post ) {
		size_t (*postHookFunc) (size_t retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_usage_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_iMalloc_usage_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_iMalloc_post_shutdown(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_iMalloc_post_shutdown_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_post_shutdown_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_iMalloc_post_shutdown_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.iMalloc.post_shutdown();
	}
	if( HPMHooks.count.HP_iMalloc_post_shutdown_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_post_shutdown_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_iMalloc_post_shutdown_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_iMalloc_init_messages(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_iMalloc_init_messages_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_init_messages_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_iMalloc_init_messages_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.iMalloc.init_messages();
	}
	if( HPMHooks.count.HP_iMalloc_init_messages_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_iMalloc_init_messages_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_iMalloc_init_messages_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
/* nullpo_interface */
void HP_nullpo_assert_report(const char *file, int line, const char *func, const char *targetname, const char *title) {
	int hIndex = 0;
	if( HPMHooks.count.HP_nullpo_assert_report_pre ) {
		void (*preHookFunc) (const char *file, int *line, const char *func, const char *targetname, const char *title);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_nullpo_assert_report_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_nullpo_assert_report_pre[hIndex].func;
			preHookFunc(file, &line, func, targetname, title);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.nullpo.assert_report(file, line, func, targetname, title);
	}
	if( HPMHooks.count.HP_nullpo_assert_report_post ) {
		void (*postHookFunc) (const char *file, int *line, const char *func, const char *targetname, const char *title);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_nullpo_assert_report_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_nullpo_assert_report_post[hIndex].func;
			postHookFunc(file, &line, func, targetname, title);
		}
	}
	return;
}
/* showmsg_interface */
void HP_showmsg_init(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_showmsg_init_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_showmsg_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_showmsg_init_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.showmsg.init();
	}
	if( HPMHooks.count.HP_showmsg_init_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_showmsg_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_showmsg_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_showmsg_final(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_showmsg_final_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_showmsg_final_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_showmsg_final_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.showmsg.final();
	}
	if( HPMHooks.count.HP_showmsg_final_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_showmsg_final_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_showmsg_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_showmsg_clearScreen(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_showmsg_clearScreen_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_showmsg_clearScreen_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_showmsg_clearScreen_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.showmsg.clearScreen();
	}
	if( HPMHooks.count.HP_showmsg_clearScreen_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_showmsg_clearScreen_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_showmsg_clearScreen_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_showmsg_showMessageV(const char *string, va_list ap) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_showmsg_showMessageV_pre ) {
		int (*preHookFunc) (const char *string, va_list ap);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_showmsg_showMessageV_pre; hIndex++ ) {
			va_list ap___copy; va_copy(ap___copy, ap);
			preHookFunc = HPMHooks.list.HP_showmsg_showMessageV_pre[hIndex].func;
			retVal___ = preHookFunc(string, ap___copy);
			va_end(ap___copy);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		va_list ap___copy; va_copy(ap___copy, ap);
		retVal___ = HPMHooks.source.showmsg.showMessageV(string, ap___copy);
		va_end(ap___copy);
	}
	if( HPMHooks.count.HP_showmsg_showMessageV_post ) {
		int (*postHookFunc) (int retVal___, const char *string, va_list ap);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_showmsg_showMessageV_post; hIndex++ ) {
			va_list ap___copy; va_copy(ap___copy, ap);
			postHookFunc = HPMHooks.list.HP_showmsg_showMessageV_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, string, ap___copy);
			va_end(ap___copy);
		}
	}
	return retVal___;
}
/* socket_interface */
void HP_sockt_init(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_sockt_init_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_init_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.init();
	}
	if( HPMHooks.count.HP_sockt_init_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_sockt_final(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_sockt_final_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_final_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_final_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.final();
	}
	if( HPMHooks.count.HP_sockt_final_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_final_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_sockt_perform(int next) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_sockt_perform_pre ) {
		int (*preHookFunc) (int *next);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_perform_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_perform_pre[hIndex].func;
			retVal___ = preHookFunc(&next);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.perform(next);
	}
	if( HPMHooks.count.HP_sockt_perform_post ) {
		int (*postHookFunc) (int retVal___, int *next);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_perform_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_perform_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &next);
		}
	}
	return retVal___;
}
void HP_sockt_datasync(int fd, bool send) {
	int hIndex = 0;
	if( HPMHooks.count.HP_sockt_datasync_pre ) {
		void (*preHookFunc) (int *fd, bool *send);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_datasync_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_datasync_pre[hIndex].func;
			preHookFunc(&fd, &send);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.datasync(fd, send);
	}
	if( HPMHooks.count.HP_sockt_datasync_post ) {
		void (*postHookFunc) (int *fd, bool *send);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_datasync_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_datasync_post[hIndex].func;
			postHookFunc(&fd, &send);
		}
	}
	return;
}
int HP_sockt_make_listen_bind(uint32 ip, uint16 port) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_sockt_make_listen_bind_pre ) {
		int (*preHookFunc) (uint32 *ip, uint16 *port);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_make_listen_bind_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_make_listen_bind_pre[hIndex].func;
			retVal___ = preHookFunc(&ip, &port);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.make_listen_bind(ip, port);
	}
	if( HPMHooks.count.HP_sockt_make_listen_bind_post ) {
		int (*postHookFunc) (int retVal___, uint32 *ip, uint16 *port);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_make_listen_bind_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_make_listen_bind_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &ip, &port);
		}
	}
	return retVal___;
}
int HP_sockt_make_connection(uint32 ip, uint16 port, struct hSockOpt *opt) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_sockt_make_connection_pre ) {
		int (*preHookFunc) (uint32 *ip, uint16 *port, struct hSockOpt *opt);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_make_connection_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_make_connection_pre[hIndex].func;
			retVal___ = preHookFunc(&ip, &port, opt);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.make_connection(ip, port, opt);
	}
	if( HPMHooks.count.HP_sockt_make_connection_post ) {
		int (*postHookFunc) (int retVal___, uint32 *ip, uint16 *port, struct hSockOpt *opt);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_make_connection_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_make_connection_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &ip, &port, opt);
		}
	}
	return retVal___;
}
int HP_sockt_realloc_fifo(int fd, unsigned int rfifo_size, unsigned int wfifo_size) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_sockt_realloc_fifo_pre ) {
		int (*preHookFunc) (int *fd, unsigned int *rfifo_size, unsigned int *wfifo_size);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_realloc_fifo_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_realloc_fifo_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &rfifo_size, &wfifo_size);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.realloc_fifo(fd, rfifo_size, wfifo_size);
	}
	if( HPMHooks.count.HP_sockt_realloc_fifo_post ) {
		int (*postHookFunc) (int retVal___, int *fd, unsigned int *rfifo_size, unsigned int *wfifo_size);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_realloc_fifo_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_realloc_fifo_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &rfifo_size, &wfifo_size);
		}
	}
	return retVal___;
}
int HP_sockt_realloc_writefifo(int fd, size_t addition) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_sockt_realloc_writefifo_pre ) {
		int (*preHookFunc) (int *fd, size_t *addition);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_realloc_writefifo_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_realloc_writefifo_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &addition);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.realloc_writefifo(fd, addition);
	}
	if( HPMHooks.count.HP_sockt_realloc_writefifo_post ) {
		int (*postHookFunc) (int retVal___, int *fd, size_t *addition);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_realloc_writefifo_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_realloc_writefifo_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &addition);
		}
	}
	return retVal___;
}
int HP_sockt_wfifoset(int fd, size_t len) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_sockt_wfifoset_pre ) {
		int (*preHookFunc) (int *fd, size_t *len);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_wfifoset_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_wfifoset_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &len);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.wfifoset(fd, len);
	}
	if( HPMHooks.count.HP_sockt_wfifoset_post ) {
		int (*postHookFunc) (int retVal___, int *fd, size_t *len);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_wfifoset_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_wfifoset_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &len);
		}
	}
	return retVal___;
}
int HP_sockt_rfifoskip(int fd, size_t len) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_sockt_rfifoskip_pre ) {
		int (*preHookFunc) (int *fd, size_t *len);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_rfifoskip_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_rfifoskip_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &len);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.rfifoskip(fd, len);
	}
	if( HPMHooks.count.HP_sockt_rfifoskip_post ) {
		int (*postHookFunc) (int retVal___, int *fd, size_t *len);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_rfifoskip_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_rfifoskip_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &len);
		}
	}
	return retVal___;
}
void HP_sockt_close(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_sockt_close_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_close_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_close_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.close(fd);
	}
	if( HPMHooks.count.HP_sockt_close_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_close_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_close_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
bool HP_sockt_session_is_valid(int fd) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_sockt_session_is_valid_pre ) {
		bool (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_session_is_valid_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_session_is_valid_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.session_is_valid(fd);
	}
	if( HPMHooks.count.HP_sockt_session_is_valid_post ) {
		bool (*postHookFunc) (bool retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_session_is_valid_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_session_is_valid_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
bool HP_sockt_session_is_active(int fd) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_sockt_session_is_active_pre ) {
		bool (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_session_is_active_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_session_is_active_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.session_is_active(fd);
	}
	if( HPMHooks.count.HP_sockt_session_is_active_post ) {
		bool (*postHookFunc) (bool retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_session_is_active_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_session_is_active_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
void HP_sockt_flush(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_sockt_flush_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_flush_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_flush_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.flush(fd);
	}
	if( HPMHooks.count.HP_sockt_flush_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_flush_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_flush_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_sockt_flush_fifos(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_sockt_flush_fifos_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_flush_fifos_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_flush_fifos_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.flush_fifos();
	}
	if( HPMHooks.count.HP_sockt_flush_fifos_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_flush_fifos_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_flush_fifos_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_sockt_set_nonblocking(int fd, unsigned long yes) {
	int hIndex = 0;
	if( HPMHooks.count.HP_sockt_set_nonblocking_pre ) {
		void (*preHookFunc) (int *fd, unsigned long *yes);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_set_nonblocking_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_set_nonblocking_pre[hIndex].func;
			preHookFunc(&fd, &yes);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.set_nonblocking(fd, yes);
	}
	if( HPMHooks.count.HP_sockt_set_nonblocking_post ) {
		void (*postHookFunc) (int *fd, unsigned long *yes);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_set_nonblocking_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_set_nonblocking_post[hIndex].func;
			postHookFunc(&fd, &yes);
		}
	}
	return;
}
void HP_sockt_set_defaultparse(ParseFunc defaultparse) {
	int hIndex = 0;
	if( HPMHooks.count.HP_sockt_set_defaultparse_pre ) {
		void (*preHookFunc) (ParseFunc *defaultparse);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_set_defaultparse_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_set_defaultparse_pre[hIndex].func;
			preHookFunc(&defaultparse);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.set_defaultparse(defaultparse);
	}
	if( HPMHooks.count.HP_sockt_set_defaultparse_post ) {
		void (*postHookFunc) (ParseFunc *defaultparse);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_set_defaultparse_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_set_defaultparse_post[hIndex].func;
			postHookFunc(&defaultparse);
		}
	}
	return;
}
uint32 HP_sockt_host2ip(const char *hostname) {
	int hIndex = 0;
	uint32 retVal___ = 0;
	if( HPMHooks.count.HP_sockt_host2ip_pre ) {
		uint32 (*preHookFunc) (const char *hostname);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_host2ip_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_host2ip_pre[hIndex].func;
			retVal___ = preHookFunc(hostname);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.host2ip(hostname);
	}
	if( HPMHooks.count.HP_sockt_host2ip_post ) {
		uint32 (*postHookFunc) (uint32 retVal___, const char *hostname);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_host2ip_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_host2ip_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, hostname);
		}
	}
	return retVal___;
}
const char* HP_sockt_ip2str(uint32 ip, char *ip_str) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if( HPMHooks.count.HP_sockt_ip2str_pre ) {
		const char* (*preHookFunc) (uint32 *ip, char *ip_str);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_ip2str_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_ip2str_pre[hIndex].func;
			retVal___ = preHookFunc(&ip, ip_str);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.ip2str(ip, ip_str);
	}
	if( HPMHooks.count.HP_sockt_ip2str_post ) {
		const char* (*postHookFunc) (const char* retVal___, uint32 *ip, char *ip_str);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_ip2str_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_ip2str_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &ip, ip_str);
		}
	}
	return retVal___;
}
uint32 HP_sockt_str2ip(const char *ip_str) {
	int hIndex = 0;
	uint32 retVal___ = 0;
	if( HPMHooks.count.HP_sockt_str2ip_pre ) {
		uint32 (*preHookFunc) (const char *ip_str);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_str2ip_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_str2ip_pre[hIndex].func;
			retVal___ = preHookFunc(ip_str);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.str2ip(ip_str);
	}
	if( HPMHooks.count.HP_sockt_str2ip_post ) {
		uint32 (*postHookFunc) (uint32 retVal___, const char *ip_str);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_str2ip_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_str2ip_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, ip_str);
		}
	}
	return retVal___;
}
uint16 HP_sockt_ntows(uint16 netshort) {
	int hIndex = 0;
	uint16 retVal___ = 0;
	if( HPMHooks.count.HP_sockt_ntows_pre ) {
		uint16 (*preHookFunc) (uint16 *netshort);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_ntows_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_ntows_pre[hIndex].func;
			retVal___ = preHookFunc(&netshort);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.ntows(netshort);
	}
	if( HPMHooks.count.HP_sockt_ntows_post ) {
		uint16 (*postHookFunc) (uint16 retVal___, uint16 *netshort);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_ntows_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_ntows_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &netshort);
		}
	}
	return retVal___;
}
int HP_sockt_getips(uint32 *ips, int max) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_sockt_getips_pre ) {
		int (*preHookFunc) (uint32 *ips, int *max);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_getips_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_getips_pre[hIndex].func;
			retVal___ = preHookFunc(ips, &max);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.getips(ips, max);
	}
	if( HPMHooks.count.HP_sockt_getips_post ) {
		int (*postHookFunc) (int retVal___, uint32 *ips, int *max);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_getips_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_getips_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, ips, &max);
		}
	}
	return retVal___;
}
void HP_sockt_eof(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_sockt_eof_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_eof_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_eof_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.eof(fd);
	}
	if( HPMHooks.count.HP_sockt_eof_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_eof_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_eof_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
uint32 HP_sockt_lan_subnet_check(uint32 ip, struct s_subnet *info) {
	int hIndex = 0;
	uint32 retVal___ = 0;
	if( HPMHooks.count.HP_sockt_lan_subnet_check_pre ) {
		uint32 (*preHookFunc) (uint32 *ip, struct s_subnet *info);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_lan_subnet_check_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_lan_subnet_check_pre[hIndex].func;
			retVal___ = preHookFunc(&ip, info);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.lan_subnet_check(ip, info);
	}
	if( HPMHooks.count.HP_sockt_lan_subnet_check_post ) {
		uint32 (*postHookFunc) (uint32 retVal___, uint32 *ip, struct s_subnet *info);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_lan_subnet_check_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_lan_subnet_check_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &ip, info);
		}
	}
	return retVal___;
}
bool HP_sockt_allowed_ip_check(uint32 ip) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_sockt_allowed_ip_check_pre ) {
		bool (*preHookFunc) (uint32 *ip);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_allowed_ip_check_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_allowed_ip_check_pre[hIndex].func;
			retVal___ = preHookFunc(&ip);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.allowed_ip_check(ip);
	}
	if( HPMHooks.count.HP_sockt_allowed_ip_check_post ) {
		bool (*postHookFunc) (bool retVal___, uint32 *ip);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_allowed_ip_check_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_allowed_ip_check_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &ip);
		}
	}
	return retVal___;
}
bool HP_sockt_trusted_ip_check(uint32 ip) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_sockt_trusted_ip_check_pre ) {
		bool (*preHookFunc) (uint32 *ip);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_trusted_ip_check_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_trusted_ip_check_pre[hIndex].func;
			retVal___ = preHookFunc(&ip);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.trusted_ip_check(ip);
	}
	if( HPMHooks.count.HP_sockt_trusted_ip_check_post ) {
		bool (*postHookFunc) (bool retVal___, uint32 *ip);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_trusted_ip_check_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_trusted_ip_check_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &ip);
		}
	}
	return retVal___;
}
int HP_sockt_net_config_read_sub(struct config_setting_t *t, struct s_subnet_vector *list, const char *filename, const char *groupname) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_sockt_net_config_read_sub_pre ) {
		int (*preHookFunc) (struct config_setting_t *t, struct s_subnet_vector *list, const char *filename, const char *groupname);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_net_config_read_sub_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_net_config_read_sub_pre[hIndex].func;
			retVal___ = preHookFunc(t, list, filename, groupname);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.net_config_read_sub(t, list, filename, groupname);
	}
	if( HPMHooks.count.HP_sockt_net_config_read_sub_post ) {
		int (*postHookFunc) (int retVal___, struct config_setting_t *t, struct s_subnet_vector *list, const char *filename, const char *groupname);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_net_config_read_sub_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_net_config_read_sub_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, t, list, filename, groupname);
		}
	}
	return retVal___;
}
void HP_sockt_net_config_read(const char *filename) {
	int hIndex = 0;
	if( HPMHooks.count.HP_sockt_net_config_read_pre ) {
		void (*preHookFunc) (const char *filename);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_net_config_read_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sockt_net_config_read_pre[hIndex].func;
			preHookFunc(filename);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.net_config_read(filename);
	}
	if( HPMHooks.count.HP_sockt_net_config_read_post ) {
		void (*postHookFunc) (const char *filename);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sockt_net_config_read_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sockt_net_config_read_post[hIndex].func;
			postHookFunc(filename);
		}
	}
	return;
}
/* sql_interface */
int HP_SQL_Connect(struct Sql *self, const char *user, const char *passwd, const char *host, uint16 port, const char *db) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_SQL_Connect_pre ) {
		int (*preHookFunc) (struct Sql *self, const char *user, const char *passwd, const char *host, uint16 *port, const char *db);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_Connect_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_Connect_pre[hIndex].func;
			retVal___ = preHookFunc(self, user, passwd, host, &port, db);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.Connect(self, user, passwd, host, port, db);
	}
	if( HPMHooks.count.HP_SQL_Connect_post ) {
		int (*postHookFunc) (int retVal___, struct Sql *self, const char *user, const char *passwd, const char *host, uint16 *port, const char *db);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_Connect_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_Connect_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, user, passwd, host, &port, db);
		}
	}
	return retVal___;
}
int HP_SQL_GetTimeout(struct Sql *self, uint32 *out_timeout) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_SQL_GetTimeout_pre ) {
		int (*preHookFunc) (struct Sql *self, uint32 *out_timeout);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_GetTimeout_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_GetTimeout_pre[hIndex].func;
			retVal___ = preHookFunc(self, out_timeout);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.GetTimeout(self, out_timeout);
	}
	if( HPMHooks.count.HP_SQL_GetTimeout_post ) {
		int (*postHookFunc) (int retVal___, struct Sql *self, uint32 *out_timeout);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_GetTimeout_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_GetTimeout_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, out_timeout);
		}
	}
	return retVal___;
}
int HP_SQL_GetColumnNames(struct Sql *self, const char *table, char *out_buf, size_t buf_len, char sep) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_SQL_GetColumnNames_pre ) {
		int (*preHookFunc) (struct Sql *self, const char *table, char *out_buf, size_t *buf_len, char *sep);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_GetColumnNames_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_GetColumnNames_pre[hIndex].func;
			retVal___ = preHookFunc(self, table, out_buf, &buf_len, &sep);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.GetColumnNames(self, table, out_buf, buf_len, sep);
	}
	if( HPMHooks.count.HP_SQL_GetColumnNames_post ) {
		int (*postHookFunc) (int retVal___, struct Sql *self, const char *table, char *out_buf, size_t *buf_len, char *sep);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_GetColumnNames_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_GetColumnNames_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, table, out_buf, &buf_len, &sep);
		}
	}
	return retVal___;
}
int HP_SQL_SetEncoding(struct Sql *self, const char *encoding) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_SQL_SetEncoding_pre ) {
		int (*preHookFunc) (struct Sql *self, const char *encoding);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_SetEncoding_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_SetEncoding_pre[hIndex].func;
			retVal___ = preHookFunc(self, encoding);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.SetEncoding(self, encoding);
	}
	if( HPMHooks.count.HP_SQL_SetEncoding_post ) {
		int (*postHookFunc) (int retVal___, struct Sql *self, const char *encoding);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_SetEncoding_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_SetEncoding_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, encoding);
		}
	}
	return retVal___;
}
int HP_SQL_Ping(struct Sql *self) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_SQL_Ping_pre ) {
		int (*preHookFunc) (struct Sql *self);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_Ping_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_Ping_pre[hIndex].func;
			retVal___ = preHookFunc(self);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.Ping(self);
	}
	if( HPMHooks.count.HP_SQL_Ping_post ) {
		int (*postHookFunc) (int retVal___, struct Sql *self);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_Ping_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_Ping_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
size_t HP_SQL_EscapeString(struct Sql *self, char *out_to, const char *from) {
	int hIndex = 0;
	size_t retVal___ = 0;
	if( HPMHooks.count.HP_SQL_EscapeString_pre ) {
		size_t (*preHookFunc) (struct Sql *self, char *out_to, const char *from);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_EscapeString_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_EscapeString_pre[hIndex].func;
			retVal___ = preHookFunc(self, out_to, from);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.EscapeString(self, out_to, from);
	}
	if( HPMHooks.count.HP_SQL_EscapeString_post ) {
		size_t (*postHookFunc) (size_t retVal___, struct Sql *self, char *out_to, const char *from);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_EscapeString_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_EscapeString_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, out_to, from);
		}
	}
	return retVal___;
}
size_t HP_SQL_EscapeStringLen(struct Sql *self, char *out_to, const char *from, size_t from_len) {
	int hIndex = 0;
	size_t retVal___ = 0;
	if( HPMHooks.count.HP_SQL_EscapeStringLen_pre ) {
		size_t (*preHookFunc) (struct Sql *self, char *out_to, const char *from, size_t *from_len);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_EscapeStringLen_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_EscapeStringLen_pre[hIndex].func;
			retVal___ = preHookFunc(self, out_to, from, &from_len);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.EscapeStringLen(self, out_to, from, from_len);
	}
	if( HPMHooks.count.HP_SQL_EscapeStringLen_post ) {
		size_t (*postHookFunc) (size_t retVal___, struct Sql *self, char *out_to, const char *from, size_t *from_len);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_EscapeStringLen_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_EscapeStringLen_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, out_to, from, &from_len);
		}
	}
	return retVal___;
}
int HP_SQL_QueryV(struct Sql *self, const char *query, va_list args) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_SQL_QueryV_pre ) {
		int (*preHookFunc) (struct Sql *self, const char *query, va_list args);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_QueryV_pre; hIndex++ ) {
			va_list args___copy; va_copy(args___copy, args);
			preHookFunc = HPMHooks.list.HP_SQL_QueryV_pre[hIndex].func;
			retVal___ = preHookFunc(self, query, args___copy);
			va_end(args___copy);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		va_list args___copy; va_copy(args___copy, args);
		retVal___ = HPMHooks.source.SQL.QueryV(self, query, args___copy);
		va_end(args___copy);
	}
	if( HPMHooks.count.HP_SQL_QueryV_post ) {
		int (*postHookFunc) (int retVal___, struct Sql *self, const char *query, va_list args);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_QueryV_post; hIndex++ ) {
			va_list args___copy; va_copy(args___copy, args);
			postHookFunc = HPMHooks.list.HP_SQL_QueryV_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, query, args___copy);
			va_end(args___copy);
		}
	}
	return retVal___;
}
int HP_SQL_QueryStr(struct Sql *self, const char *query) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_SQL_QueryStr_pre ) {
		int (*preHookFunc) (struct Sql *self, const char *query);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_QueryStr_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_QueryStr_pre[hIndex].func;
			retVal___ = preHookFunc(self, query);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.QueryStr(self, query);
	}
	if( HPMHooks.count.HP_SQL_QueryStr_post ) {
		int (*postHookFunc) (int retVal___, struct Sql *self, const char *query);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_QueryStr_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_QueryStr_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, query);
		}
	}
	return retVal___;
}
uint64 HP_SQL_LastInsertId(struct Sql *self) {
	int hIndex = 0;
	uint64 retVal___ = 0;
	if( HPMHooks.count.HP_SQL_LastInsertId_pre ) {
		uint64 (*preHookFunc) (struct Sql *self);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_LastInsertId_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_LastInsertId_pre[hIndex].func;
			retVal___ = preHookFunc(self);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.LastInsertId(self);
	}
	if( HPMHooks.count.HP_SQL_LastInsertId_post ) {
		uint64 (*postHookFunc) (uint64 retVal___, struct Sql *self);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_LastInsertId_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_LastInsertId_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
uint32 HP_SQL_NumColumns(struct Sql *self) {
	int hIndex = 0;
	uint32 retVal___ = 0;
	if( HPMHooks.count.HP_SQL_NumColumns_pre ) {
		uint32 (*preHookFunc) (struct Sql *self);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_NumColumns_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_NumColumns_pre[hIndex].func;
			retVal___ = preHookFunc(self);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.NumColumns(self);
	}
	if( HPMHooks.count.HP_SQL_NumColumns_post ) {
		uint32 (*postHookFunc) (uint32 retVal___, struct Sql *self);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_NumColumns_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_NumColumns_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
uint64 HP_SQL_NumRows(struct Sql *self) {
	int hIndex = 0;
	uint64 retVal___ = 0;
	if( HPMHooks.count.HP_SQL_NumRows_pre ) {
		uint64 (*preHookFunc) (struct Sql *self);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_NumRows_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_NumRows_pre[hIndex].func;
			retVal___ = preHookFunc(self);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.NumRows(self);
	}
	if( HPMHooks.count.HP_SQL_NumRows_post ) {
		uint64 (*postHookFunc) (uint64 retVal___, struct Sql *self);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_NumRows_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_NumRows_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
int HP_SQL_NextRow(struct Sql *self) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_SQL_NextRow_pre ) {
		int (*preHookFunc) (struct Sql *self);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_NextRow_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_NextRow_pre[hIndex].func;
			retVal___ = preHookFunc(self);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.NextRow(self);
	}
	if( HPMHooks.count.HP_SQL_NextRow_post ) {
		int (*postHookFunc) (int retVal___, struct Sql *self);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_NextRow_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_NextRow_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
int HP_SQL_GetData(struct Sql *self, size_t col, char **out_buf, size_t *out_len) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_SQL_GetData_pre ) {
		int (*preHookFunc) (struct Sql *self, size_t *col, char **out_buf, size_t *out_len);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_GetData_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_GetData_pre[hIndex].func;
			retVal___ = preHookFunc(self, &col, out_buf, out_len);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.GetData(self, col, out_buf, out_len);
	}
	if( HPMHooks.count.HP_SQL_GetData_post ) {
		int (*postHookFunc) (int retVal___, struct Sql *self, size_t *col, char **out_buf, size_t *out_len);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_GetData_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_GetData_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, &col, out_buf, out_len);
		}
	}
	return retVal___;
}
void HP_SQL_FreeResult(struct Sql *self) {
	int hIndex = 0;
	if( HPMHooks.count.HP_SQL_FreeResult_pre ) {
		void (*preHookFunc) (struct Sql *self);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_FreeResult_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_FreeResult_pre[hIndex].func;
			preHookFunc(self);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.SQL.FreeResult(self);
	}
	if( HPMHooks.count.HP_SQL_FreeResult_post ) {
		void (*postHookFunc) (struct Sql *self);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_FreeResult_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_FreeResult_post[hIndex].func;
			postHookFunc(self);
		}
	}
	return;
}
void HP_SQL_ShowDebug_(struct Sql *self, const char *debug_file, const unsigned long debug_line) {
	int hIndex = 0;
	if( HPMHooks.count.HP_SQL_ShowDebug__pre ) {
		void (*preHookFunc) (struct Sql *self, const char *debug_file, const unsigned long *debug_line);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_ShowDebug__pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_ShowDebug__pre[hIndex].func;
			preHookFunc(self, debug_file, &debug_line);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.SQL.ShowDebug_(self, debug_file, debug_line);
	}
	if( HPMHooks.count.HP_SQL_ShowDebug__post ) {
		void (*postHookFunc) (struct Sql *self, const char *debug_file, const unsigned long *debug_line);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_ShowDebug__post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_ShowDebug__post[hIndex].func;
			postHookFunc(self, debug_file, &debug_line);
		}
	}
	return;
}
void HP_SQL_Free(struct Sql *self) {
	int hIndex = 0;
	if( HPMHooks.count.HP_SQL_Free_pre ) {
		void (*preHookFunc) (struct Sql *self);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_Free_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_Free_pre[hIndex].func;
			preHookFunc(self);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.SQL.Free(self);
	}
	if( HPMHooks.count.HP_SQL_Free_post ) {
		void (*postHookFunc) (struct Sql *self);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_Free_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_Free_post[hIndex].func;
			postHookFunc(self);
		}
	}
	return;
}
struct Sql* HP_SQL_Malloc(void) {
	int hIndex = 0;
	struct Sql* retVal___ = NULL;
	if( HPMHooks.count.HP_SQL_Malloc_pre ) {
		struct Sql* (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_Malloc_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_Malloc_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.Malloc();
	}
	if( HPMHooks.count.HP_SQL_Malloc_post ) {
		struct Sql* (*postHookFunc) (struct Sql* retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_Malloc_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_Malloc_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
struct SqlStmt* HP_SQL_StmtMalloc(struct Sql *sql) {
	int hIndex = 0;
	struct SqlStmt* retVal___ = NULL;
	if( HPMHooks.count.HP_SQL_StmtMalloc_pre ) {
		struct SqlStmt* (*preHookFunc) (struct Sql *sql);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtMalloc_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtMalloc_pre[hIndex].func;
			retVal___ = preHookFunc(sql);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.StmtMalloc(sql);
	}
	if( HPMHooks.count.HP_SQL_StmtMalloc_post ) {
		struct SqlStmt* (*postHookFunc) (struct SqlStmt* retVal___, struct Sql *sql);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtMalloc_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtMalloc_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, sql);
		}
	}
	return retVal___;
}
int HP_SQL_StmtPrepareV(struct SqlStmt *self, const char *query, va_list args) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_SQL_StmtPrepareV_pre ) {
		int (*preHookFunc) (struct SqlStmt *self, const char *query, va_list args);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtPrepareV_pre; hIndex++ ) {
			va_list args___copy; va_copy(args___copy, args);
			preHookFunc = HPMHooks.list.HP_SQL_StmtPrepareV_pre[hIndex].func;
			retVal___ = preHookFunc(self, query, args___copy);
			va_end(args___copy);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		va_list args___copy; va_copy(args___copy, args);
		retVal___ = HPMHooks.source.SQL.StmtPrepareV(self, query, args___copy);
		va_end(args___copy);
	}
	if( HPMHooks.count.HP_SQL_StmtPrepareV_post ) {
		int (*postHookFunc) (int retVal___, struct SqlStmt *self, const char *query, va_list args);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtPrepareV_post; hIndex++ ) {
			va_list args___copy; va_copy(args___copy, args);
			postHookFunc = HPMHooks.list.HP_SQL_StmtPrepareV_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, query, args___copy);
			va_end(args___copy);
		}
	}
	return retVal___;
}
int HP_SQL_StmtPrepareStr(struct SqlStmt *self, const char *query) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_SQL_StmtPrepareStr_pre ) {
		int (*preHookFunc) (struct SqlStmt *self, const char *query);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtPrepareStr_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtPrepareStr_pre[hIndex].func;
			retVal___ = preHookFunc(self, query);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.StmtPrepareStr(self, query);
	}
	if( HPMHooks.count.HP_SQL_StmtPrepareStr_post ) {
		int (*postHookFunc) (int retVal___, struct SqlStmt *self, const char *query);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtPrepareStr_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtPrepareStr_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, query);
		}
	}
	return retVal___;
}
size_t HP_SQL_StmtNumParams(struct SqlStmt *self) {
	int hIndex = 0;
	size_t retVal___ = 0;
	if( HPMHooks.count.HP_SQL_StmtNumParams_pre ) {
		size_t (*preHookFunc) (struct SqlStmt *self);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtNumParams_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtNumParams_pre[hIndex].func;
			retVal___ = preHookFunc(self);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.StmtNumParams(self);
	}
	if( HPMHooks.count.HP_SQL_StmtNumParams_post ) {
		size_t (*postHookFunc) (size_t retVal___, struct SqlStmt *self);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtNumParams_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtNumParams_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
int HP_SQL_StmtBindParam(struct SqlStmt *self, size_t idx, enum SqlDataType buffer_type, const void *buffer, size_t buffer_len) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_SQL_StmtBindParam_pre ) {
		int (*preHookFunc) (struct SqlStmt *self, size_t *idx, enum SqlDataType *buffer_type, const void *buffer, size_t *buffer_len);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtBindParam_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtBindParam_pre[hIndex].func;
			retVal___ = preHookFunc(self, &idx, &buffer_type, buffer, &buffer_len);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.StmtBindParam(self, idx, buffer_type, buffer, buffer_len);
	}
	if( HPMHooks.count.HP_SQL_StmtBindParam_post ) {
		int (*postHookFunc) (int retVal___, struct SqlStmt *self, size_t *idx, enum SqlDataType *buffer_type, const void *buffer, size_t *buffer_len);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtBindParam_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtBindParam_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, &idx, &buffer_type, buffer, &buffer_len);
		}
	}
	return retVal___;
}
int HP_SQL_StmtExecute(struct SqlStmt *self) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_SQL_StmtExecute_pre ) {
		int (*preHookFunc) (struct SqlStmt *self);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtExecute_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtExecute_pre[hIndex].func;
			retVal___ = preHookFunc(self);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.StmtExecute(self);
	}
	if( HPMHooks.count.HP_SQL_StmtExecute_post ) {
		int (*postHookFunc) (int retVal___, struct SqlStmt *self);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtExecute_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtExecute_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
uint64 HP_SQL_StmtLastInsertId(struct SqlStmt *self) {
	int hIndex = 0;
	uint64 retVal___ = 0;
	if( HPMHooks.count.HP_SQL_StmtLastInsertId_pre ) {
		uint64 (*preHookFunc) (struct SqlStmt *self);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtLastInsertId_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtLastInsertId_pre[hIndex].func;
			retVal___ = preHookFunc(self);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.StmtLastInsertId(self);
	}
	if( HPMHooks.count.HP_SQL_StmtLastInsertId_post ) {
		uint64 (*postHookFunc) (uint64 retVal___, struct SqlStmt *self);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtLastInsertId_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtLastInsertId_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
size_t HP_SQL_StmtNumColumns(struct SqlStmt *self) {
	int hIndex = 0;
	size_t retVal___ = 0;
	if( HPMHooks.count.HP_SQL_StmtNumColumns_pre ) {
		size_t (*preHookFunc) (struct SqlStmt *self);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtNumColumns_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtNumColumns_pre[hIndex].func;
			retVal___ = preHookFunc(self);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.StmtNumColumns(self);
	}
	if( HPMHooks.count.HP_SQL_StmtNumColumns_post ) {
		size_t (*postHookFunc) (size_t retVal___, struct SqlStmt *self);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtNumColumns_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtNumColumns_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
int HP_SQL_StmtBindColumn(struct SqlStmt *self, size_t idx, enum SqlDataType buffer_type, void *buffer, size_t buffer_len, uint32 *out_length, int8 *out_is_null) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_SQL_StmtBindColumn_pre ) {
		int (*preHookFunc) (struct SqlStmt *self, size_t *idx, enum SqlDataType *buffer_type, void *buffer, size_t *buffer_len, uint32 *out_length, int8 *out_is_null);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtBindColumn_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtBindColumn_pre[hIndex].func;
			retVal___ = preHookFunc(self, &idx, &buffer_type, buffer, &buffer_len, out_length, out_is_null);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.StmtBindColumn(self, idx, buffer_type, buffer, buffer_len, out_length, out_is_null);
	}
	if( HPMHooks.count.HP_SQL_StmtBindColumn_post ) {
		int (*postHookFunc) (int retVal___, struct SqlStmt *self, size_t *idx, enum SqlDataType *buffer_type, void *buffer, size_t *buffer_len, uint32 *out_length, int8 *out_is_null);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtBindColumn_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtBindColumn_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, &idx, &buffer_type, buffer, &buffer_len, out_length, out_is_null);
		}
	}
	return retVal___;
}
uint64 HP_SQL_StmtNumRows(struct SqlStmt *self) {
	int hIndex = 0;
	uint64 retVal___ = 0;
	if( HPMHooks.count.HP_SQL_StmtNumRows_pre ) {
		uint64 (*preHookFunc) (struct SqlStmt *self);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtNumRows_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtNumRows_pre[hIndex].func;
			retVal___ = preHookFunc(self);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.StmtNumRows(self);
	}
	if( HPMHooks.count.HP_SQL_StmtNumRows_post ) {
		uint64 (*postHookFunc) (uint64 retVal___, struct SqlStmt *self);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtNumRows_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtNumRows_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
int HP_SQL_StmtNextRow(struct SqlStmt *self) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_SQL_StmtNextRow_pre ) {
		int (*preHookFunc) (struct SqlStmt *self);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtNextRow_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtNextRow_pre[hIndex].func;
			retVal___ = preHookFunc(self);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.StmtNextRow(self);
	}
	if( HPMHooks.count.HP_SQL_StmtNextRow_post ) {
		int (*postHookFunc) (int retVal___, struct SqlStmt *self);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtNextRow_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtNextRow_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
void HP_SQL_StmtFreeResult(struct SqlStmt *self) {
	int hIndex = 0;
	if( HPMHooks.count.HP_SQL_StmtFreeResult_pre ) {
		void (*preHookFunc) (struct SqlStmt *self);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtFreeResult_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtFreeResult_pre[hIndex].func;
			preHookFunc(self);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.SQL.StmtFreeResult(self);
	}
	if( HPMHooks.count.HP_SQL_StmtFreeResult_post ) {
		void (*postHookFunc) (struct SqlStmt *self);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtFreeResult_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtFreeResult_post[hIndex].func;
			postHookFunc(self);
		}
	}
	return;
}
void HP_SQL_StmtFree(struct SqlStmt *self) {
	int hIndex = 0;
	if( HPMHooks.count.HP_SQL_StmtFree_pre ) {
		void (*preHookFunc) (struct SqlStmt *self);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtFree_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtFree_pre[hIndex].func;
			preHookFunc(self);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.SQL.StmtFree(self);
	}
	if( HPMHooks.count.HP_SQL_StmtFree_post ) {
		void (*postHookFunc) (struct SqlStmt *self);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtFree_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtFree_post[hIndex].func;
			postHookFunc(self);
		}
	}
	return;
}
void HP_SQL_StmtShowDebug_(struct SqlStmt *self, const char *debug_file, const unsigned long debug_line) {
	int hIndex = 0;
	if( HPMHooks.count.HP_SQL_StmtShowDebug__pre ) {
		void (*preHookFunc) (struct SqlStmt *self, const char *debug_file, const unsigned long *debug_line);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtShowDebug__pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtShowDebug__pre[hIndex].func;
			preHookFunc(self, debug_file, &debug_line);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.SQL.StmtShowDebug_(self, debug_file, debug_line);
	}
	if( HPMHooks.count.HP_SQL_StmtShowDebug__post ) {
		void (*postHookFunc) (struct SqlStmt *self, const char *debug_file, const unsigned long *debug_line);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtShowDebug__post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtShowDebug__post[hIndex].func;
			postHookFunc(self, debug_file, &debug_line);
		}
	}
	return;
}
/* stringbuf_interface */
StringBuf* HP_StrBuf_Malloc(void) {
	int hIndex = 0;
	StringBuf* retVal___ = NULL;
	if( HPMHooks.count.HP_StrBuf_Malloc_pre ) {
		StringBuf* (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Malloc_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_StrBuf_Malloc_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.StrBuf.Malloc();
	}
	if( HPMHooks.count.HP_StrBuf_Malloc_post ) {
		StringBuf* (*postHookFunc) (StringBuf* retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Malloc_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_StrBuf_Malloc_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_StrBuf_Init(StringBuf *self) {
	int hIndex = 0;
	if( HPMHooks.count.HP_StrBuf_Init_pre ) {
		void (*preHookFunc) (StringBuf *self);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_StrBuf_Init_pre[hIndex].func;
			preHookFunc(self);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.StrBuf.Init(self);
	}
	if( HPMHooks.count.HP_StrBuf_Init_post ) {
		void (*postHookFunc) (StringBuf *self);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_StrBuf_Init_post[hIndex].func;
			postHookFunc(self);
		}
	}
	return;
}
int HP_StrBuf_Vprintf(StringBuf *self, const char *fmt, va_list args) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_StrBuf_Vprintf_pre ) {
		int (*preHookFunc) (StringBuf *self, const char *fmt, va_list args);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Vprintf_pre; hIndex++ ) {
			va_list args___copy; va_copy(args___copy, args);
			preHookFunc = HPMHooks.list.HP_StrBuf_Vprintf_pre[hIndex].func;
			retVal___ = preHookFunc(self, fmt, args___copy);
			va_end(args___copy);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		va_list args___copy; va_copy(args___copy, args);
		retVal___ = HPMHooks.source.StrBuf.Vprintf(self, fmt, args___copy);
		va_end(args___copy);
	}
	if( HPMHooks.count.HP_StrBuf_Vprintf_post ) {
		int (*postHookFunc) (int retVal___, StringBuf *self, const char *fmt, va_list args);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Vprintf_post; hIndex++ ) {
			va_list args___copy; va_copy(args___copy, args);
			postHookFunc = HPMHooks.list.HP_StrBuf_Vprintf_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, fmt, args___copy);
			va_end(args___copy);
		}
	}
	return retVal___;
}
int HP_StrBuf_Append(StringBuf *self, const StringBuf *sbuf) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_StrBuf_Append_pre ) {
		int (*preHookFunc) (StringBuf *self, const StringBuf *sbuf);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Append_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_StrBuf_Append_pre[hIndex].func;
			retVal___ = preHookFunc(self, sbuf);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.StrBuf.Append(self, sbuf);
	}
	if( HPMHooks.count.HP_StrBuf_Append_post ) {
		int (*postHookFunc) (int retVal___, StringBuf *self, const StringBuf *sbuf);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Append_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_StrBuf_Append_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, sbuf);
		}
	}
	return retVal___;
}
int HP_StrBuf_AppendStr(StringBuf *self, const char *str) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_StrBuf_AppendStr_pre ) {
		int (*preHookFunc) (StringBuf *self, const char *str);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_AppendStr_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_StrBuf_AppendStr_pre[hIndex].func;
			retVal___ = preHookFunc(self, str);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.StrBuf.AppendStr(self, str);
	}
	if( HPMHooks.count.HP_StrBuf_AppendStr_post ) {
		int (*postHookFunc) (int retVal___, StringBuf *self, const char *str);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_AppendStr_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_StrBuf_AppendStr_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, str);
		}
	}
	return retVal___;
}
int HP_StrBuf_Length(StringBuf *self) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_StrBuf_Length_pre ) {
		int (*preHookFunc) (StringBuf *self);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Length_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_StrBuf_Length_pre[hIndex].func;
			retVal___ = preHookFunc(self);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.StrBuf.Length(self);
	}
	if( HPMHooks.count.HP_StrBuf_Length_post ) {
		int (*postHookFunc) (int retVal___, StringBuf *self);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Length_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_StrBuf_Length_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
char* HP_StrBuf_Value(StringBuf *self) {
	int hIndex = 0;
	char* retVal___ = NULL;
	if( HPMHooks.count.HP_StrBuf_Value_pre ) {
		char* (*preHookFunc) (StringBuf *self);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Value_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_StrBuf_Value_pre[hIndex].func;
			retVal___ = preHookFunc(self);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.StrBuf.Value(self);
	}
	if( HPMHooks.count.HP_StrBuf_Value_post ) {
		char* (*postHookFunc) (char* retVal___, StringBuf *self);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Value_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_StrBuf_Value_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
void HP_StrBuf_Clear(StringBuf *self) {
	int hIndex = 0;
	if( HPMHooks.count.HP_StrBuf_Clear_pre ) {
		void (*preHookFunc) (StringBuf *self);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Clear_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_StrBuf_Clear_pre[hIndex].func;
			preHookFunc(self);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.StrBuf.Clear(self);
	}
	if( HPMHooks.count.HP_StrBuf_Clear_post ) {
		void (*postHookFunc) (StringBuf *self);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Clear_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_StrBuf_Clear_post[hIndex].func;
			postHookFunc(self);
		}
	}
	return;
}
void HP_StrBuf_Destroy(StringBuf *self) {
	int hIndex = 0;
	if( HPMHooks.count.HP_StrBuf_Destroy_pre ) {
		void (*preHookFunc) (StringBuf *self);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Destroy_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_StrBuf_Destroy_pre[hIndex].func;
			preHookFunc(self);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.StrBuf.Destroy(self);
	}
	if( HPMHooks.count.HP_StrBuf_Destroy_post ) {
		void (*postHookFunc) (StringBuf *self);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Destroy_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_StrBuf_Destroy_post[hIndex].func;
			postHookFunc(self);
		}
	}
	return;
}
void HP_StrBuf_Free(StringBuf *self) {
	int hIndex = 0;
	if( HPMHooks.count.HP_StrBuf_Free_pre ) {
		void (*preHookFunc) (StringBuf *self);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Free_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_StrBuf_Free_pre[hIndex].func;
			preHookFunc(self);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.StrBuf.Free(self);
	}
	if( HPMHooks.count.HP_StrBuf_Free_post ) {
		void (*postHookFunc) (StringBuf *self);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Free_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_StrBuf_Free_post[hIndex].func;
			postHookFunc(self);
		}
	}
	return;
}
/* strlib_interface */
char* HP_strlib_jstrescape(char *pt) {
	int hIndex = 0;
	char* retVal___ = NULL;
	if( HPMHooks.count.HP_strlib_jstrescape_pre ) {
		char* (*preHookFunc) (char *pt);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_jstrescape_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_strlib_jstrescape_pre[hIndex].func;
			retVal___ = preHookFunc(pt);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.jstrescape(pt);
	}
	if( HPMHooks.count.HP_strlib_jstrescape_post ) {
		char* (*postHookFunc) (char* retVal___, char *pt);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_jstrescape_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_strlib_jstrescape_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, pt);
		}
	}
	return retVal___;
}
char* HP_strlib_jstrescapecpy(char *pt, const char *spt) {
	int hIndex = 0;
	char* retVal___ = NULL;
	if( HPMHooks.count.HP_strlib_jstrescapecpy_pre ) {
		char* (*preHookFunc) (char *pt, const char *spt);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_jstrescapecpy_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_strlib_jstrescapecpy_pre[hIndex].func;
			retVal___ = preHookFunc(pt, spt);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.jstrescapecpy(pt, spt);
	}
	if( HPMHooks.count.HP_strlib_jstrescapecpy_post ) {
		char* (*postHookFunc) (char* retVal___, char *pt, const char *spt);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_jstrescapecpy_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_strlib_jstrescapecpy_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, pt, spt);
		}
	}
	return retVal___;
}
int HP_strlib_jmemescapecpy(char *pt, const char *spt, int size) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_strlib_jmemescapecpy_pre ) {
		int (*preHookFunc) (char *pt, const char *spt, int *size);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_jmemescapecpy_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_strlib_jmemescapecpy_pre[hIndex].func;
			retVal___ = preHookFunc(pt, spt, &size);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.jmemescapecpy(pt, spt, size);
	}
	if( HPMHooks.count.HP_strlib_jmemescapecpy_post ) {
		int (*postHookFunc) (int retVal___, char *pt, const char *spt, int *size);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_jmemescapecpy_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_strlib_jmemescapecpy_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, pt, spt, &size);
		}
	}
	return retVal___;
}
int HP_strlib_remove_control_chars_(char *str) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_strlib_remove_control_chars__pre ) {
		int (*preHookFunc) (char *str);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_remove_control_chars__pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_strlib_remove_control_chars__pre[hIndex].func;
			retVal___ = preHookFunc(str);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.remove_control_chars_(str);
	}
	if( HPMHooks.count.HP_strlib_remove_control_chars__post ) {
		int (*postHookFunc) (int retVal___, char *str);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_remove_control_chars__post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_strlib_remove_control_chars__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, str);
		}
	}
	return retVal___;
}
char* HP_strlib_trim_(char *str) {
	int hIndex = 0;
	char* retVal___ = NULL;
	if( HPMHooks.count.HP_strlib_trim__pre ) {
		char* (*preHookFunc) (char *str);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_trim__pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_strlib_trim__pre[hIndex].func;
			retVal___ = preHookFunc(str);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.trim_(str);
	}
	if( HPMHooks.count.HP_strlib_trim__post ) {
		char* (*postHookFunc) (char* retVal___, char *str);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_trim__post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_strlib_trim__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, str);
		}
	}
	return retVal___;
}
char* HP_strlib_normalize_name_(char *str, const char *delims) {
	int hIndex = 0;
	char* retVal___ = NULL;
	if( HPMHooks.count.HP_strlib_normalize_name__pre ) {
		char* (*preHookFunc) (char *str, const char *delims);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_normalize_name__pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_strlib_normalize_name__pre[hIndex].func;
			retVal___ = preHookFunc(str, delims);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.normalize_name_(str, delims);
	}
	if( HPMHooks.count.HP_strlib_normalize_name__post ) {
		char* (*postHookFunc) (char* retVal___, char *str, const char *delims);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_normalize_name__post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_strlib_normalize_name__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, str, delims);
		}
	}
	return retVal___;
}
const char* HP_strlib_stristr_(const char *haystack, const char *needle) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if( HPMHooks.count.HP_strlib_stristr__pre ) {
		const char* (*preHookFunc) (const char *haystack, const char *needle);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_stristr__pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_strlib_stristr__pre[hIndex].func;
			retVal___ = preHookFunc(haystack, needle);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.stristr_(haystack, needle);
	}
	if( HPMHooks.count.HP_strlib_stristr__post ) {
		const char* (*postHookFunc) (const char* retVal___, const char *haystack, const char *needle);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_stristr__post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_strlib_stristr__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, haystack, needle);
		}
	}
	return retVal___;
}
size_t HP_strlib_strnlen_(const char *string, size_t maxlen) {
	int hIndex = 0;
	size_t retVal___ = 0;
	if( HPMHooks.count.HP_strlib_strnlen__pre ) {
		size_t (*preHookFunc) (const char *string, size_t *maxlen);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_strnlen__pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_strlib_strnlen__pre[hIndex].func;
			retVal___ = preHookFunc(string, &maxlen);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.strnlen_(string, maxlen);
	}
	if( HPMHooks.count.HP_strlib_strnlen__post ) {
		size_t (*postHookFunc) (size_t retVal___, const char *string, size_t *maxlen);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_strnlen__post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_strlib_strnlen__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, string, &maxlen);
		}
	}
	return retVal___;
}
char* HP_strlib_strtok_r_(char *s1, const char *s2, char **lasts) {
	int hIndex = 0;
	char* retVal___ = NULL;
	if( HPMHooks.count.HP_strlib_strtok_r__pre ) {
		char* (*preHookFunc) (char *s1, const char *s2, char **lasts);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_strtok_r__pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_strlib_strtok_r__pre[hIndex].func;
			retVal___ = preHookFunc(s1, s2, lasts);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.strtok_r_(s1, s2, lasts);
	}
	if( HPMHooks.count.HP_strlib_strtok_r__post ) {
		char* (*postHookFunc) (char* retVal___, char *s1, const char *s2, char **lasts);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_strtok_r__post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_strlib_strtok_r__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, s1, s2, lasts);
		}
	}
	return retVal___;
}
int HP_strlib_e_mail_check_(char *email) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_strlib_e_mail_check__pre ) {
		int (*preHookFunc) (char *email);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_e_mail_check__pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_strlib_e_mail_check__pre[hIndex].func;
			retVal___ = preHookFunc(email);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.e_mail_check_(email);
	}
	if( HPMHooks.count.HP_strlib_e_mail_check__post ) {
		int (*postHookFunc) (int retVal___, char *email);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_e_mail_check__post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_strlib_e_mail_check__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, email);
		}
	}
	return retVal___;
}
int HP_strlib_config_switch_(const char *str) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_strlib_config_switch__pre ) {
		int (*preHookFunc) (const char *str);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_config_switch__pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_strlib_config_switch__pre[hIndex].func;
			retVal___ = preHookFunc(str);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.config_switch_(str);
	}
	if( HPMHooks.count.HP_strlib_config_switch__post ) {
		int (*postHookFunc) (int retVal___, const char *str);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_config_switch__post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_strlib_config_switch__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, str);
		}
	}
	return retVal___;
}
char* HP_strlib_safestrncpy_(char *dst, const char *src, size_t n) {
	int hIndex = 0;
	char* retVal___ = NULL;
	if( HPMHooks.count.HP_strlib_safestrncpy__pre ) {
		char* (*preHookFunc) (char *dst, const char *src, size_t *n);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_safestrncpy__pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_strlib_safestrncpy__pre[hIndex].func;
			retVal___ = preHookFunc(dst, src, &n);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.safestrncpy_(dst, src, n);
	}
	if( HPMHooks.count.HP_strlib_safestrncpy__post ) {
		char* (*postHookFunc) (char* retVal___, char *dst, const char *src, size_t *n);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_safestrncpy__post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_strlib_safestrncpy__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, dst, src, &n);
		}
	}
	return retVal___;
}
size_t HP_strlib_safestrnlen_(const char *string, size_t maxlen) {
	int hIndex = 0;
	size_t retVal___ = 0;
	if( HPMHooks.count.HP_strlib_safestrnlen__pre ) {
		size_t (*preHookFunc) (const char *string, size_t *maxlen);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_safestrnlen__pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_strlib_safestrnlen__pre[hIndex].func;
			retVal___ = preHookFunc(string, &maxlen);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.safestrnlen_(string, maxlen);
	}
	if( HPMHooks.count.HP_strlib_safestrnlen__post ) {
		size_t (*postHookFunc) (size_t retVal___, const char *string, size_t *maxlen);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_safestrnlen__post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_strlib_safestrnlen__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, string, &maxlen);
		}
	}
	return retVal___;
}
int HP_strlib_strline_(const char *str, size_t pos) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_strlib_strline__pre ) {
		int (*preHookFunc) (const char *str, size_t *pos);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_strline__pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_strlib_strline__pre[hIndex].func;
			retVal___ = preHookFunc(str, &pos);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.strline_(str, pos);
	}
	if( HPMHooks.count.HP_strlib_strline__post ) {
		int (*postHookFunc) (int retVal___, const char *str, size_t *pos);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_strline__post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_strlib_strline__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, str, &pos);
		}
	}
	return retVal___;
}
bool HP_strlib_bin2hex_(char *output, unsigned char *input, size_t count) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_strlib_bin2hex__pre ) {
		bool (*preHookFunc) (char *output, unsigned char *input, size_t *count);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_bin2hex__pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_strlib_bin2hex__pre[hIndex].func;
			retVal___ = preHookFunc(output, input, &count);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.bin2hex_(output, input, count);
	}
	if( HPMHooks.count.HP_strlib_bin2hex__post ) {
		bool (*postHookFunc) (bool retVal___, char *output, unsigned char *input, size_t *count);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_strlib_bin2hex__post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_strlib_bin2hex__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, output, input, &count);
		}
	}
	return retVal___;
}
/* sv_interface */
int HP_sv_parse_next(struct s_svstate *svstate) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_sv_parse_next_pre ) {
		int (*preHookFunc) (struct s_svstate *svstate);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sv_parse_next_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sv_parse_next_pre[hIndex].func;
			retVal___ = preHookFunc(svstate);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sv.parse_next(svstate);
	}
	if( HPMHooks.count.HP_sv_parse_next_post ) {
		int (*postHookFunc) (int retVal___, struct s_svstate *svstate);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sv_parse_next_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sv_parse_next_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, svstate);
		}
	}
	return retVal___;
}
int HP_sv_parse(const char *str, int len, int startoff, char delim, int *out_pos, int npos, enum e_svopt opt) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_sv_parse_pre ) {
		int (*preHookFunc) (const char *str, int *len, int *startoff, char *delim, int *out_pos, int *npos, enum e_svopt *opt);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sv_parse_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sv_parse_pre[hIndex].func;
			retVal___ = preHookFunc(str, &len, &startoff, &delim, out_pos, &npos, &opt);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sv.parse(str, len, startoff, delim, out_pos, npos, opt);
	}
	if( HPMHooks.count.HP_sv_parse_post ) {
		int (*postHookFunc) (int retVal___, const char *str, int *len, int *startoff, char *delim, int *out_pos, int *npos, enum e_svopt *opt);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sv_parse_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sv_parse_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, str, &len, &startoff, &delim, out_pos, &npos, &opt);
		}
	}
	return retVal___;
}
int HP_sv_split(char *str, int len, int startoff, char delim, char **out_fields, int nfields, enum e_svopt opt) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_sv_split_pre ) {
		int (*preHookFunc) (char *str, int *len, int *startoff, char *delim, char **out_fields, int *nfields, enum e_svopt *opt);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sv_split_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sv_split_pre[hIndex].func;
			retVal___ = preHookFunc(str, &len, &startoff, &delim, out_fields, &nfields, &opt);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sv.split(str, len, startoff, delim, out_fields, nfields, opt);
	}
	if( HPMHooks.count.HP_sv_split_post ) {
		int (*postHookFunc) (int retVal___, char *str, int *len, int *startoff, char *delim, char **out_fields, int *nfields, enum e_svopt *opt);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sv_split_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sv_split_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, str, &len, &startoff, &delim, out_fields, &nfields, &opt);
		}
	}
	return retVal___;
}
size_t HP_sv_escape_c(char *out_dest, const char *src, size_t len, const char *escapes) {
	int hIndex = 0;
	size_t retVal___ = 0;
	if( HPMHooks.count.HP_sv_escape_c_pre ) {
		size_t (*preHookFunc) (char *out_dest, const char *src, size_t *len, const char *escapes);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sv_escape_c_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sv_escape_c_pre[hIndex].func;
			retVal___ = preHookFunc(out_dest, src, &len, escapes);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sv.escape_c(out_dest, src, len, escapes);
	}
	if( HPMHooks.count.HP_sv_escape_c_post ) {
		size_t (*postHookFunc) (size_t retVal___, char *out_dest, const char *src, size_t *len, const char *escapes);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sv_escape_c_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sv_escape_c_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, out_dest, src, &len, escapes);
		}
	}
	return retVal___;
}
size_t HP_sv_unescape_c(char *out_dest, const char *src, size_t len) {
	int hIndex = 0;
	size_t retVal___ = 0;
	if( HPMHooks.count.HP_sv_unescape_c_pre ) {
		size_t (*preHookFunc) (char *out_dest, const char *src, size_t *len);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sv_unescape_c_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sv_unescape_c_pre[hIndex].func;
			retVal___ = preHookFunc(out_dest, src, &len);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sv.unescape_c(out_dest, src, len);
	}
	if( HPMHooks.count.HP_sv_unescape_c_post ) {
		size_t (*postHookFunc) (size_t retVal___, char *out_dest, const char *src, size_t *len);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sv_unescape_c_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sv_unescape_c_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, out_dest, src, &len);
		}
	}
	return retVal___;
}
const char* HP_sv_skip_escaped_c(const char *p) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if( HPMHooks.count.HP_sv_skip_escaped_c_pre ) {
		const char* (*preHookFunc) (const char *p);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sv_skip_escaped_c_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sv_skip_escaped_c_pre[hIndex].func;
			retVal___ = preHookFunc(p);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sv.skip_escaped_c(p);
	}
	if( HPMHooks.count.HP_sv_skip_escaped_c_post ) {
		const char* (*postHookFunc) (const char* retVal___, const char *p);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sv_skip_escaped_c_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sv_skip_escaped_c_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, p);
		}
	}
	return retVal___;
}
bool HP_sv_readdb(const char *directory, const char *filename, char delim, int mincols, int maxcols, int maxrows, bool ( *parseproc ) (char *fields[], int columns, int current)) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_sv_readdb_pre ) {
		bool (*preHookFunc) (const char *directory, const char *filename, char *delim, int *mincols, int *maxcols, int *maxrows, bool ( *parseproc ) (char *fields[], int columns, int current));
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sv_readdb_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sv_readdb_pre[hIndex].func;
			retVal___ = preHookFunc(directory, filename, &delim, &mincols, &maxcols, &maxrows, parseproc);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sv.readdb(directory, filename, delim, mincols, maxcols, maxrows, parseproc);
	}
	if( HPMHooks.count.HP_sv_readdb_post ) {
		bool (*postHookFunc) (bool retVal___, const char *directory, const char *filename, char *delim, int *mincols, int *maxcols, int *maxrows, bool ( *parseproc ) (char *fields[], int columns, int current));
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sv_readdb_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sv_readdb_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, directory, filename, &delim, &mincols, &maxcols, &maxrows, parseproc);
		}
	}
	return retVal___;
}
/* sysinfo_interface */
int HP_sysinfo_getpagesize(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_sysinfo_getpagesize_pre ) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_getpagesize_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sysinfo_getpagesize_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.getpagesize();
	}
	if( HPMHooks.count.HP_sysinfo_getpagesize_post ) {
		int (*postHookFunc) (int retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_getpagesize_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sysinfo_getpagesize_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
const char* HP_sysinfo_platform(void) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if( HPMHooks.count.HP_sysinfo_platform_pre ) {
		const char* (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_platform_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sysinfo_platform_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.platform();
	}
	if( HPMHooks.count.HP_sysinfo_platform_post ) {
		const char* (*postHookFunc) (const char* retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_platform_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sysinfo_platform_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
const char* HP_sysinfo_osversion(void) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if( HPMHooks.count.HP_sysinfo_osversion_pre ) {
		const char* (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_osversion_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sysinfo_osversion_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.osversion();
	}
	if( HPMHooks.count.HP_sysinfo_osversion_post ) {
		const char* (*postHookFunc) (const char* retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_osversion_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sysinfo_osversion_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
const char* HP_sysinfo_cpu(void) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if( HPMHooks.count.HP_sysinfo_cpu_pre ) {
		const char* (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_cpu_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sysinfo_cpu_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.cpu();
	}
	if( HPMHooks.count.HP_sysinfo_cpu_post ) {
		const char* (*postHookFunc) (const char* retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_cpu_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sysinfo_cpu_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
int HP_sysinfo_cpucores(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_sysinfo_cpucores_pre ) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_cpucores_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sysinfo_cpucores_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.cpucores();
	}
	if( HPMHooks.count.HP_sysinfo_cpucores_post ) {
		int (*postHookFunc) (int retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_cpucores_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sysinfo_cpucores_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
const char* HP_sysinfo_arch(void) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if( HPMHooks.count.HP_sysinfo_arch_pre ) {
		const char* (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_arch_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sysinfo_arch_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.arch();
	}
	if( HPMHooks.count.HP_sysinfo_arch_post ) {
		const char* (*postHookFunc) (const char* retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_arch_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sysinfo_arch_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
bool HP_sysinfo_is64bit(void) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_sysinfo_is64bit_pre ) {
		bool (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_is64bit_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sysinfo_is64bit_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.is64bit();
	}
	if( HPMHooks.count.HP_sysinfo_is64bit_post ) {
		bool (*postHookFunc) (bool retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_is64bit_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sysinfo_is64bit_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
const char* HP_sysinfo_compiler(void) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if( HPMHooks.count.HP_sysinfo_compiler_pre ) {
		const char* (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_compiler_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sysinfo_compiler_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.compiler();
	}
	if( HPMHooks.count.HP_sysinfo_compiler_post ) {
		const char* (*postHookFunc) (const char* retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_compiler_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sysinfo_compiler_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
const char* HP_sysinfo_cflags(void) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if( HPMHooks.count.HP_sysinfo_cflags_pre ) {
		const char* (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_cflags_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sysinfo_cflags_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.cflags();
	}
	if( HPMHooks.count.HP_sysinfo_cflags_post ) {
		const char* (*postHookFunc) (const char* retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_cflags_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sysinfo_cflags_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
const char* HP_sysinfo_time(void) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if( HPMHooks.count.HP_sysinfo_time_pre ) {
		const char* (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_time_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sysinfo_time_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.time();
	}
	if( HPMHooks.count.HP_sysinfo_time_post ) {
		const char* (*postHookFunc) (const char* retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_time_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sysinfo_time_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
const char* HP_sysinfo_vcstype(void) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if( HPMHooks.count.HP_sysinfo_vcstype_pre ) {
		const char* (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_vcstype_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sysinfo_vcstype_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.vcstype();
	}
	if( HPMHooks.count.HP_sysinfo_vcstype_post ) {
		const char* (*postHookFunc) (const char* retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_vcstype_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sysinfo_vcstype_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
int HP_sysinfo_vcstypeid(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_sysinfo_vcstypeid_pre ) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_vcstypeid_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sysinfo_vcstypeid_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.vcstypeid();
	}
	if( HPMHooks.count.HP_sysinfo_vcstypeid_post ) {
		int (*postHookFunc) (int retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_vcstypeid_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sysinfo_vcstypeid_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
const char* HP_sysinfo_vcsrevision_src(void) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if( HPMHooks.count.HP_sysinfo_vcsrevision_src_pre ) {
		const char* (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_vcsrevision_src_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sysinfo_vcsrevision_src_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.vcsrevision_src();
	}
	if( HPMHooks.count.HP_sysinfo_vcsrevision_src_post ) {
		const char* (*postHookFunc) (const char* retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_vcsrevision_src_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sysinfo_vcsrevision_src_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
const char* HP_sysinfo_vcsrevision_scripts(void) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if( HPMHooks.count.HP_sysinfo_vcsrevision_scripts_pre ) {
		const char* (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_vcsrevision_scripts_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sysinfo_vcsrevision_scripts_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.vcsrevision_scripts();
	}
	if( HPMHooks.count.HP_sysinfo_vcsrevision_scripts_post ) {
		const char* (*postHookFunc) (const char* retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_vcsrevision_scripts_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sysinfo_vcsrevision_scripts_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_sysinfo_vcsrevision_reload(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_sysinfo_vcsrevision_reload_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_vcsrevision_reload_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sysinfo_vcsrevision_reload_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sysinfo.vcsrevision_reload();
	}
	if( HPMHooks.count.HP_sysinfo_vcsrevision_reload_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_vcsrevision_reload_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sysinfo_vcsrevision_reload_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
bool HP_sysinfo_is_superuser(void) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_sysinfo_is_superuser_pre ) {
		bool (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_is_superuser_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sysinfo_is_superuser_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.is_superuser();
	}
	if( HPMHooks.count.HP_sysinfo_is_superuser_post ) {
		bool (*postHookFunc) (bool retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_is_superuser_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sysinfo_is_superuser_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_sysinfo_init(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_sysinfo_init_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sysinfo_init_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sysinfo.init();
	}
	if( HPMHooks.count.HP_sysinfo_init_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sysinfo_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_sysinfo_final(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_sysinfo_final_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_final_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_sysinfo_final_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sysinfo.final();
	}
	if( HPMHooks.count.HP_sysinfo_final_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_final_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_sysinfo_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
/* timer_interface */
int64 HP_timer_gettick(void) {
	int hIndex = 0;
	int64 retVal___ = 0;
	if( HPMHooks.count.HP_timer_gettick_pre ) {
		int64 (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_gettick_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_timer_gettick_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.timer.gettick();
	}
	if( HPMHooks.count.HP_timer_gettick_post ) {
		int64 (*postHookFunc) (int64 retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_gettick_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_timer_gettick_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
int64 HP_timer_gettick_nocache(void) {
	int hIndex = 0;
	int64 retVal___ = 0;
	if( HPMHooks.count.HP_timer_gettick_nocache_pre ) {
		int64 (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_gettick_nocache_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_timer_gettick_nocache_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.timer.gettick_nocache();
	}
	if( HPMHooks.count.HP_timer_gettick_nocache_post ) {
		int64 (*postHookFunc) (int64 retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_gettick_nocache_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_timer_gettick_nocache_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
int HP_timer_add(int64 tick, TimerFunc func, int id, intptr_t data) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_timer_add_pre ) {
		int (*preHookFunc) (int64 *tick, TimerFunc *func, int *id, intptr_t *data);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_add_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_timer_add_pre[hIndex].func;
			retVal___ = preHookFunc(&tick, &func, &id, &data);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.timer.add(tick, func, id, data);
	}
	if( HPMHooks.count.HP_timer_add_post ) {
		int (*postHookFunc) (int retVal___, int64 *tick, TimerFunc *func, int *id, intptr_t *data);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_add_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_timer_add_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &tick, &func, &id, &data);
		}
	}
	return retVal___;
}
int HP_timer_add_interval(int64 tick, TimerFunc func, int id, intptr_t data, int interval) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_timer_add_interval_pre ) {
		int (*preHookFunc) (int64 *tick, TimerFunc *func, int *id, intptr_t *data, int *interval);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_add_interval_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_timer_add_interval_pre[hIndex].func;
			retVal___ = preHookFunc(&tick, &func, &id, &data, &interval);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.timer.add_interval(tick, func, id, data, interval);
	}
	if( HPMHooks.count.HP_timer_add_interval_post ) {
		int (*postHookFunc) (int retVal___, int64 *tick, TimerFunc *func, int *id, intptr_t *data, int *interval);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_add_interval_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_timer_add_interval_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &tick, &func, &id, &data, &interval);
		}
	}
	return retVal___;
}
const struct TimerData* HP_timer_get(int tid) {
	int hIndex = 0;
	const struct TimerData* retVal___ = NULL;
	if( HPMHooks.count.HP_timer_get_pre ) {
		const struct TimerData* (*preHookFunc) (int *tid);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_get_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_timer_get_pre[hIndex].func;
			retVal___ = preHookFunc(&tid);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.timer.get(tid);
	}
	if( HPMHooks.count.HP_timer_get_post ) {
		const struct TimerData* (*postHookFunc) (const struct TimerData* retVal___, int *tid);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_get_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_timer_get_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &tid);
		}
	}
	return retVal___;
}
int HP_timer_delete(int tid, TimerFunc func) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_timer_delete_pre ) {
		int (*preHookFunc) (int *tid, TimerFunc *func);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_delete_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_timer_delete_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &func);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.timer.delete(tid, func);
	}
	if( HPMHooks.count.HP_timer_delete_post ) {
		int (*postHookFunc) (int retVal___, int *tid, TimerFunc *func);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_delete_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_timer_delete_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &tid, &func);
		}
	}
	return retVal___;
}
int64 HP_timer_addtick(int tid, int64 tick) {
	int hIndex = 0;
	int64 retVal___ = 0;
	if( HPMHooks.count.HP_timer_addtick_pre ) {
		int64 (*preHookFunc) (int *tid, int64 *tick);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_addtick_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_timer_addtick_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &tick);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.timer.addtick(tid, tick);
	}
	if( HPMHooks.count.HP_timer_addtick_post ) {
		int64 (*postHookFunc) (int64 retVal___, int *tid, int64 *tick);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_addtick_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_timer_addtick_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &tid, &tick);
		}
	}
	return retVal___;
}
int64 HP_timer_settick(int tid, int64 tick) {
	int hIndex = 0;
	int64 retVal___ = 0;
	if( HPMHooks.count.HP_timer_settick_pre ) {
		int64 (*preHookFunc) (int *tid, int64 *tick);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_settick_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_timer_settick_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &tick);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.timer.settick(tid, tick);
	}
	if( HPMHooks.count.HP_timer_settick_post ) {
		int64 (*postHookFunc) (int64 retVal___, int *tid, int64 *tick);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_settick_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_timer_settick_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &tid, &tick);
		}
	}
	return retVal___;
}
int HP_timer_add_func_list(TimerFunc func, char *name) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_timer_add_func_list_pre ) {
		int (*preHookFunc) (TimerFunc *func, char *name);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_add_func_list_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_timer_add_func_list_pre[hIndex].func;
			retVal___ = preHookFunc(&func, name);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.timer.add_func_list(func, name);
	}
	if( HPMHooks.count.HP_timer_add_func_list_post ) {
		int (*postHookFunc) (int retVal___, TimerFunc *func, char *name);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_add_func_list_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_timer_add_func_list_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &func, name);
		}
	}
	return retVal___;
}
unsigned long HP_timer_get_uptime(void) {
	int hIndex = 0;
	unsigned long retVal___ = 0;
	if( HPMHooks.count.HP_timer_get_uptime_pre ) {
		unsigned long (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_get_uptime_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_timer_get_uptime_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.timer.get_uptime();
	}
	if( HPMHooks.count.HP_timer_get_uptime_post ) {
		unsigned long (*postHookFunc) (unsigned long retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_get_uptime_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_timer_get_uptime_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
int HP_timer_perform(int64 tick) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_timer_perform_pre ) {
		int (*preHookFunc) (int64 *tick);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_perform_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_timer_perform_pre[hIndex].func;
			retVal___ = preHookFunc(&tick);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.timer.perform(tick);
	}
	if( HPMHooks.count.HP_timer_perform_post ) {
		int (*postHookFunc) (int retVal___, int64 *tick);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_perform_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_timer_perform_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &tick);
		}
	}
	return retVal___;
}
void HP_timer_init(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_timer_init_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_timer_init_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.timer.init();
	}
	if( HPMHooks.count.HP_timer_init_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_timer_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_timer_final(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_timer_final_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_final_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_timer_final_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.timer.final();
	}
	if( HPMHooks.count.HP_timer_final_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_timer_final_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_timer_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
