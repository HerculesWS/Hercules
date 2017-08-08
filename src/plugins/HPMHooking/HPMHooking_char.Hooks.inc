/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-2017  Hercules Dev Team
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

/* GENERATED FILE DO NOT EDIT */

/* HCache_interface */
void HP_HCache_init(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_HCache_init_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_HCache_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_HCache_init_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.HCache.init();
	}
	if (HPMHooks.count.HP_HCache_init_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_HCache_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_HCache_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
bool HP_HCache_check(const char *file) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_HCache_check_pre > 0) {
		bool (*preHookFunc) (const char **file);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_HCache_check_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_HCache_check_pre[hIndex].func;
			retVal___ = preHookFunc(&file);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.HCache.check(file);
	}
	if (HPMHooks.count.HP_HCache_check_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *file);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_HCache_check_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_HCache_check_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, file);
		}
	}
	return retVal___;
}
FILE* HP_HCache_open(const char *file, const char *opt) {
	int hIndex = 0;
	FILE* retVal___ = NULL;
	if (HPMHooks.count.HP_HCache_open_pre > 0) {
		FILE* (*preHookFunc) (const char **file, const char **opt);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_HCache_open_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_HCache_open_pre[hIndex].func;
			retVal___ = preHookFunc(&file, &opt);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.HCache.open(file, opt);
	}
	if (HPMHooks.count.HP_HCache_open_post > 0) {
		FILE* (*postHookFunc) (FILE* retVal___, const char *file, const char *opt);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_HCache_open_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_HCache_open_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, file, opt);
		}
	}
	return retVal___;
}
/* char_interface */
int HP_chr_waiting_disconnect(int tid, int64 tick, int id, intptr_t data) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_waiting_disconnect_pre > 0) {
		int (*preHookFunc) (int *tid, int64 *tick, int *id, intptr_t *data);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_waiting_disconnect_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_waiting_disconnect_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &tick, &id, &data);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.waiting_disconnect(tid, tick, id, data);
	}
	if (HPMHooks.count.HP_chr_waiting_disconnect_post > 0) {
		int (*postHookFunc) (int retVal___, int tid, int64 tick, int id, intptr_t data);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_waiting_disconnect_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_waiting_disconnect_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, tid, tick, id, data);
		}
	}
	return retVal___;
}
int HP_chr_delete_char_sql(int char_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_delete_char_sql_pre > 0) {
		int (*preHookFunc) (int *char_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete_char_sql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_delete_char_sql_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.delete_char_sql(char_id);
	}
	if (HPMHooks.count.HP_chr_delete_char_sql_post > 0) {
		int (*postHookFunc) (int retVal___, int char_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete_char_sql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_delete_char_sql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, char_id);
		}
	}
	return retVal___;
}
struct DBData HP_chr_create_online_char_data(union DBKey key, va_list args) {
	int hIndex = 0;
	struct DBData retVal___ = { 0 };
	if (HPMHooks.count.HP_chr_create_online_char_data_pre > 0) {
		struct DBData (*preHookFunc) (union DBKey *key, va_list args);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_create_online_char_data_pre; hIndex++) {
			va_list args___copy; va_copy(args___copy, args);
			preHookFunc = HPMHooks.list.HP_chr_create_online_char_data_pre[hIndex].func;
			retVal___ = preHookFunc(&key, args___copy);
			va_end(args___copy);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		va_list args___copy; va_copy(args___copy, args);
		retVal___ = HPMHooks.source.chr.create_online_char_data(key, args___copy);
		va_end(args___copy);
	}
	if (HPMHooks.count.HP_chr_create_online_char_data_post > 0) {
		struct DBData (*postHookFunc) (struct DBData retVal___, union DBKey key, va_list args);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_create_online_char_data_post; hIndex++) {
			va_list args___copy; va_copy(args___copy, args);
			postHookFunc = HPMHooks.list.HP_chr_create_online_char_data_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, key, args___copy);
			va_end(args___copy);
		}
	}
	return retVal___;
}
void HP_chr_set_account_online(int account_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_set_account_online_pre > 0) {
		void (*preHookFunc) (int *account_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_account_online_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_set_account_online_pre[hIndex].func;
			preHookFunc(&account_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.set_account_online(account_id);
	}
	if (HPMHooks.count.HP_chr_set_account_online_post > 0) {
		void (*postHookFunc) (int account_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_account_online_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_set_account_online_post[hIndex].func;
			postHookFunc(account_id);
		}
	}
	return;
}
void HP_chr_set_account_offline(int account_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_set_account_offline_pre > 0) {
		void (*preHookFunc) (int *account_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_account_offline_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_set_account_offline_pre[hIndex].func;
			preHookFunc(&account_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.set_account_offline(account_id);
	}
	if (HPMHooks.count.HP_chr_set_account_offline_post > 0) {
		void (*postHookFunc) (int account_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_account_offline_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_set_account_offline_post[hIndex].func;
			postHookFunc(account_id);
		}
	}
	return;
}
void HP_chr_set_char_charselect(int account_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_set_char_charselect_pre > 0) {
		void (*preHookFunc) (int *account_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_char_charselect_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_set_char_charselect_pre[hIndex].func;
			preHookFunc(&account_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.set_char_charselect(account_id);
	}
	if (HPMHooks.count.HP_chr_set_char_charselect_post > 0) {
		void (*postHookFunc) (int account_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_char_charselect_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_set_char_charselect_post[hIndex].func;
			postHookFunc(account_id);
		}
	}
	return;
}
void HP_chr_set_char_online(int map_id, int char_id, int account_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_set_char_online_pre > 0) {
		void (*preHookFunc) (int *map_id, int *char_id, int *account_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_char_online_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_set_char_online_pre[hIndex].func;
			preHookFunc(&map_id, &char_id, &account_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.set_char_online(map_id, char_id, account_id);
	}
	if (HPMHooks.count.HP_chr_set_char_online_post > 0) {
		void (*postHookFunc) (int map_id, int char_id, int account_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_char_online_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_set_char_online_post[hIndex].func;
			postHookFunc(map_id, char_id, account_id);
		}
	}
	return;
}
void HP_chr_set_char_offline(int char_id, int account_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_set_char_offline_pre > 0) {
		void (*preHookFunc) (int *char_id, int *account_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_char_offline_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_set_char_offline_pre[hIndex].func;
			preHookFunc(&char_id, &account_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.set_char_offline(char_id, account_id);
	}
	if (HPMHooks.count.HP_chr_set_char_offline_post > 0) {
		void (*postHookFunc) (int char_id, int account_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_char_offline_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_set_char_offline_post[hIndex].func;
			postHookFunc(char_id, account_id);
		}
	}
	return;
}
int HP_chr_db_setoffline(union DBKey key, struct DBData *data, va_list ap) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_db_setoffline_pre > 0) {
		int (*preHookFunc) (union DBKey *key, struct DBData **data, va_list ap);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_db_setoffline_pre; hIndex++) {
			va_list ap___copy; va_copy(ap___copy, ap);
			preHookFunc = HPMHooks.list.HP_chr_db_setoffline_pre[hIndex].func;
			retVal___ = preHookFunc(&key, &data, ap___copy);
			va_end(ap___copy);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		va_list ap___copy; va_copy(ap___copy, ap);
		retVal___ = HPMHooks.source.chr.db_setoffline(key, data, ap___copy);
		va_end(ap___copy);
	}
	if (HPMHooks.count.HP_chr_db_setoffline_post > 0) {
		int (*postHookFunc) (int retVal___, union DBKey key, struct DBData *data, va_list ap);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_db_setoffline_post; hIndex++) {
			va_list ap___copy; va_copy(ap___copy, ap);
			postHookFunc = HPMHooks.list.HP_chr_db_setoffline_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, key, data, ap___copy);
			va_end(ap___copy);
		}
	}
	return retVal___;
}
int HP_chr_db_kickoffline(union DBKey key, struct DBData *data, va_list ap) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_db_kickoffline_pre > 0) {
		int (*preHookFunc) (union DBKey *key, struct DBData **data, va_list ap);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_db_kickoffline_pre; hIndex++) {
			va_list ap___copy; va_copy(ap___copy, ap);
			preHookFunc = HPMHooks.list.HP_chr_db_kickoffline_pre[hIndex].func;
			retVal___ = preHookFunc(&key, &data, ap___copy);
			va_end(ap___copy);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		va_list ap___copy; va_copy(ap___copy, ap);
		retVal___ = HPMHooks.source.chr.db_kickoffline(key, data, ap___copy);
		va_end(ap___copy);
	}
	if (HPMHooks.count.HP_chr_db_kickoffline_post > 0) {
		int (*postHookFunc) (int retVal___, union DBKey key, struct DBData *data, va_list ap);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_db_kickoffline_post; hIndex++) {
			va_list ap___copy; va_copy(ap___copy, ap);
			postHookFunc = HPMHooks.list.HP_chr_db_kickoffline_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, key, data, ap___copy);
			va_end(ap___copy);
		}
	}
	return retVal___;
}
void HP_chr_set_login_all_offline(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_set_login_all_offline_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_login_all_offline_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_set_login_all_offline_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.set_login_all_offline();
	}
	if (HPMHooks.count.HP_chr_set_login_all_offline_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_login_all_offline_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_set_login_all_offline_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_chr_set_all_offline(int id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_set_all_offline_pre > 0) {
		void (*preHookFunc) (int *id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_all_offline_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_set_all_offline_pre[hIndex].func;
			preHookFunc(&id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.set_all_offline(id);
	}
	if (HPMHooks.count.HP_chr_set_all_offline_post > 0) {
		void (*postHookFunc) (int id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_all_offline_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_set_all_offline_post[hIndex].func;
			postHookFunc(id);
		}
	}
	return;
}
void HP_chr_set_all_offline_sql(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_set_all_offline_sql_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_all_offline_sql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_set_all_offline_sql_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.set_all_offline_sql();
	}
	if (HPMHooks.count.HP_chr_set_all_offline_sql_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_all_offline_sql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_set_all_offline_sql_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
struct DBData HP_chr_create_charstatus(union DBKey key, va_list args) {
	int hIndex = 0;
	struct DBData retVal___ = { 0 };
	if (HPMHooks.count.HP_chr_create_charstatus_pre > 0) {
		struct DBData (*preHookFunc) (union DBKey *key, va_list args);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_create_charstatus_pre; hIndex++) {
			va_list args___copy; va_copy(args___copy, args);
			preHookFunc = HPMHooks.list.HP_chr_create_charstatus_pre[hIndex].func;
			retVal___ = preHookFunc(&key, args___copy);
			va_end(args___copy);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		va_list args___copy; va_copy(args___copy, args);
		retVal___ = HPMHooks.source.chr.create_charstatus(key, args___copy);
		va_end(args___copy);
	}
	if (HPMHooks.count.HP_chr_create_charstatus_post > 0) {
		struct DBData (*postHookFunc) (struct DBData retVal___, union DBKey key, va_list args);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_create_charstatus_post; hIndex++) {
			va_list args___copy; va_copy(args___copy, args);
			postHookFunc = HPMHooks.list.HP_chr_create_charstatus_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, key, args___copy);
			va_end(args___copy);
		}
	}
	return retVal___;
}
int HP_chr_mmo_char_tosql(int char_id, struct mmo_charstatus *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_mmo_char_tosql_pre > 0) {
		int (*preHookFunc) (int *char_id, struct mmo_charstatus **p);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_tosql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_mmo_char_tosql_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &p);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.mmo_char_tosql(char_id, p);
	}
	if (HPMHooks.count.HP_chr_mmo_char_tosql_post > 0) {
		int (*postHookFunc) (int retVal___, int char_id, struct mmo_charstatus *p);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_tosql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_mmo_char_tosql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, char_id, p);
		}
	}
	return retVal___;
}
int HP_chr_getitemdata_from_sql(struct item *items, int max, int guid, enum inventory_table_type table) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_getitemdata_from_sql_pre > 0) {
		int (*preHookFunc) (struct item **items, int *max, int *guid, enum inventory_table_type *table);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_getitemdata_from_sql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_getitemdata_from_sql_pre[hIndex].func;
			retVal___ = preHookFunc(&items, &max, &guid, &table);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.getitemdata_from_sql(items, max, guid, table);
	}
	if (HPMHooks.count.HP_chr_getitemdata_from_sql_post > 0) {
		int (*postHookFunc) (int retVal___, struct item *items, int max, int guid, enum inventory_table_type table);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_getitemdata_from_sql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_getitemdata_from_sql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, items, max, guid, table);
		}
	}
	return retVal___;
}
int HP_chr_memitemdata_to_sql(const struct item items[], int id, enum inventory_table_type table) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_memitemdata_to_sql_pre > 0) {
		int (*preHookFunc) (const struct item *items[], int *id, enum inventory_table_type *table);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_memitemdata_to_sql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_memitemdata_to_sql_pre[hIndex].func;
			retVal___ = preHookFunc(&items, &id, &table);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.memitemdata_to_sql(items, id, table);
	}
	if (HPMHooks.count.HP_chr_memitemdata_to_sql_post > 0) {
		int (*postHookFunc) (int retVal___, const struct item items[], int id, enum inventory_table_type table);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_memitemdata_to_sql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_memitemdata_to_sql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, items, id, table);
		}
	}
	return retVal___;
}
int HP_chr_mmo_gender(const struct char_session_data *sd, const struct mmo_charstatus *p, char sex) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_mmo_gender_pre > 0) {
		int (*preHookFunc) (const struct char_session_data **sd, const struct mmo_charstatus **p, char *sex);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_gender_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_mmo_gender_pre[hIndex].func;
			retVal___ = preHookFunc(&sd, &p, &sex);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.mmo_gender(sd, p, sex);
	}
	if (HPMHooks.count.HP_chr_mmo_gender_post > 0) {
		int (*postHookFunc) (int retVal___, const struct char_session_data *sd, const struct mmo_charstatus *p, char sex);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_gender_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_mmo_gender_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, sd, p, sex);
		}
	}
	return retVal___;
}
int HP_chr_mmo_chars_fromsql(struct char_session_data *sd, uint8 *buf) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_mmo_chars_fromsql_pre > 0) {
		int (*preHookFunc) (struct char_session_data **sd, uint8 **buf);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_chars_fromsql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_mmo_chars_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(&sd, &buf);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.mmo_chars_fromsql(sd, buf);
	}
	if (HPMHooks.count.HP_chr_mmo_chars_fromsql_post > 0) {
		int (*postHookFunc) (int retVal___, struct char_session_data *sd, uint8 *buf);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_chars_fromsql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_mmo_chars_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, sd, buf);
		}
	}
	return retVal___;
}
int HP_chr_mmo_char_fromsql(int char_id, struct mmo_charstatus *p, bool load_everything) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_mmo_char_fromsql_pre > 0) {
		int (*preHookFunc) (int *char_id, struct mmo_charstatus **p, bool *load_everything);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_fromsql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_mmo_char_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &p, &load_everything);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.mmo_char_fromsql(char_id, p, load_everything);
	}
	if (HPMHooks.count.HP_chr_mmo_char_fromsql_post > 0) {
		int (*postHookFunc) (int retVal___, int char_id, struct mmo_charstatus *p, bool load_everything);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_fromsql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_mmo_char_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, char_id, p, load_everything);
		}
	}
	return retVal___;
}
int HP_chr_mmo_char_sql_init(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_mmo_char_sql_init_pre > 0) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_sql_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_mmo_char_sql_init_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.mmo_char_sql_init();
	}
	if (HPMHooks.count.HP_chr_mmo_char_sql_init_post > 0) {
		int (*postHookFunc) (int retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_sql_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_mmo_char_sql_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
bool HP_chr_char_slotchange(struct char_session_data *sd, int fd, unsigned short from, unsigned short to) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_chr_char_slotchange_pre > 0) {
		bool (*preHookFunc) (struct char_session_data **sd, int *fd, unsigned short *from, unsigned short *to);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_char_slotchange_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_char_slotchange_pre[hIndex].func;
			retVal___ = preHookFunc(&sd, &fd, &from, &to);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.char_slotchange(sd, fd, from, to);
	}
	if (HPMHooks.count.HP_chr_char_slotchange_post > 0) {
		bool (*postHookFunc) (bool retVal___, struct char_session_data *sd, int fd, unsigned short from, unsigned short to);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_char_slotchange_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_char_slotchange_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, sd, fd, from, to);
		}
	}
	return retVal___;
}
int HP_chr_rename_char_sql(struct char_session_data *sd, int char_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_rename_char_sql_pre > 0) {
		int (*preHookFunc) (struct char_session_data **sd, int *char_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_rename_char_sql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_rename_char_sql_pre[hIndex].func;
			retVal___ = preHookFunc(&sd, &char_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.rename_char_sql(sd, char_id);
	}
	if (HPMHooks.count.HP_chr_rename_char_sql_post > 0) {
		int (*postHookFunc) (int retVal___, struct char_session_data *sd, int char_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_rename_char_sql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_rename_char_sql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, sd, char_id);
		}
	}
	return retVal___;
}
int HP_chr_check_char_name(char *name, char *esc_name) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_check_char_name_pre > 0) {
		int (*preHookFunc) (char **name, char **esc_name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_check_char_name_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_check_char_name_pre[hIndex].func;
			retVal___ = preHookFunc(&name, &esc_name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.check_char_name(name, esc_name);
	}
	if (HPMHooks.count.HP_chr_check_char_name_post > 0) {
		int (*postHookFunc) (int retVal___, char *name, char *esc_name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_check_char_name_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_check_char_name_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, name, esc_name);
		}
	}
	return retVal___;
}
int HP_chr_make_new_char_sql(struct char_session_data *sd, const char *name_, int str, int agi, int vit, int int_, int dex, int luk, int slot, int hair_color, int hair_style, short starting_job, uint8 sex) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_make_new_char_sql_pre > 0) {
		int (*preHookFunc) (struct char_session_data **sd, const char **name_, int *str, int *agi, int *vit, int *int_, int *dex, int *luk, int *slot, int *hair_color, int *hair_style, short *starting_job, uint8 *sex);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_make_new_char_sql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_make_new_char_sql_pre[hIndex].func;
			retVal___ = preHookFunc(&sd, &name_, &str, &agi, &vit, &int_, &dex, &luk, &slot, &hair_color, &hair_style, &starting_job, &sex);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.make_new_char_sql(sd, name_, str, agi, vit, int_, dex, luk, slot, hair_color, hair_style, starting_job, sex);
	}
	if (HPMHooks.count.HP_chr_make_new_char_sql_post > 0) {
		int (*postHookFunc) (int retVal___, struct char_session_data *sd, const char *name_, int str, int agi, int vit, int int_, int dex, int luk, int slot, int hair_color, int hair_style, short starting_job, uint8 sex);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_make_new_char_sql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_make_new_char_sql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, sd, name_, str, agi, vit, int_, dex, luk, slot, hair_color, hair_style, starting_job, sex);
		}
	}
	return retVal___;
}
int HP_chr_divorce_char_sql(int partner_id1, int partner_id2) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_divorce_char_sql_pre > 0) {
		int (*preHookFunc) (int *partner_id1, int *partner_id2);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_divorce_char_sql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_divorce_char_sql_pre[hIndex].func;
			retVal___ = preHookFunc(&partner_id1, &partner_id2);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.divorce_char_sql(partner_id1, partner_id2);
	}
	if (HPMHooks.count.HP_chr_divorce_char_sql_post > 0) {
		int (*postHookFunc) (int retVal___, int partner_id1, int partner_id2);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_divorce_char_sql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_divorce_char_sql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, partner_id1, partner_id2);
		}
	}
	return retVal___;
}
int HP_chr_count_users(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_count_users_pre > 0) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_count_users_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_count_users_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.count_users();
	}
	if (HPMHooks.count.HP_chr_count_users_post > 0) {
		int (*postHookFunc) (int retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_count_users_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_count_users_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
int HP_chr_mmo_char_tobuf(uint8 *buffer, struct mmo_charstatus *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_mmo_char_tobuf_pre > 0) {
		int (*preHookFunc) (uint8 **buffer, struct mmo_charstatus **p);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_tobuf_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_mmo_char_tobuf_pre[hIndex].func;
			retVal___ = preHookFunc(&buffer, &p);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.mmo_char_tobuf(buffer, p);
	}
	if (HPMHooks.count.HP_chr_mmo_char_tobuf_post > 0) {
		int (*postHookFunc) (int retVal___, uint8 *buffer, struct mmo_charstatus *p);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_tobuf_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_mmo_char_tobuf_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, buffer, p);
		}
	}
	return retVal___;
}
void HP_chr_mmo_char_send099d(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_mmo_char_send099d_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_send099d_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_mmo_char_send099d_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.mmo_char_send099d(fd, sd);
	}
	if (HPMHooks.count.HP_chr_mmo_char_send099d_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_send099d_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_mmo_char_send099d_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_chr_mmo_char_send_ban_list(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_mmo_char_send_ban_list_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_send_ban_list_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_mmo_char_send_ban_list_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.mmo_char_send_ban_list(fd, sd);
	}
	if (HPMHooks.count.HP_chr_mmo_char_send_ban_list_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_send_ban_list_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_mmo_char_send_ban_list_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_chr_mmo_char_send_slots_info(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_mmo_char_send_slots_info_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_send_slots_info_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_mmo_char_send_slots_info_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.mmo_char_send_slots_info(fd, sd);
	}
	if (HPMHooks.count.HP_chr_mmo_char_send_slots_info_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_send_slots_info_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_mmo_char_send_slots_info_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
int HP_chr_mmo_char_send_characters(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_mmo_char_send_characters_pre > 0) {
		int (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_send_characters_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_mmo_char_send_characters_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.mmo_char_send_characters(fd, sd);
	}
	if (HPMHooks.count.HP_chr_mmo_char_send_characters_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_send_characters_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_mmo_char_send_characters_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, sd);
		}
	}
	return retVal___;
}
int HP_chr_char_married(int pl1, int pl2) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_char_married_pre > 0) {
		int (*preHookFunc) (int *pl1, int *pl2);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_char_married_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_char_married_pre[hIndex].func;
			retVal___ = preHookFunc(&pl1, &pl2);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.char_married(pl1, pl2);
	}
	if (HPMHooks.count.HP_chr_char_married_post > 0) {
		int (*postHookFunc) (int retVal___, int pl1, int pl2);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_char_married_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_char_married_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, pl1, pl2);
		}
	}
	return retVal___;
}
int HP_chr_char_child(int parent_id, int child_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_char_child_pre > 0) {
		int (*preHookFunc) (int *parent_id, int *child_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_char_child_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_char_child_pre[hIndex].func;
			retVal___ = preHookFunc(&parent_id, &child_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.char_child(parent_id, child_id);
	}
	if (HPMHooks.count.HP_chr_char_child_post > 0) {
		int (*postHookFunc) (int retVal___, int parent_id, int child_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_char_child_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_char_child_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent_id, child_id);
		}
	}
	return retVal___;
}
int HP_chr_char_family(int cid1, int cid2, int cid3) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_char_family_pre > 0) {
		int (*preHookFunc) (int *cid1, int *cid2, int *cid3);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_char_family_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_char_family_pre[hIndex].func;
			retVal___ = preHookFunc(&cid1, &cid2, &cid3);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.char_family(cid1, cid2, cid3);
	}
	if (HPMHooks.count.HP_chr_char_family_post > 0) {
		int (*postHookFunc) (int retVal___, int cid1, int cid2, int cid3);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_char_family_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_char_family_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, cid1, cid2, cid3);
		}
	}
	return retVal___;
}
void HP_chr_disconnect_player(int account_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_disconnect_player_pre > 0) {
		void (*preHookFunc) (int *account_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_disconnect_player_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_disconnect_player_pre[hIndex].func;
			preHookFunc(&account_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.disconnect_player(account_id);
	}
	if (HPMHooks.count.HP_chr_disconnect_player_post > 0) {
		void (*postHookFunc) (int account_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_disconnect_player_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_disconnect_player_post[hIndex].func;
			postHookFunc(account_id);
		}
	}
	return;
}
void HP_chr_authfail_fd(int fd, int type) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_authfail_fd_pre > 0) {
		void (*preHookFunc) (int *fd, int *type);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_authfail_fd_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_authfail_fd_pre[hIndex].func;
			preHookFunc(&fd, &type);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.authfail_fd(fd, type);
	}
	if (HPMHooks.count.HP_chr_authfail_fd_post > 0) {
		void (*postHookFunc) (int fd, int type);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_authfail_fd_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_authfail_fd_post[hIndex].func;
			postHookFunc(fd, type);
		}
	}
	return;
}
void HP_chr_request_account_data(int account_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_request_account_data_pre > 0) {
		void (*preHookFunc) (int *account_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_request_account_data_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_request_account_data_pre[hIndex].func;
			preHookFunc(&account_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.request_account_data(account_id);
	}
	if (HPMHooks.count.HP_chr_request_account_data_post > 0) {
		void (*postHookFunc) (int account_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_request_account_data_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_request_account_data_post[hIndex].func;
			postHookFunc(account_id);
		}
	}
	return;
}
void HP_chr_auth_ok(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_auth_ok_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_auth_ok_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_auth_ok_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.auth_ok(fd, sd);
	}
	if (HPMHooks.count.HP_chr_auth_ok_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_auth_ok_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_auth_ok_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_chr_ping_login_server(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_ping_login_server_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_ping_login_server_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_ping_login_server_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.ping_login_server(fd);
	}
	if (HPMHooks.count.HP_chr_ping_login_server_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_ping_login_server_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_ping_login_server_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
int HP_chr_parse_fromlogin_connection_state(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_parse_fromlogin_connection_state_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_connection_state_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_connection_state_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.parse_fromlogin_connection_state(fd);
	}
	if (HPMHooks.count.HP_chr_parse_fromlogin_connection_state_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_connection_state_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_connection_state_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
void HP_chr_auth_error(int fd, unsigned char flag) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_auth_error_pre > 0) {
		void (*preHookFunc) (int *fd, unsigned char *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_auth_error_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_auth_error_pre[hIndex].func;
			preHookFunc(&fd, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.auth_error(fd, flag);
	}
	if (HPMHooks.count.HP_chr_auth_error_post > 0) {
		void (*postHookFunc) (int fd, unsigned char flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_auth_error_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_auth_error_post[hIndex].func;
			postHookFunc(fd, flag);
		}
	}
	return;
}
void HP_chr_parse_fromlogin_auth_state(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_fromlogin_auth_state_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_auth_state_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_auth_state_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_fromlogin_auth_state(fd);
	}
	if (HPMHooks.count.HP_chr_parse_fromlogin_auth_state_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_auth_state_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_auth_state_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_parse_fromlogin_account_data(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_fromlogin_account_data_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_account_data_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_account_data_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_fromlogin_account_data(fd);
	}
	if (HPMHooks.count.HP_chr_parse_fromlogin_account_data_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_account_data_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_account_data_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_parse_fromlogin_login_pong(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_fromlogin_login_pong_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_login_pong_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_login_pong_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_fromlogin_login_pong(fd);
	}
	if (HPMHooks.count.HP_chr_parse_fromlogin_login_pong_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_login_pong_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_login_pong_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_changesex(int account_id, int sex) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_changesex_pre > 0) {
		void (*preHookFunc) (int *account_id, int *sex);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_changesex_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_changesex_pre[hIndex].func;
			preHookFunc(&account_id, &sex);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.changesex(account_id, sex);
	}
	if (HPMHooks.count.HP_chr_changesex_post > 0) {
		void (*postHookFunc) (int account_id, int sex);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_changesex_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_changesex_post[hIndex].func;
			postHookFunc(account_id, sex);
		}
	}
	return;
}
int HP_chr_parse_fromlogin_changesex_reply(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_parse_fromlogin_changesex_reply_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_changesex_reply_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_changesex_reply_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.parse_fromlogin_changesex_reply(fd);
	}
	if (HPMHooks.count.HP_chr_parse_fromlogin_changesex_reply_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_changesex_reply_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_changesex_reply_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
void HP_chr_parse_fromlogin_account_reg2(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_fromlogin_account_reg2_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_account_reg2_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_account_reg2_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_fromlogin_account_reg2(fd);
	}
	if (HPMHooks.count.HP_chr_parse_fromlogin_account_reg2_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_account_reg2_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_account_reg2_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_parse_fromlogin_ban(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_fromlogin_ban_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_ban_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_ban_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_fromlogin_ban(fd);
	}
	if (HPMHooks.count.HP_chr_parse_fromlogin_ban_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_ban_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_ban_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_parse_fromlogin_kick(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_fromlogin_kick_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_kick_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_kick_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_fromlogin_kick(fd);
	}
	if (HPMHooks.count.HP_chr_parse_fromlogin_kick_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_kick_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_kick_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_update_ip(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_update_ip_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_update_ip_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_update_ip_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.update_ip(fd);
	}
	if (HPMHooks.count.HP_chr_update_ip_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_update_ip_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_update_ip_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_parse_fromlogin_update_ip(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_fromlogin_update_ip_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_update_ip_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_update_ip_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_fromlogin_update_ip(fd);
	}
	if (HPMHooks.count.HP_chr_parse_fromlogin_update_ip_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_update_ip_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_update_ip_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_parse_fromlogin_accinfo2_failed(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_fromlogin_accinfo2_failed_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_accinfo2_failed_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_accinfo2_failed_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_fromlogin_accinfo2_failed(fd);
	}
	if (HPMHooks.count.HP_chr_parse_fromlogin_accinfo2_failed_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_accinfo2_failed_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_accinfo2_failed_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_parse_fromlogin_accinfo2_ok(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_fromlogin_accinfo2_ok_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_accinfo2_ok_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_accinfo2_ok_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_fromlogin_accinfo2_ok(fd);
	}
	if (HPMHooks.count.HP_chr_parse_fromlogin_accinfo2_ok_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_accinfo2_ok_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_accinfo2_ok_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
int HP_chr_parse_fromlogin(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_parse_fromlogin_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.parse_fromlogin(fd);
	}
	if (HPMHooks.count.HP_chr_parse_fromlogin_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_chr_request_accreg2(int account_id, int char_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_request_accreg2_pre > 0) {
		int (*preHookFunc) (int *account_id, int *char_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_request_accreg2_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_request_accreg2_pre[hIndex].func;
			retVal___ = preHookFunc(&account_id, &char_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.request_accreg2(account_id, char_id);
	}
	if (HPMHooks.count.HP_chr_request_accreg2_post > 0) {
		int (*postHookFunc) (int retVal___, int account_id, int char_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_request_accreg2_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_request_accreg2_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, account_id, char_id);
		}
	}
	return retVal___;
}
void HP_chr_global_accreg_to_login_start(int account_id, int char_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_global_accreg_to_login_start_pre > 0) {
		void (*preHookFunc) (int *account_id, int *char_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_global_accreg_to_login_start_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_global_accreg_to_login_start_pre[hIndex].func;
			preHookFunc(&account_id, &char_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.global_accreg_to_login_start(account_id, char_id);
	}
	if (HPMHooks.count.HP_chr_global_accreg_to_login_start_post > 0) {
		void (*postHookFunc) (int account_id, int char_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_global_accreg_to_login_start_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_global_accreg_to_login_start_post[hIndex].func;
			postHookFunc(account_id, char_id);
		}
	}
	return;
}
void HP_chr_global_accreg_to_login_send(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_global_accreg_to_login_send_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_global_accreg_to_login_send_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_global_accreg_to_login_send_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.global_accreg_to_login_send();
	}
	if (HPMHooks.count.HP_chr_global_accreg_to_login_send_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_global_accreg_to_login_send_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_global_accreg_to_login_send_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_chr_global_accreg_to_login_add(const char *key, unsigned int index, intptr_t val, bool is_string) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_global_accreg_to_login_add_pre > 0) {
		void (*preHookFunc) (const char **key, unsigned int *index, intptr_t *val, bool *is_string);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_global_accreg_to_login_add_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_global_accreg_to_login_add_pre[hIndex].func;
			preHookFunc(&key, &index, &val, &is_string);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.global_accreg_to_login_add(key, index, val, is_string);
	}
	if (HPMHooks.count.HP_chr_global_accreg_to_login_add_post > 0) {
		void (*postHookFunc) (const char *key, unsigned int index, intptr_t val, bool is_string);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_global_accreg_to_login_add_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_global_accreg_to_login_add_post[hIndex].func;
			postHookFunc(key, index, val, is_string);
		}
	}
	return;
}
void HP_chr_read_fame_list(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_read_fame_list_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_read_fame_list_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_read_fame_list_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.read_fame_list();
	}
	if (HPMHooks.count.HP_chr_read_fame_list_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_read_fame_list_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_read_fame_list_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_chr_send_fame_list(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_send_fame_list_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_fame_list_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_send_fame_list_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.send_fame_list(fd);
	}
	if (HPMHooks.count.HP_chr_send_fame_list_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_fame_list_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_send_fame_list_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
void HP_chr_update_fame_list(int type, int index, int fame) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_update_fame_list_pre > 0) {
		void (*preHookFunc) (int *type, int *index, int *fame);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_update_fame_list_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_update_fame_list_pre[hIndex].func;
			preHookFunc(&type, &index, &fame);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.update_fame_list(type, index, fame);
	}
	if (HPMHooks.count.HP_chr_update_fame_list_post > 0) {
		void (*postHookFunc) (int type, int index, int fame);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_update_fame_list_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_update_fame_list_post[hIndex].func;
			postHookFunc(type, index, fame);
		}
	}
	return;
}
int HP_chr_loadName(int char_id, char *name) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_loadName_pre > 0) {
		int (*preHookFunc) (int *char_id, char **name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_loadName_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_loadName_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.loadName(char_id, name);
	}
	if (HPMHooks.count.HP_chr_loadName_post > 0) {
		int (*postHookFunc) (int retVal___, int char_id, char *name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_loadName_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_loadName_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, char_id, name);
		}
	}
	return retVal___;
}
void HP_chr_parse_frommap_datasync(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_datasync_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_datasync_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_datasync_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_datasync(fd);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_datasync_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_datasync_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_datasync_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_skillid2idx(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_skillid2idx_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_skillid2idx_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_skillid2idx_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_skillid2idx(fd);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_skillid2idx_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_skillid2idx_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_skillid2idx_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_map_received_ok(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_map_received_ok_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_map_received_ok_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_map_received_ok_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.map_received_ok(fd);
	}
	if (HPMHooks.count.HP_chr_map_received_ok_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_map_received_ok_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_map_received_ok_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_send_maps(int fd, int id, int j) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_send_maps_pre > 0) {
		void (*preHookFunc) (int *fd, int *id, int *j);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_maps_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_send_maps_pre[hIndex].func;
			preHookFunc(&fd, &id, &j);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.send_maps(fd, id, j);
	}
	if (HPMHooks.count.HP_chr_send_maps_post > 0) {
		void (*postHookFunc) (int fd, int id, int j);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_maps_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_send_maps_post[hIndex].func;
			postHookFunc(fd, id, j);
		}
	}
	return;
}
void HP_chr_parse_frommap_map_names(int fd, int id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_map_names_pre > 0) {
		void (*preHookFunc) (int *fd, int *id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_map_names_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_map_names_pre[hIndex].func;
			preHookFunc(&fd, &id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_map_names(fd, id);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_map_names_post > 0) {
		void (*postHookFunc) (int fd, int id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_map_names_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_map_names_post[hIndex].func;
			postHookFunc(fd, id);
		}
	}
	return;
}
void HP_chr_send_scdata(int fd, int aid, int cid) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_send_scdata_pre > 0) {
		void (*preHookFunc) (int *fd, int *aid, int *cid);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_scdata_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_send_scdata_pre[hIndex].func;
			preHookFunc(&fd, &aid, &cid);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.send_scdata(fd, aid, cid);
	}
	if (HPMHooks.count.HP_chr_send_scdata_post > 0) {
		void (*postHookFunc) (int fd, int aid, int cid);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_scdata_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_send_scdata_post[hIndex].func;
			postHookFunc(fd, aid, cid);
		}
	}
	return;
}
void HP_chr_parse_frommap_request_scdata(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_request_scdata_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_request_scdata_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_request_scdata_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_request_scdata(fd);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_request_scdata_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_request_scdata_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_request_scdata_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_set_users_count(int fd, int id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_set_users_count_pre > 0) {
		void (*preHookFunc) (int *fd, int *id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_set_users_count_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_set_users_count_pre[hIndex].func;
			preHookFunc(&fd, &id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_set_users_count(fd, id);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_set_users_count_post > 0) {
		void (*postHookFunc) (int fd, int id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_set_users_count_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_set_users_count_post[hIndex].func;
			postHookFunc(fd, id);
		}
	}
	return;
}
void HP_chr_parse_frommap_set_users(int fd, int id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_set_users_pre > 0) {
		void (*preHookFunc) (int *fd, int *id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_set_users_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_set_users_pre[hIndex].func;
			preHookFunc(&fd, &id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_set_users(fd, id);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_set_users_post > 0) {
		void (*postHookFunc) (int fd, int id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_set_users_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_set_users_post[hIndex].func;
			postHookFunc(fd, id);
		}
	}
	return;
}
void HP_chr_save_character_ack(int fd, int aid, int cid) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_save_character_ack_pre > 0) {
		void (*preHookFunc) (int *fd, int *aid, int *cid);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_save_character_ack_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_save_character_ack_pre[hIndex].func;
			preHookFunc(&fd, &aid, &cid);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.save_character_ack(fd, aid, cid);
	}
	if (HPMHooks.count.HP_chr_save_character_ack_post > 0) {
		void (*postHookFunc) (int fd, int aid, int cid);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_save_character_ack_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_save_character_ack_post[hIndex].func;
			postHookFunc(fd, aid, cid);
		}
	}
	return;
}
void HP_chr_parse_frommap_save_character(int fd, int id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_save_character_pre > 0) {
		void (*preHookFunc) (int *fd, int *id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_save_character_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_save_character_pre[hIndex].func;
			preHookFunc(&fd, &id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_save_character(fd, id);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_save_character_post > 0) {
		void (*postHookFunc) (int fd, int id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_save_character_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_save_character_post[hIndex].func;
			postHookFunc(fd, id);
		}
	}
	return;
}
void HP_chr_select_ack(int fd, int account_id, uint8 flag) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_select_ack_pre > 0) {
		void (*preHookFunc) (int *fd, int *account_id, uint8 *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_select_ack_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_select_ack_pre[hIndex].func;
			preHookFunc(&fd, &account_id, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.select_ack(fd, account_id, flag);
	}
	if (HPMHooks.count.HP_chr_select_ack_post > 0) {
		void (*postHookFunc) (int fd, int account_id, uint8 flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_select_ack_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_select_ack_post[hIndex].func;
			postHookFunc(fd, account_id, flag);
		}
	}
	return;
}
void HP_chr_parse_frommap_char_select_req(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_char_select_req_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_char_select_req_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_char_select_req_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_char_select_req(fd);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_char_select_req_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_char_select_req_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_char_select_req_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_change_map_server_ack(int fd, const uint8 *data, bool ok) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_change_map_server_ack_pre > 0) {
		void (*preHookFunc) (int *fd, const uint8 **data, bool *ok);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_change_map_server_ack_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_change_map_server_ack_pre[hIndex].func;
			preHookFunc(&fd, &data, &ok);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.change_map_server_ack(fd, data, ok);
	}
	if (HPMHooks.count.HP_chr_change_map_server_ack_post > 0) {
		void (*postHookFunc) (int fd, const uint8 *data, bool ok);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_change_map_server_ack_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_change_map_server_ack_post[hIndex].func;
			postHookFunc(fd, data, ok);
		}
	}
	return;
}
void HP_chr_parse_frommap_change_map_server(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_change_map_server_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_change_map_server_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_change_map_server_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_change_map_server(fd);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_change_map_server_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_change_map_server_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_change_map_server_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_remove_friend(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_remove_friend_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_remove_friend_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_remove_friend_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_remove_friend(fd);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_remove_friend_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_remove_friend_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_remove_friend_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_char_name_ack(int fd, int char_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_char_name_ack_pre > 0) {
		void (*preHookFunc) (int *fd, int *char_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_char_name_ack_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_char_name_ack_pre[hIndex].func;
			preHookFunc(&fd, &char_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.char_name_ack(fd, char_id);
	}
	if (HPMHooks.count.HP_chr_char_name_ack_post > 0) {
		void (*postHookFunc) (int fd, int char_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_char_name_ack_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_char_name_ack_post[hIndex].func;
			postHookFunc(fd, char_id);
		}
	}
	return;
}
void HP_chr_parse_frommap_char_name_request(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_char_name_request_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_char_name_request_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_char_name_request_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_char_name_request(fd);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_char_name_request_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_char_name_request_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_char_name_request_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_change_email(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_change_email_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_change_email_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_change_email_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_change_email(fd);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_change_email_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_change_email_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_change_email_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_ban(int account_id, int char_id, time_t *unban_time, short year, short month, short day, short hour, short minute, short second) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_ban_pre > 0) {
		void (*preHookFunc) (int *account_id, int *char_id, time_t **unban_time, short *year, short *month, short *day, short *hour, short *minute, short *second);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_ban_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_ban_pre[hIndex].func;
			preHookFunc(&account_id, &char_id, &unban_time, &year, &month, &day, &hour, &minute, &second);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.ban(account_id, char_id, unban_time, year, month, day, hour, minute, second);
	}
	if (HPMHooks.count.HP_chr_ban_post > 0) {
		void (*postHookFunc) (int account_id, int char_id, time_t *unban_time, short year, short month, short day, short hour, short minute, short second);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_ban_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_ban_post[hIndex].func;
			postHookFunc(account_id, char_id, unban_time, year, month, day, hour, minute, second);
		}
	}
	return;
}
void HP_chr_unban(int char_id, int *result) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_unban_pre > 0) {
		void (*preHookFunc) (int *char_id, int **result);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_unban_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_unban_pre[hIndex].func;
			preHookFunc(&char_id, &result);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.unban(char_id, result);
	}
	if (HPMHooks.count.HP_chr_unban_post > 0) {
		void (*postHookFunc) (int char_id, int *result);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_unban_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_unban_post[hIndex].func;
			postHookFunc(char_id, result);
		}
	}
	return;
}
void HP_chr_ask_name_ack(int fd, int acc, const char *name, int type, int result) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_ask_name_ack_pre > 0) {
		void (*preHookFunc) (int *fd, int *acc, const char **name, int *type, int *result);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_ask_name_ack_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_ask_name_ack_pre[hIndex].func;
			preHookFunc(&fd, &acc, &name, &type, &result);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.ask_name_ack(fd, acc, name, type, result);
	}
	if (HPMHooks.count.HP_chr_ask_name_ack_post > 0) {
		void (*postHookFunc) (int fd, int acc, const char *name, int type, int result);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_ask_name_ack_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_ask_name_ack_post[hIndex].func;
			postHookFunc(fd, acc, name, type, result);
		}
	}
	return;
}
int HP_chr_changecharsex(int char_id, int sex) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_changecharsex_pre > 0) {
		int (*preHookFunc) (int *char_id, int *sex);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_changecharsex_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_changecharsex_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &sex);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.changecharsex(char_id, sex);
	}
	if (HPMHooks.count.HP_chr_changecharsex_post > 0) {
		int (*postHookFunc) (int retVal___, int char_id, int sex);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_changecharsex_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_changecharsex_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, char_id, sex);
		}
	}
	return retVal___;
}
void HP_chr_parse_frommap_change_account(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_change_account_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_change_account_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_change_account_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_change_account(fd);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_change_account_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_change_account_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_change_account_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_fame_list(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_fame_list_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_fame_list_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_fame_list_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_fame_list(fd);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_fame_list_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_fame_list_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_fame_list_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_divorce_char(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_divorce_char_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_divorce_char_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_divorce_char_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_divorce_char(fd);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_divorce_char_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_divorce_char_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_divorce_char_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_ragsrvinfo(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_ragsrvinfo_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_ragsrvinfo_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_ragsrvinfo_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_ragsrvinfo(fd);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_ragsrvinfo_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_ragsrvinfo_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_ragsrvinfo_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_set_char_offline(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_set_char_offline_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_set_char_offline_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_set_char_offline_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_set_char_offline(fd);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_set_char_offline_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_set_char_offline_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_set_char_offline_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_set_all_offline(int fd, int id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_set_all_offline_pre > 0) {
		void (*preHookFunc) (int *fd, int *id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_set_all_offline_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_set_all_offline_pre[hIndex].func;
			preHookFunc(&fd, &id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_set_all_offline(fd, id);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_set_all_offline_post > 0) {
		void (*postHookFunc) (int fd, int id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_set_all_offline_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_set_all_offline_post[hIndex].func;
			postHookFunc(fd, id);
		}
	}
	return;
}
void HP_chr_parse_frommap_set_char_online(int fd, int id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_set_char_online_pre > 0) {
		void (*preHookFunc) (int *fd, int *id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_set_char_online_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_set_char_online_pre[hIndex].func;
			preHookFunc(&fd, &id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_set_char_online(fd, id);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_set_char_online_post > 0) {
		void (*postHookFunc) (int fd, int id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_set_char_online_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_set_char_online_post[hIndex].func;
			postHookFunc(fd, id);
		}
	}
	return;
}
void HP_chr_parse_frommap_build_fame_list(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_build_fame_list_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_build_fame_list_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_build_fame_list_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_build_fame_list(fd);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_build_fame_list_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_build_fame_list_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_build_fame_list_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_save_status_change_data(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_save_status_change_data_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_save_status_change_data_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_save_status_change_data_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_save_status_change_data(fd);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_save_status_change_data_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_save_status_change_data_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_save_status_change_data_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_send_pong(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_send_pong_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_pong_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_send_pong_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.send_pong(fd);
	}
	if (HPMHooks.count.HP_chr_send_pong_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_pong_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_send_pong_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_ping(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_ping_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_ping_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_ping_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_ping(fd);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_ping_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_ping_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_ping_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_map_auth_ok(int fd, int account_id, struct char_auth_node *node, struct mmo_charstatus *cd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_map_auth_ok_pre > 0) {
		void (*preHookFunc) (int *fd, int *account_id, struct char_auth_node **node, struct mmo_charstatus **cd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_map_auth_ok_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_map_auth_ok_pre[hIndex].func;
			preHookFunc(&fd, &account_id, &node, &cd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.map_auth_ok(fd, account_id, node, cd);
	}
	if (HPMHooks.count.HP_chr_map_auth_ok_post > 0) {
		void (*postHookFunc) (int fd, int account_id, struct char_auth_node *node, struct mmo_charstatus *cd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_map_auth_ok_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_map_auth_ok_post[hIndex].func;
			postHookFunc(fd, account_id, node, cd);
		}
	}
	return;
}
void HP_chr_map_auth_failed(int fd, int account_id, int char_id, int login_id1, char sex, uint32 ip) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_map_auth_failed_pre > 0) {
		void (*preHookFunc) (int *fd, int *account_id, int *char_id, int *login_id1, char *sex, uint32 *ip);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_map_auth_failed_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_map_auth_failed_pre[hIndex].func;
			preHookFunc(&fd, &account_id, &char_id, &login_id1, &sex, &ip);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.map_auth_failed(fd, account_id, char_id, login_id1, sex, ip);
	}
	if (HPMHooks.count.HP_chr_map_auth_failed_post > 0) {
		void (*postHookFunc) (int fd, int account_id, int char_id, int login_id1, char sex, uint32 ip);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_map_auth_failed_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_map_auth_failed_post[hIndex].func;
			postHookFunc(fd, account_id, char_id, login_id1, sex, ip);
		}
	}
	return;
}
void HP_chr_parse_frommap_auth_request(int fd, int id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_auth_request_pre > 0) {
		void (*preHookFunc) (int *fd, int *id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_auth_request_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_auth_request_pre[hIndex].func;
			preHookFunc(&fd, &id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_auth_request(fd, id);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_auth_request_post > 0) {
		void (*postHookFunc) (int fd, int id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_auth_request_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_auth_request_post[hIndex].func;
			postHookFunc(fd, id);
		}
	}
	return;
}
void HP_chr_parse_frommap_update_ip(int fd, int id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_update_ip_pre > 0) {
		void (*preHookFunc) (int *fd, int *id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_update_ip_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_update_ip_pre[hIndex].func;
			preHookFunc(&fd, &id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_update_ip(fd, id);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_update_ip_post > 0) {
		void (*postHookFunc) (int fd, int id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_update_ip_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_update_ip_post[hIndex].func;
			postHookFunc(fd, id);
		}
	}
	return;
}
void HP_chr_parse_frommap_request_stats_report(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_request_stats_report_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_request_stats_report_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_request_stats_report_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_request_stats_report(fd);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_request_stats_report_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_request_stats_report_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_request_stats_report_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_scdata_update(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_scdata_update_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_scdata_update_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_scdata_update_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_scdata_update(fd);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_scdata_update_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_scdata_update_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_scdata_update_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_scdata_delete(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_scdata_delete_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_scdata_delete_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_scdata_delete_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_scdata_delete(fd);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_scdata_delete_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_scdata_delete_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_scdata_delete_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
int HP_chr_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_parse_frommap_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.parse_frommap(fd);
	}
	if (HPMHooks.count.HP_chr_parse_frommap_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_chr_search_mapserver(unsigned short map, uint32 ip, uint16 port) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_search_mapserver_pre > 0) {
		int (*preHookFunc) (unsigned short *map, uint32 *ip, uint16 *port);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_search_mapserver_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_search_mapserver_pre[hIndex].func;
			retVal___ = preHookFunc(&map, &ip, &port);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.search_mapserver(map, ip, port);
	}
	if (HPMHooks.count.HP_chr_search_mapserver_post > 0) {
		int (*postHookFunc) (int retVal___, unsigned short map, uint32 ip, uint16 port);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_search_mapserver_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_search_mapserver_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, map, ip, port);
		}
	}
	return retVal___;
}
int HP_chr_mapif_init(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_mapif_init_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_mapif_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_mapif_init_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.mapif_init(fd);
	}
	if (HPMHooks.count.HP_chr_mapif_init_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_mapif_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_mapif_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
uint32 HP_chr_lan_subnet_check(uint32 ip) {
	int hIndex = 0;
	uint32 retVal___ = 0;
	if (HPMHooks.count.HP_chr_lan_subnet_check_pre > 0) {
		uint32 (*preHookFunc) (uint32 *ip);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_lan_subnet_check_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_lan_subnet_check_pre[hIndex].func;
			retVal___ = preHookFunc(&ip);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.lan_subnet_check(ip);
	}
	if (HPMHooks.count.HP_chr_lan_subnet_check_post > 0) {
		uint32 (*postHookFunc) (uint32 retVal___, uint32 ip);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_lan_subnet_check_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_lan_subnet_check_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, ip);
		}
	}
	return retVal___;
}
void HP_chr_delete2_ack(int fd, int char_id, uint32 result, time_t delete_date) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_delete2_ack_pre > 0) {
		void (*preHookFunc) (int *fd, int *char_id, uint32 *result, time_t *delete_date);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_ack_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_delete2_ack_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &result, &delete_date);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.delete2_ack(fd, char_id, result, delete_date);
	}
	if (HPMHooks.count.HP_chr_delete2_ack_post > 0) {
		void (*postHookFunc) (int fd, int char_id, uint32 result, time_t delete_date);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_ack_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_delete2_ack_post[hIndex].func;
			postHookFunc(fd, char_id, result, delete_date);
		}
	}
	return;
}
void HP_chr_delete2_accept_actual_ack(int fd, int char_id, uint32 result) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_delete2_accept_actual_ack_pre > 0) {
		void (*preHookFunc) (int *fd, int *char_id, uint32 *result);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_accept_actual_ack_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_delete2_accept_actual_ack_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &result);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.delete2_accept_actual_ack(fd, char_id, result);
	}
	if (HPMHooks.count.HP_chr_delete2_accept_actual_ack_post > 0) {
		void (*postHookFunc) (int fd, int char_id, uint32 result);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_accept_actual_ack_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_delete2_accept_actual_ack_post[hIndex].func;
			postHookFunc(fd, char_id, result);
		}
	}
	return;
}
void HP_chr_delete2_accept_ack(int fd, int char_id, uint32 result) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_delete2_accept_ack_pre > 0) {
		void (*preHookFunc) (int *fd, int *char_id, uint32 *result);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_accept_ack_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_delete2_accept_ack_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &result);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.delete2_accept_ack(fd, char_id, result);
	}
	if (HPMHooks.count.HP_chr_delete2_accept_ack_post > 0) {
		void (*postHookFunc) (int fd, int char_id, uint32 result);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_accept_ack_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_delete2_accept_ack_post[hIndex].func;
			postHookFunc(fd, char_id, result);
		}
	}
	return;
}
void HP_chr_delete2_cancel_ack(int fd, int char_id, uint32 result) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_delete2_cancel_ack_pre > 0) {
		void (*preHookFunc) (int *fd, int *char_id, uint32 *result);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_cancel_ack_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_delete2_cancel_ack_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &result);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.delete2_cancel_ack(fd, char_id, result);
	}
	if (HPMHooks.count.HP_chr_delete2_cancel_ack_post > 0) {
		void (*postHookFunc) (int fd, int char_id, uint32 result);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_cancel_ack_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_delete2_cancel_ack_post[hIndex].func;
			postHookFunc(fd, char_id, result);
		}
	}
	return;
}
void HP_chr_delete2_req(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_delete2_req_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_req_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_delete2_req_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.delete2_req(fd, sd);
	}
	if (HPMHooks.count.HP_chr_delete2_req_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_req_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_delete2_req_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_chr_delete2_accept(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_delete2_accept_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_accept_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_delete2_accept_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.delete2_accept(fd, sd);
	}
	if (HPMHooks.count.HP_chr_delete2_accept_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_accept_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_delete2_accept_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_chr_delete2_cancel(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_delete2_cancel_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_cancel_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_delete2_cancel_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.delete2_cancel(fd, sd);
	}
	if (HPMHooks.count.HP_chr_delete2_cancel_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_cancel_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_delete2_cancel_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_chr_send_account_id(int fd, int account_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_send_account_id_pre > 0) {
		void (*preHookFunc) (int *fd, int *account_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_account_id_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_send_account_id_pre[hIndex].func;
			preHookFunc(&fd, &account_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.send_account_id(fd, account_id);
	}
	if (HPMHooks.count.HP_chr_send_account_id_post > 0) {
		void (*postHookFunc) (int fd, int account_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_account_id_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_send_account_id_post[hIndex].func;
			postHookFunc(fd, account_id);
		}
	}
	return;
}
void HP_chr_parse_char_connect(int fd, struct char_session_data *sd, uint32 ipl) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_char_connect_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd, uint32 *ipl);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_connect_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_connect_pre[hIndex].func;
			preHookFunc(&fd, &sd, &ipl);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_connect(fd, sd, ipl);
	}
	if (HPMHooks.count.HP_chr_parse_char_connect_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd, uint32 ipl);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_connect_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_connect_post[hIndex].func;
			postHookFunc(fd, sd, ipl);
		}
	}
	return;
}
void HP_chr_send_map_info(int fd, int i, uint32 subnet_map_ip, struct mmo_charstatus *cd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_send_map_info_pre > 0) {
		void (*preHookFunc) (int *fd, int *i, uint32 *subnet_map_ip, struct mmo_charstatus **cd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_map_info_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_send_map_info_pre[hIndex].func;
			preHookFunc(&fd, &i, &subnet_map_ip, &cd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.send_map_info(fd, i, subnet_map_ip, cd);
	}
	if (HPMHooks.count.HP_chr_send_map_info_post > 0) {
		void (*postHookFunc) (int fd, int i, uint32 subnet_map_ip, struct mmo_charstatus *cd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_map_info_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_send_map_info_post[hIndex].func;
			postHookFunc(fd, i, subnet_map_ip, cd);
		}
	}
	return;
}
void HP_chr_send_wait_char_server(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_send_wait_char_server_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_wait_char_server_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_send_wait_char_server_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.send_wait_char_server(fd);
	}
	if (HPMHooks.count.HP_chr_send_wait_char_server_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_wait_char_server_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_send_wait_char_server_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
int HP_chr_search_default_maps_mapserver(struct mmo_charstatus *cd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_search_default_maps_mapserver_pre > 0) {
		int (*preHookFunc) (struct mmo_charstatus **cd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_search_default_maps_mapserver_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_search_default_maps_mapserver_pre[hIndex].func;
			retVal___ = preHookFunc(&cd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.search_default_maps_mapserver(cd);
	}
	if (HPMHooks.count.HP_chr_search_default_maps_mapserver_post > 0) {
		int (*postHookFunc) (int retVal___, struct mmo_charstatus *cd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_search_default_maps_mapserver_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_search_default_maps_mapserver_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, cd);
		}
	}
	return retVal___;
}
void HP_chr_parse_char_select(int fd, struct char_session_data *sd, uint32 ipl) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_char_select_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd, uint32 *ipl);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_select_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_select_pre[hIndex].func;
			preHookFunc(&fd, &sd, &ipl);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_select(fd, sd, ipl);
	}
	if (HPMHooks.count.HP_chr_parse_char_select_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd, uint32 ipl);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_select_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_select_post[hIndex].func;
			postHookFunc(fd, sd, ipl);
		}
	}
	return;
}
void HP_chr_creation_failed(int fd, int result) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_creation_failed_pre > 0) {
		void (*preHookFunc) (int *fd, int *result);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_creation_failed_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_creation_failed_pre[hIndex].func;
			preHookFunc(&fd, &result);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.creation_failed(fd, result);
	}
	if (HPMHooks.count.HP_chr_creation_failed_post > 0) {
		void (*postHookFunc) (int fd, int result);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_creation_failed_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_creation_failed_post[hIndex].func;
			postHookFunc(fd, result);
		}
	}
	return;
}
void HP_chr_creation_ok(int fd, struct mmo_charstatus *char_dat) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_creation_ok_pre > 0) {
		void (*preHookFunc) (int *fd, struct mmo_charstatus **char_dat);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_creation_ok_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_creation_ok_pre[hIndex].func;
			preHookFunc(&fd, &char_dat);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.creation_ok(fd, char_dat);
	}
	if (HPMHooks.count.HP_chr_creation_ok_post > 0) {
		void (*postHookFunc) (int fd, struct mmo_charstatus *char_dat);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_creation_ok_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_creation_ok_post[hIndex].func;
			postHookFunc(fd, char_dat);
		}
	}
	return;
}
void HP_chr_parse_char_create_new_char(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_char_create_new_char_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_create_new_char_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_create_new_char_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_create_new_char(fd, sd);
	}
	if (HPMHooks.count.HP_chr_parse_char_create_new_char_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_create_new_char_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_create_new_char_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_chr_delete_char_failed(int fd, int flag) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_delete_char_failed_pre > 0) {
		void (*preHookFunc) (int *fd, int *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete_char_failed_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_delete_char_failed_pre[hIndex].func;
			preHookFunc(&fd, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.delete_char_failed(fd, flag);
	}
	if (HPMHooks.count.HP_chr_delete_char_failed_post > 0) {
		void (*postHookFunc) (int fd, int flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete_char_failed_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_delete_char_failed_post[hIndex].func;
			postHookFunc(fd, flag);
		}
	}
	return;
}
void HP_chr_delete_char_ok(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_delete_char_ok_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete_char_ok_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_delete_char_ok_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.delete_char_ok(fd);
	}
	if (HPMHooks.count.HP_chr_delete_char_ok_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete_char_ok_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_delete_char_ok_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_parse_char_delete_char(int fd, struct char_session_data *sd, unsigned short cmd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_char_delete_char_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd, unsigned short *cmd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_delete_char_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_delete_char_pre[hIndex].func;
			preHookFunc(&fd, &sd, &cmd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_delete_char(fd, sd, cmd);
	}
	if (HPMHooks.count.HP_chr_parse_char_delete_char_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd, unsigned short cmd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_delete_char_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_delete_char_post[hIndex].func;
			postHookFunc(fd, sd, cmd);
		}
	}
	return;
}
void HP_chr_parse_char_ping(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_char_ping_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_ping_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_ping_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_ping(fd);
	}
	if (HPMHooks.count.HP_chr_parse_char_ping_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_ping_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_ping_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_allow_rename(int fd, int flag) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_allow_rename_pre > 0) {
		void (*preHookFunc) (int *fd, int *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_allow_rename_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_allow_rename_pre[hIndex].func;
			preHookFunc(&fd, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.allow_rename(fd, flag);
	}
	if (HPMHooks.count.HP_chr_allow_rename_post > 0) {
		void (*postHookFunc) (int fd, int flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_allow_rename_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_allow_rename_post[hIndex].func;
			postHookFunc(fd, flag);
		}
	}
	return;
}
void HP_chr_parse_char_rename_char(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_char_rename_char_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_rename_char_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_rename_char_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_rename_char(fd, sd);
	}
	if (HPMHooks.count.HP_chr_parse_char_rename_char_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_rename_char_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_rename_char_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_chr_parse_char_rename_char2(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_char_rename_char2_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_rename_char2_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_rename_char2_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_rename_char2(fd, sd);
	}
	if (HPMHooks.count.HP_chr_parse_char_rename_char2_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_rename_char2_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_rename_char2_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_chr_rename_char_ack(int fd, int flag) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_rename_char_ack_pre > 0) {
		void (*preHookFunc) (int *fd, int *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_rename_char_ack_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_rename_char_ack_pre[hIndex].func;
			preHookFunc(&fd, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.rename_char_ack(fd, flag);
	}
	if (HPMHooks.count.HP_chr_rename_char_ack_post > 0) {
		void (*postHookFunc) (int fd, int flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_rename_char_ack_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_rename_char_ack_post[hIndex].func;
			postHookFunc(fd, flag);
		}
	}
	return;
}
void HP_chr_parse_char_rename_char_confirm(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_char_rename_char_confirm_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_rename_char_confirm_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_rename_char_confirm_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_rename_char_confirm(fd, sd);
	}
	if (HPMHooks.count.HP_chr_parse_char_rename_char_confirm_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_rename_char_confirm_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_rename_char_confirm_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_chr_captcha_notsupported(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_captcha_notsupported_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_captcha_notsupported_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_captcha_notsupported_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.captcha_notsupported(fd);
	}
	if (HPMHooks.count.HP_chr_captcha_notsupported_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_captcha_notsupported_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_captcha_notsupported_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_parse_char_request_captcha(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_char_request_captcha_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_request_captcha_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_request_captcha_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_request_captcha(fd);
	}
	if (HPMHooks.count.HP_chr_parse_char_request_captcha_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_request_captcha_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_request_captcha_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_parse_char_check_captcha(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_char_check_captcha_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_check_captcha_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_check_captcha_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_check_captcha(fd);
	}
	if (HPMHooks.count.HP_chr_parse_char_check_captcha_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_check_captcha_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_check_captcha_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_chr_parse_char_delete2_req(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_char_delete2_req_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_delete2_req_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_delete2_req_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_delete2_req(fd, sd);
	}
	if (HPMHooks.count.HP_chr_parse_char_delete2_req_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_delete2_req_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_delete2_req_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_chr_parse_char_delete2_accept(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_char_delete2_accept_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_delete2_accept_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_delete2_accept_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_delete2_accept(fd, sd);
	}
	if (HPMHooks.count.HP_chr_parse_char_delete2_accept_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_delete2_accept_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_delete2_accept_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_chr_parse_char_delete2_cancel(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_char_delete2_cancel_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_delete2_cancel_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_delete2_cancel_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_delete2_cancel(fd, sd);
	}
	if (HPMHooks.count.HP_chr_parse_char_delete2_cancel_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_delete2_cancel_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_delete2_cancel_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_chr_login_map_server_ack(int fd, uint8 flag) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_login_map_server_ack_pre > 0) {
		void (*preHookFunc) (int *fd, uint8 *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_login_map_server_ack_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_login_map_server_ack_pre[hIndex].func;
			preHookFunc(&fd, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.login_map_server_ack(fd, flag);
	}
	if (HPMHooks.count.HP_chr_login_map_server_ack_post > 0) {
		void (*postHookFunc) (int fd, uint8 flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_login_map_server_ack_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_login_map_server_ack_post[hIndex].func;
			postHookFunc(fd, flag);
		}
	}
	return;
}
void HP_chr_parse_char_login_map_server(int fd, uint32 ipl) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_char_login_map_server_pre > 0) {
		void (*preHookFunc) (int *fd, uint32 *ipl);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_login_map_server_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_login_map_server_pre[hIndex].func;
			preHookFunc(&fd, &ipl);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_login_map_server(fd, ipl);
	}
	if (HPMHooks.count.HP_chr_parse_char_login_map_server_post > 0) {
		void (*postHookFunc) (int fd, uint32 ipl);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_login_map_server_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_login_map_server_post[hIndex].func;
			postHookFunc(fd, ipl);
		}
	}
	return;
}
void HP_chr_parse_char_pincode_check(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_char_pincode_check_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_pincode_check_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_pincode_check_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_pincode_check(fd, sd);
	}
	if (HPMHooks.count.HP_chr_parse_char_pincode_check_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_pincode_check_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_pincode_check_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_chr_parse_char_pincode_window(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_char_pincode_window_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_pincode_window_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_pincode_window_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_pincode_window(fd, sd);
	}
	if (HPMHooks.count.HP_chr_parse_char_pincode_window_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_pincode_window_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_pincode_window_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_chr_parse_char_pincode_change(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_char_pincode_change_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_pincode_change_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_pincode_change_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_pincode_change(fd, sd);
	}
	if (HPMHooks.count.HP_chr_parse_char_pincode_change_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_pincode_change_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_pincode_change_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_chr_parse_char_pincode_first_pin(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_char_pincode_first_pin_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_pincode_first_pin_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_pincode_first_pin_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_pincode_first_pin(fd, sd);
	}
	if (HPMHooks.count.HP_chr_parse_char_pincode_first_pin_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_pincode_first_pin_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_pincode_first_pin_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_chr_parse_char_request_chars(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_char_request_chars_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_request_chars_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_request_chars_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_request_chars(fd, sd);
	}
	if (HPMHooks.count.HP_chr_parse_char_request_chars_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_request_chars_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_request_chars_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_chr_change_character_slot_ack(int fd, bool ret) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_change_character_slot_ack_pre > 0) {
		void (*preHookFunc) (int *fd, bool *ret);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_change_character_slot_ack_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_change_character_slot_ack_pre[hIndex].func;
			preHookFunc(&fd, &ret);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.change_character_slot_ack(fd, ret);
	}
	if (HPMHooks.count.HP_chr_change_character_slot_ack_post > 0) {
		void (*postHookFunc) (int fd, bool ret);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_change_character_slot_ack_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_change_character_slot_ack_post[hIndex].func;
			postHookFunc(fd, ret);
		}
	}
	return;
}
void HP_chr_parse_char_move_character(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_parse_char_move_character_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_move_character_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_move_character_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_move_character(fd, sd);
	}
	if (HPMHooks.count.HP_chr_parse_char_move_character_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_move_character_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_move_character_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
int HP_chr_parse_char_unknown_packet(int fd, uint32 ipl) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_parse_char_unknown_packet_pre > 0) {
		int (*preHookFunc) (int *fd, uint32 *ipl);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_unknown_packet_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_unknown_packet_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &ipl);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.parse_char_unknown_packet(fd, ipl);
	}
	if (HPMHooks.count.HP_chr_parse_char_unknown_packet_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, uint32 ipl);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_unknown_packet_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_unknown_packet_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, ipl);
		}
	}
	return retVal___;
}
int HP_chr_parse_char(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_parse_char_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.parse_char(fd);
	}
	if (HPMHooks.count.HP_chr_parse_char_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_chr_broadcast_user_count(int tid, int64 tick, int id, intptr_t data) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_broadcast_user_count_pre > 0) {
		int (*preHookFunc) (int *tid, int64 *tick, int *id, intptr_t *data);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_broadcast_user_count_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_broadcast_user_count_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &tick, &id, &data);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.broadcast_user_count(tid, tick, id, data);
	}
	if (HPMHooks.count.HP_chr_broadcast_user_count_post > 0) {
		int (*postHookFunc) (int retVal___, int tid, int64 tick, int id, intptr_t data);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_broadcast_user_count_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_broadcast_user_count_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, tid, tick, id, data);
		}
	}
	return retVal___;
}
int HP_chr_send_accounts_tologin_sub(union DBKey key, struct DBData *data, va_list ap) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_send_accounts_tologin_sub_pre > 0) {
		int (*preHookFunc) (union DBKey *key, struct DBData **data, va_list ap);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_accounts_tologin_sub_pre; hIndex++) {
			va_list ap___copy; va_copy(ap___copy, ap);
			preHookFunc = HPMHooks.list.HP_chr_send_accounts_tologin_sub_pre[hIndex].func;
			retVal___ = preHookFunc(&key, &data, ap___copy);
			va_end(ap___copy);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		va_list ap___copy; va_copy(ap___copy, ap);
		retVal___ = HPMHooks.source.chr.send_accounts_tologin_sub(key, data, ap___copy);
		va_end(ap___copy);
	}
	if (HPMHooks.count.HP_chr_send_accounts_tologin_sub_post > 0) {
		int (*postHookFunc) (int retVal___, union DBKey key, struct DBData *data, va_list ap);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_accounts_tologin_sub_post; hIndex++) {
			va_list ap___copy; va_copy(ap___copy, ap);
			postHookFunc = HPMHooks.list.HP_chr_send_accounts_tologin_sub_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, key, data, ap___copy);
			va_end(ap___copy);
		}
	}
	return retVal___;
}
int HP_chr_send_accounts_tologin(int tid, int64 tick, int id, intptr_t data) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_send_accounts_tologin_pre > 0) {
		int (*preHookFunc) (int *tid, int64 *tick, int *id, intptr_t *data);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_accounts_tologin_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_send_accounts_tologin_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &tick, &id, &data);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.send_accounts_tologin(tid, tick, id, data);
	}
	if (HPMHooks.count.HP_chr_send_accounts_tologin_post > 0) {
		int (*postHookFunc) (int retVal___, int tid, int64 tick, int id, intptr_t data);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_accounts_tologin_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_send_accounts_tologin_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, tid, tick, id, data);
		}
	}
	return retVal___;
}
int HP_chr_check_connect_login_server(int tid, int64 tick, int id, intptr_t data) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_check_connect_login_server_pre > 0) {
		int (*preHookFunc) (int *tid, int64 *tick, int *id, intptr_t *data);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_check_connect_login_server_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_check_connect_login_server_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &tick, &id, &data);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.check_connect_login_server(tid, tick, id, data);
	}
	if (HPMHooks.count.HP_chr_check_connect_login_server_post > 0) {
		int (*postHookFunc) (int retVal___, int tid, int64 tick, int id, intptr_t data);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_check_connect_login_server_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_check_connect_login_server_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, tid, tick, id, data);
		}
	}
	return retVal___;
}
int HP_chr_online_data_cleanup_sub(union DBKey key, struct DBData *data, va_list ap) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_online_data_cleanup_sub_pre > 0) {
		int (*preHookFunc) (union DBKey *key, struct DBData **data, va_list ap);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_online_data_cleanup_sub_pre; hIndex++) {
			va_list ap___copy; va_copy(ap___copy, ap);
			preHookFunc = HPMHooks.list.HP_chr_online_data_cleanup_sub_pre[hIndex].func;
			retVal___ = preHookFunc(&key, &data, ap___copy);
			va_end(ap___copy);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		va_list ap___copy; va_copy(ap___copy, ap);
		retVal___ = HPMHooks.source.chr.online_data_cleanup_sub(key, data, ap___copy);
		va_end(ap___copy);
	}
	if (HPMHooks.count.HP_chr_online_data_cleanup_sub_post > 0) {
		int (*postHookFunc) (int retVal___, union DBKey key, struct DBData *data, va_list ap);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_online_data_cleanup_sub_post; hIndex++) {
			va_list ap___copy; va_copy(ap___copy, ap);
			postHookFunc = HPMHooks.list.HP_chr_online_data_cleanup_sub_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, key, data, ap___copy);
			va_end(ap___copy);
		}
	}
	return retVal___;
}
int HP_chr_online_data_cleanup(int tid, int64 tick, int id, intptr_t data) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_chr_online_data_cleanup_pre > 0) {
		int (*preHookFunc) (int *tid, int64 *tick, int *id, intptr_t *data);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_online_data_cleanup_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_online_data_cleanup_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &tick, &id, &data);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.online_data_cleanup(tid, tick, id, data);
	}
	if (HPMHooks.count.HP_chr_online_data_cleanup_post > 0) {
		int (*postHookFunc) (int retVal___, int tid, int64 tick, int id, intptr_t data);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_online_data_cleanup_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_online_data_cleanup_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, tid, tick, id, data);
		}
	}
	return retVal___;
}
bool HP_chr_sql_config_read(const char *filename, bool imported) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_chr_sql_config_read_pre > 0) {
		bool (*preHookFunc) (const char **filename, bool *imported);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_sql_config_read_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_sql_config_read_pre[hIndex].func;
			retVal___ = preHookFunc(&filename, &imported);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.sql_config_read(filename, imported);
	}
	if (HPMHooks.count.HP_chr_sql_config_read_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *filename, bool imported);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_sql_config_read_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_sql_config_read_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, filename, imported);
		}
	}
	return retVal___;
}
bool HP_chr_sql_config_read_registry(const char *filename, const struct config_t *config, bool imported) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_chr_sql_config_read_registry_pre > 0) {
		bool (*preHookFunc) (const char **filename, const struct config_t **config, bool *imported);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_sql_config_read_registry_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_sql_config_read_registry_pre[hIndex].func;
			retVal___ = preHookFunc(&filename, &config, &imported);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.sql_config_read_registry(filename, config, imported);
	}
	if (HPMHooks.count.HP_chr_sql_config_read_registry_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *filename, const struct config_t *config, bool imported);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_sql_config_read_registry_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_sql_config_read_registry_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, filename, config, imported);
		}
	}
	return retVal___;
}
bool HP_chr_sql_config_read_pc(const char *filename, const struct config_t *config, bool imported) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_chr_sql_config_read_pc_pre > 0) {
		bool (*preHookFunc) (const char **filename, const struct config_t **config, bool *imported);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_sql_config_read_pc_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_sql_config_read_pc_pre[hIndex].func;
			retVal___ = preHookFunc(&filename, &config, &imported);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.sql_config_read_pc(filename, config, imported);
	}
	if (HPMHooks.count.HP_chr_sql_config_read_pc_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *filename, const struct config_t *config, bool imported);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_sql_config_read_pc_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_sql_config_read_pc_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, filename, config, imported);
		}
	}
	return retVal___;
}
bool HP_chr_sql_config_read_guild(const char *filename, const struct config_t *config, bool imported) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_chr_sql_config_read_guild_pre > 0) {
		bool (*preHookFunc) (const char **filename, const struct config_t **config, bool *imported);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_sql_config_read_guild_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_sql_config_read_guild_pre[hIndex].func;
			retVal___ = preHookFunc(&filename, &config, &imported);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.sql_config_read_guild(filename, config, imported);
	}
	if (HPMHooks.count.HP_chr_sql_config_read_guild_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *filename, const struct config_t *config, bool imported);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_sql_config_read_guild_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_sql_config_read_guild_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, filename, config, imported);
		}
	}
	return retVal___;
}
bool HP_chr_config_read(const char *filename, bool imported) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_chr_config_read_pre > 0) {
		bool (*preHookFunc) (const char **filename, bool *imported);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_read_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_config_read_pre[hIndex].func;
			retVal___ = preHookFunc(&filename, &imported);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.config_read(filename, imported);
	}
	if (HPMHooks.count.HP_chr_config_read_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *filename, bool imported);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_read_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_config_read_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, filename, imported);
		}
	}
	return retVal___;
}
bool HP_chr_config_read_database(const char *filename, const struct config_t *config, bool imported) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_chr_config_read_database_pre > 0) {
		bool (*preHookFunc) (const char **filename, const struct config_t **config, bool *imported);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_read_database_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_config_read_database_pre[hIndex].func;
			retVal___ = preHookFunc(&filename, &config, &imported);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.config_read_database(filename, config, imported);
	}
	if (HPMHooks.count.HP_chr_config_read_database_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *filename, const struct config_t *config, bool imported);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_read_database_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_config_read_database_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, filename, config, imported);
		}
	}
	return retVal___;
}
bool HP_chr_config_read_console(const char *filename, const struct config_t *config, bool imported) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_chr_config_read_console_pre > 0) {
		bool (*preHookFunc) (const char **filename, const struct config_t **config, bool *imported);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_read_console_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_config_read_console_pre[hIndex].func;
			retVal___ = preHookFunc(&filename, &config, &imported);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.config_read_console(filename, config, imported);
	}
	if (HPMHooks.count.HP_chr_config_read_console_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *filename, const struct config_t *config, bool imported);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_read_console_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_config_read_console_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, filename, config, imported);
		}
	}
	return retVal___;
}
bool HP_chr_config_read_player_fame(const char *filename, const struct config_t *config, bool imported) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_chr_config_read_player_fame_pre > 0) {
		bool (*preHookFunc) (const char **filename, const struct config_t **config, bool *imported);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_read_player_fame_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_config_read_player_fame_pre[hIndex].func;
			retVal___ = preHookFunc(&filename, &config, &imported);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.config_read_player_fame(filename, config, imported);
	}
	if (HPMHooks.count.HP_chr_config_read_player_fame_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *filename, const struct config_t *config, bool imported);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_read_player_fame_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_config_read_player_fame_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, filename, config, imported);
		}
	}
	return retVal___;
}
bool HP_chr_config_read_player_deletion(const char *filename, const struct config_t *config, bool imported) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_chr_config_read_player_deletion_pre > 0) {
		bool (*preHookFunc) (const char **filename, const struct config_t **config, bool *imported);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_read_player_deletion_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_config_read_player_deletion_pre[hIndex].func;
			retVal___ = preHookFunc(&filename, &config, &imported);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.config_read_player_deletion(filename, config, imported);
	}
	if (HPMHooks.count.HP_chr_config_read_player_deletion_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *filename, const struct config_t *config, bool imported);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_read_player_deletion_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_config_read_player_deletion_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, filename, config, imported);
		}
	}
	return retVal___;
}
bool HP_chr_config_read_player_name(const char *filename, const struct config_t *config, bool imported) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_chr_config_read_player_name_pre > 0) {
		bool (*preHookFunc) (const char **filename, const struct config_t **config, bool *imported);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_read_player_name_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_config_read_player_name_pre[hIndex].func;
			retVal___ = preHookFunc(&filename, &config, &imported);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.config_read_player_name(filename, config, imported);
	}
	if (HPMHooks.count.HP_chr_config_read_player_name_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *filename, const struct config_t *config, bool imported);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_read_player_name_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_config_read_player_name_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, filename, config, imported);
		}
	}
	return retVal___;
}
void HP_chr_config_set_start_item(const struct config_setting_t *setting) {
	int hIndex = 0;
	if (HPMHooks.count.HP_chr_config_set_start_item_pre > 0) {
		void (*preHookFunc) (const struct config_setting_t **setting);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_set_start_item_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_config_set_start_item_pre[hIndex].func;
			preHookFunc(&setting);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.config_set_start_item(setting);
	}
	if (HPMHooks.count.HP_chr_config_set_start_item_post > 0) {
		void (*postHookFunc) (const struct config_setting_t *setting);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_set_start_item_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_config_set_start_item_post[hIndex].func;
			postHookFunc(setting);
		}
	}
	return;
}
bool HP_chr_config_read_player_new(const char *filename, const struct config_t *config, bool imported) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_chr_config_read_player_new_pre > 0) {
		bool (*preHookFunc) (const char **filename, const struct config_t **config, bool *imported);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_read_player_new_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_config_read_player_new_pre[hIndex].func;
			retVal___ = preHookFunc(&filename, &config, &imported);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.config_read_player_new(filename, config, imported);
	}
	if (HPMHooks.count.HP_chr_config_read_player_new_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *filename, const struct config_t *config, bool imported);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_read_player_new_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_config_read_player_new_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, filename, config, imported);
		}
	}
	return retVal___;
}
bool HP_chr_config_read_player(const char *filename, const struct config_t *config, bool imported) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_chr_config_read_player_pre > 0) {
		bool (*preHookFunc) (const char **filename, const struct config_t **config, bool *imported);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_read_player_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_config_read_player_pre[hIndex].func;
			retVal___ = preHookFunc(&filename, &config, &imported);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.config_read_player(filename, config, imported);
	}
	if (HPMHooks.count.HP_chr_config_read_player_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *filename, const struct config_t *config, bool imported);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_read_player_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_config_read_player_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, filename, config, imported);
		}
	}
	return retVal___;
}
bool HP_chr_config_read_permission(const char *filename, const struct config_t *config, bool imported) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_chr_config_read_permission_pre > 0) {
		bool (*preHookFunc) (const char **filename, const struct config_t **config, bool *imported);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_read_permission_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_config_read_permission_pre[hIndex].func;
			retVal___ = preHookFunc(&filename, &config, &imported);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.config_read_permission(filename, config, imported);
	}
	if (HPMHooks.count.HP_chr_config_read_permission_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *filename, const struct config_t *config, bool imported);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_read_permission_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_config_read_permission_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, filename, config, imported);
		}
	}
	return retVal___;
}
bool HP_chr_config_set_ip(const char *type, const char *value, uint32 *out_ip, char *out_ip_str) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_chr_config_set_ip_pre > 0) {
		bool (*preHookFunc) (const char **type, const char **value, uint32 **out_ip, char **out_ip_str);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_set_ip_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_config_set_ip_pre[hIndex].func;
			retVal___ = preHookFunc(&type, &value, &out_ip, &out_ip_str);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.config_set_ip(type, value, out_ip, out_ip_str);
	}
	if (HPMHooks.count.HP_chr_config_set_ip_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *type, const char *value, uint32 *out_ip, char *out_ip_str);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_set_ip_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_config_set_ip_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, type, value, out_ip, out_ip_str);
		}
	}
	return retVal___;
}
bool HP_chr_config_read_inter(const char *filename, const struct config_t *config, bool imported) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_chr_config_read_inter_pre > 0) {
		bool (*preHookFunc) (const char **filename, const struct config_t **config, bool *imported);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_read_inter_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_config_read_inter_pre[hIndex].func;
			retVal___ = preHookFunc(&filename, &config, &imported);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.config_read_inter(filename, config, imported);
	}
	if (HPMHooks.count.HP_chr_config_read_inter_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *filename, const struct config_t *config, bool imported);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_read_inter_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_config_read_inter_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, filename, config, imported);
		}
	}
	return retVal___;
}
bool HP_chr_config_read_top(const char *filename, const struct config_t *config, bool imported) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_chr_config_read_top_pre > 0) {
		bool (*preHookFunc) (const char **filename, const struct config_t **config, bool *imported);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_read_top_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_chr_config_read_top_pre[hIndex].func;
			retVal___ = preHookFunc(&filename, &config, &imported);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.config_read_top(filename, config, imported);
	}
	if (HPMHooks.count.HP_chr_config_read_top_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *filename, const struct config_t *config, bool imported);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_read_top_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_chr_config_read_top_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, filename, config, imported);
		}
	}
	return retVal___;
}
/* cmdline_interface */
void HP_cmdline_init(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_cmdline_init_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_cmdline_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_cmdline_init_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.cmdline.init();
	}
	if (HPMHooks.count.HP_cmdline_init_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_cmdline_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_cmdline_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_cmdline_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_cmdline_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_cmdline_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_cmdline_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.cmdline.final();
	}
	if (HPMHooks.count.HP_cmdline_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_cmdline_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_cmdline_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
bool HP_cmdline_arg_add(unsigned int pluginID, const char *name, char shortname, CmdlineExecFunc func, const char *help, unsigned int options) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_cmdline_arg_add_pre > 0) {
		bool (*preHookFunc) (unsigned int *pluginID, const char **name, char *shortname, CmdlineExecFunc *func, const char **help, unsigned int *options);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_cmdline_arg_add_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_cmdline_arg_add_pre[hIndex].func;
			retVal___ = preHookFunc(&pluginID, &name, &shortname, &func, &help, &options);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.cmdline.arg_add(pluginID, name, shortname, func, help, options);
	}
	if (HPMHooks.count.HP_cmdline_arg_add_post > 0) {
		bool (*postHookFunc) (bool retVal___, unsigned int pluginID, const char *name, char shortname, CmdlineExecFunc func, const char *help, unsigned int options);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_cmdline_arg_add_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_cmdline_arg_add_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, pluginID, name, shortname, func, help, options);
		}
	}
	return retVal___;
}
int HP_cmdline_exec(int argc, char **argv, unsigned int options) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_cmdline_exec_pre > 0) {
		int (*preHookFunc) (int *argc, char ***argv, unsigned int *options);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_cmdline_exec_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_cmdline_exec_pre[hIndex].func;
			retVal___ = preHookFunc(&argc, &argv, &options);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.cmdline.exec(argc, argv, options);
	}
	if (HPMHooks.count.HP_cmdline_exec_post > 0) {
		int (*postHookFunc) (int retVal___, int argc, char **argv, unsigned int options);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_cmdline_exec_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_cmdline_exec_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, argc, argv, options);
		}
	}
	return retVal___;
}
bool HP_cmdline_arg_next_value(const char *name, int current_arg, int argc) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_cmdline_arg_next_value_pre > 0) {
		bool (*preHookFunc) (const char **name, int *current_arg, int *argc);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_cmdline_arg_next_value_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_cmdline_arg_next_value_pre[hIndex].func;
			retVal___ = preHookFunc(&name, &current_arg, &argc);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.cmdline.arg_next_value(name, current_arg, argc);
	}
	if (HPMHooks.count.HP_cmdline_arg_next_value_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *name, int current_arg, int argc);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_cmdline_arg_next_value_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_cmdline_arg_next_value_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, name, current_arg, argc);
		}
	}
	return retVal___;
}
const char* HP_cmdline_arg_source(struct CmdlineArgData *arg) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if (HPMHooks.count.HP_cmdline_arg_source_pre > 0) {
		const char* (*preHookFunc) (struct CmdlineArgData **arg);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_cmdline_arg_source_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_cmdline_arg_source_pre[hIndex].func;
			retVal___ = preHookFunc(&arg);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.cmdline.arg_source(arg);
	}
	if (HPMHooks.count.HP_cmdline_arg_source_post > 0) {
		const char* (*postHookFunc) (const char* retVal___, struct CmdlineArgData *arg);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_cmdline_arg_source_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_cmdline_arg_source_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, arg);
		}
	}
	return retVal___;
}
/* console_interface */
void HP_console_init(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_console_init_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_console_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_console_init_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.console.init();
	}
	if (HPMHooks.count.HP_console_init_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_console_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_console_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_console_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_console_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_console_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_console_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.console.final();
	}
	if (HPMHooks.count.HP_console_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_console_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_console_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_console_display_title(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_console_display_title_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_console_display_title_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_console_display_title_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.console.display_title();
	}
	if (HPMHooks.count.HP_console_display_title_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_console_display_title_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_console_display_title_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_console_display_gplnotice(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_console_display_gplnotice_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_console_display_gplnotice_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_console_display_gplnotice_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.console.display_gplnotice();
	}
	if (HPMHooks.count.HP_console_display_gplnotice_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_console_display_gplnotice_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_console_display_gplnotice_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
/* core_interface */
void HP_core_shutdown_callback(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_core_shutdown_callback_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_core_shutdown_callback_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_core_shutdown_callback_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.core.shutdown_callback();
	}
	if (HPMHooks.count.HP_core_shutdown_callback_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_core_shutdown_callback_post; hIndex++) {
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
	if (HPMHooks.count.HP_DB_fix_options_pre > 0) {
		enum DBOptions (*preHookFunc) (enum DBType *type, enum DBOptions *options);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_fix_options_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_DB_fix_options_pre[hIndex].func;
			retVal___ = preHookFunc(&type, &options);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.fix_options(type, options);
	}
	if (HPMHooks.count.HP_DB_fix_options_post > 0) {
		enum DBOptions (*postHookFunc) (enum DBOptions retVal___, enum DBType type, enum DBOptions options);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_fix_options_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_DB_fix_options_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, type, options);
		}
	}
	return retVal___;
}
DBComparator HP_DB_default_cmp(enum DBType type) {
	int hIndex = 0;
	DBComparator retVal___ = NULL;
	if (HPMHooks.count.HP_DB_default_cmp_pre > 0) {
		DBComparator (*preHookFunc) (enum DBType *type);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_default_cmp_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_DB_default_cmp_pre[hIndex].func;
			retVal___ = preHookFunc(&type);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.default_cmp(type);
	}
	if (HPMHooks.count.HP_DB_default_cmp_post > 0) {
		DBComparator (*postHookFunc) (DBComparator retVal___, enum DBType type);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_default_cmp_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_DB_default_cmp_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, type);
		}
	}
	return retVal___;
}
DBHasher HP_DB_default_hash(enum DBType type) {
	int hIndex = 0;
	DBHasher retVal___ = NULL;
	if (HPMHooks.count.HP_DB_default_hash_pre > 0) {
		DBHasher (*preHookFunc) (enum DBType *type);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_default_hash_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_DB_default_hash_pre[hIndex].func;
			retVal___ = preHookFunc(&type);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.default_hash(type);
	}
	if (HPMHooks.count.HP_DB_default_hash_post > 0) {
		DBHasher (*postHookFunc) (DBHasher retVal___, enum DBType type);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_default_hash_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_DB_default_hash_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, type);
		}
	}
	return retVal___;
}
DBReleaser HP_DB_default_release(enum DBType type, enum DBOptions options) {
	int hIndex = 0;
	DBReleaser retVal___ = NULL;
	if (HPMHooks.count.HP_DB_default_release_pre > 0) {
		DBReleaser (*preHookFunc) (enum DBType *type, enum DBOptions *options);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_default_release_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_DB_default_release_pre[hIndex].func;
			retVal___ = preHookFunc(&type, &options);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.default_release(type, options);
	}
	if (HPMHooks.count.HP_DB_default_release_post > 0) {
		DBReleaser (*postHookFunc) (DBReleaser retVal___, enum DBType type, enum DBOptions options);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_default_release_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_DB_default_release_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, type, options);
		}
	}
	return retVal___;
}
DBReleaser HP_DB_custom_release(enum DBReleaseOption which) {
	int hIndex = 0;
	DBReleaser retVal___ = NULL;
	if (HPMHooks.count.HP_DB_custom_release_pre > 0) {
		DBReleaser (*preHookFunc) (enum DBReleaseOption *which);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_custom_release_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_DB_custom_release_pre[hIndex].func;
			retVal___ = preHookFunc(&which);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.custom_release(which);
	}
	if (HPMHooks.count.HP_DB_custom_release_post > 0) {
		DBReleaser (*postHookFunc) (DBReleaser retVal___, enum DBReleaseOption which);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_custom_release_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_DB_custom_release_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, which);
		}
	}
	return retVal___;
}
struct DBMap* HP_DB_alloc(const char *file, const char *func, int line, enum DBType type, enum DBOptions options, unsigned short maxlen) {
	int hIndex = 0;
	struct DBMap* retVal___ = NULL;
	if (HPMHooks.count.HP_DB_alloc_pre > 0) {
		struct DBMap* (*preHookFunc) (const char **file, const char **func, int *line, enum DBType *type, enum DBOptions *options, unsigned short *maxlen);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_alloc_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_DB_alloc_pre[hIndex].func;
			retVal___ = preHookFunc(&file, &func, &line, &type, &options, &maxlen);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.alloc(file, func, line, type, options, maxlen);
	}
	if (HPMHooks.count.HP_DB_alloc_post > 0) {
		struct DBMap* (*postHookFunc) (struct DBMap* retVal___, const char *file, const char *func, int line, enum DBType type, enum DBOptions options, unsigned short maxlen);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_alloc_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_DB_alloc_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, file, func, line, type, options, maxlen);
		}
	}
	return retVal___;
}
union DBKey HP_DB_i2key(int key) {
	int hIndex = 0;
	union DBKey retVal___ = { 0 };
	if (HPMHooks.count.HP_DB_i2key_pre > 0) {
		union DBKey (*preHookFunc) (int *key);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_i2key_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_DB_i2key_pre[hIndex].func;
			retVal___ = preHookFunc(&key);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.i2key(key);
	}
	if (HPMHooks.count.HP_DB_i2key_post > 0) {
		union DBKey (*postHookFunc) (union DBKey retVal___, int key);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_i2key_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_DB_i2key_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, key);
		}
	}
	return retVal___;
}
union DBKey HP_DB_ui2key(unsigned int key) {
	int hIndex = 0;
	union DBKey retVal___ = { 0 };
	if (HPMHooks.count.HP_DB_ui2key_pre > 0) {
		union DBKey (*preHookFunc) (unsigned int *key);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_ui2key_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_DB_ui2key_pre[hIndex].func;
			retVal___ = preHookFunc(&key);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.ui2key(key);
	}
	if (HPMHooks.count.HP_DB_ui2key_post > 0) {
		union DBKey (*postHookFunc) (union DBKey retVal___, unsigned int key);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_ui2key_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_DB_ui2key_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, key);
		}
	}
	return retVal___;
}
union DBKey HP_DB_str2key(const char *key) {
	int hIndex = 0;
	union DBKey retVal___ = { 0 };
	if (HPMHooks.count.HP_DB_str2key_pre > 0) {
		union DBKey (*preHookFunc) (const char **key);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_str2key_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_DB_str2key_pre[hIndex].func;
			retVal___ = preHookFunc(&key);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.str2key(key);
	}
	if (HPMHooks.count.HP_DB_str2key_post > 0) {
		union DBKey (*postHookFunc) (union DBKey retVal___, const char *key);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_str2key_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_DB_str2key_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, key);
		}
	}
	return retVal___;
}
union DBKey HP_DB_i642key(int64 key) {
	int hIndex = 0;
	union DBKey retVal___ = { 0 };
	if (HPMHooks.count.HP_DB_i642key_pre > 0) {
		union DBKey (*preHookFunc) (int64 *key);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_i642key_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_DB_i642key_pre[hIndex].func;
			retVal___ = preHookFunc(&key);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.i642key(key);
	}
	if (HPMHooks.count.HP_DB_i642key_post > 0) {
		union DBKey (*postHookFunc) (union DBKey retVal___, int64 key);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_i642key_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_DB_i642key_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, key);
		}
	}
	return retVal___;
}
union DBKey HP_DB_ui642key(uint64 key) {
	int hIndex = 0;
	union DBKey retVal___ = { 0 };
	if (HPMHooks.count.HP_DB_ui642key_pre > 0) {
		union DBKey (*preHookFunc) (uint64 *key);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_ui642key_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_DB_ui642key_pre[hIndex].func;
			retVal___ = preHookFunc(&key);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.ui642key(key);
	}
	if (HPMHooks.count.HP_DB_ui642key_post > 0) {
		union DBKey (*postHookFunc) (union DBKey retVal___, uint64 key);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_ui642key_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_DB_ui642key_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, key);
		}
	}
	return retVal___;
}
struct DBData HP_DB_i2data(int data) {
	int hIndex = 0;
	struct DBData retVal___ = { 0 };
	if (HPMHooks.count.HP_DB_i2data_pre > 0) {
		struct DBData (*preHookFunc) (int *data);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_i2data_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_DB_i2data_pre[hIndex].func;
			retVal___ = preHookFunc(&data);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.i2data(data);
	}
	if (HPMHooks.count.HP_DB_i2data_post > 0) {
		struct DBData (*postHookFunc) (struct DBData retVal___, int data);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_i2data_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_DB_i2data_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, data);
		}
	}
	return retVal___;
}
struct DBData HP_DB_ui2data(unsigned int data) {
	int hIndex = 0;
	struct DBData retVal___ = { 0 };
	if (HPMHooks.count.HP_DB_ui2data_pre > 0) {
		struct DBData (*preHookFunc) (unsigned int *data);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_ui2data_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_DB_ui2data_pre[hIndex].func;
			retVal___ = preHookFunc(&data);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.ui2data(data);
	}
	if (HPMHooks.count.HP_DB_ui2data_post > 0) {
		struct DBData (*postHookFunc) (struct DBData retVal___, unsigned int data);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_ui2data_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_DB_ui2data_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, data);
		}
	}
	return retVal___;
}
struct DBData HP_DB_ptr2data(void *data) {
	int hIndex = 0;
	struct DBData retVal___ = { 0 };
	if (HPMHooks.count.HP_DB_ptr2data_pre > 0) {
		struct DBData (*preHookFunc) (void **data);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_ptr2data_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_DB_ptr2data_pre[hIndex].func;
			retVal___ = preHookFunc(&data);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.ptr2data(data);
	}
	if (HPMHooks.count.HP_DB_ptr2data_post > 0) {
		struct DBData (*postHookFunc) (struct DBData retVal___, void *data);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_ptr2data_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_DB_ptr2data_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, data);
		}
	}
	return retVal___;
}
int HP_DB_data2i(struct DBData *data) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_DB_data2i_pre > 0) {
		int (*preHookFunc) (struct DBData **data);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_data2i_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_DB_data2i_pre[hIndex].func;
			retVal___ = preHookFunc(&data);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.data2i(data);
	}
	if (HPMHooks.count.HP_DB_data2i_post > 0) {
		int (*postHookFunc) (int retVal___, struct DBData *data);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_data2i_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_DB_data2i_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, data);
		}
	}
	return retVal___;
}
unsigned int HP_DB_data2ui(struct DBData *data) {
	int hIndex = 0;
	unsigned int retVal___ = 0;
	if (HPMHooks.count.HP_DB_data2ui_pre > 0) {
		unsigned int (*preHookFunc) (struct DBData **data);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_data2ui_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_DB_data2ui_pre[hIndex].func;
			retVal___ = preHookFunc(&data);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.data2ui(data);
	}
	if (HPMHooks.count.HP_DB_data2ui_post > 0) {
		unsigned int (*postHookFunc) (unsigned int retVal___, struct DBData *data);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_data2ui_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_DB_data2ui_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, data);
		}
	}
	return retVal___;
}
void* HP_DB_data2ptr(struct DBData *data) {
	int hIndex = 0;
	void* retVal___ = NULL;
	if (HPMHooks.count.HP_DB_data2ptr_pre > 0) {
		void* (*preHookFunc) (struct DBData **data);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_data2ptr_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_DB_data2ptr_pre[hIndex].func;
			retVal___ = preHookFunc(&data);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.DB.data2ptr(data);
	}
	if (HPMHooks.count.HP_DB_data2ptr_post > 0) {
		void* (*postHookFunc) (void* retVal___, struct DBData *data);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_data2ptr_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_DB_data2ptr_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, data);
		}
	}
	return retVal___;
}
void HP_DB_init(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_DB_init_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_DB_init_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.DB.init();
	}
	if (HPMHooks.count.HP_DB_init_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_DB_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_DB_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_DB_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_DB_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.DB.final();
	}
	if (HPMHooks.count.HP_DB_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_DB_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_DB_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
/* des_interface */
void HP_des_decrypt_block(struct des_bit64 *block) {
	int hIndex = 0;
	if (HPMHooks.count.HP_des_decrypt_block_pre > 0) {
		void (*preHookFunc) (struct des_bit64 **block);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_des_decrypt_block_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_des_decrypt_block_pre[hIndex].func;
			preHookFunc(&block);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.des.decrypt_block(block);
	}
	if (HPMHooks.count.HP_des_decrypt_block_post > 0) {
		void (*postHookFunc) (struct des_bit64 *block);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_des_decrypt_block_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_des_decrypt_block_post[hIndex].func;
			postHookFunc(block);
		}
	}
	return;
}
void HP_des_decrypt(unsigned char *data, size_t size) {
	int hIndex = 0;
	if (HPMHooks.count.HP_des_decrypt_pre > 0) {
		void (*preHookFunc) (unsigned char **data, size_t *size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_des_decrypt_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_des_decrypt_pre[hIndex].func;
			preHookFunc(&data, &size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.des.decrypt(data, size);
	}
	if (HPMHooks.count.HP_des_decrypt_post > 0) {
		void (*postHookFunc) (unsigned char *data, size_t size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_des_decrypt_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_des_decrypt_post[hIndex].func;
			postHookFunc(data, size);
		}
	}
	return;
}
/* geoip_interface */
const char* HP_geoip_getcountry(uint32 ipnum) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if (HPMHooks.count.HP_geoip_getcountry_pre > 0) {
		const char* (*preHookFunc) (uint32 *ipnum);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_geoip_getcountry_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_geoip_getcountry_pre[hIndex].func;
			retVal___ = preHookFunc(&ipnum);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.geoip.getcountry(ipnum);
	}
	if (HPMHooks.count.HP_geoip_getcountry_post > 0) {
		const char* (*postHookFunc) (const char* retVal___, uint32 ipnum);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_geoip_getcountry_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_geoip_getcountry_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, ipnum);
		}
	}
	return retVal___;
}
void HP_geoip_final(bool shutdown) {
	int hIndex = 0;
	if (HPMHooks.count.HP_geoip_final_pre > 0) {
		void (*preHookFunc) (bool *shutdown);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_geoip_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_geoip_final_pre[hIndex].func;
			preHookFunc(&shutdown);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.geoip.final(shutdown);
	}
	if (HPMHooks.count.HP_geoip_final_post > 0) {
		void (*postHookFunc) (bool shutdown);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_geoip_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_geoip_final_post[hIndex].func;
			postHookFunc(shutdown);
		}
	}
	return;
}
void HP_geoip_init(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_geoip_init_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_geoip_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_geoip_init_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.geoip.init();
	}
	if (HPMHooks.count.HP_geoip_init_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_geoip_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_geoip_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
/* inter_auction_interface */
int HP_inter_auction_count(int char_id, bool buy) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_auction_count_pre > 0) {
		int (*preHookFunc) (int *char_id, bool *buy);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_count_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_auction_count_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &buy);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_auction.count(char_id, buy);
	}
	if (HPMHooks.count.HP_inter_auction_count_post > 0) {
		int (*postHookFunc) (int retVal___, int char_id, bool buy);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_count_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_auction_count_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, char_id, buy);
		}
	}
	return retVal___;
}
void HP_inter_auction_save(struct auction_data *auction) {
	int hIndex = 0;
	if (HPMHooks.count.HP_inter_auction_save_pre > 0) {
		void (*preHookFunc) (struct auction_data **auction);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_save_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_auction_save_pre[hIndex].func;
			preHookFunc(&auction);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_auction.save(auction);
	}
	if (HPMHooks.count.HP_inter_auction_save_post > 0) {
		void (*postHookFunc) (struct auction_data *auction);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_save_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_auction_save_post[hIndex].func;
			postHookFunc(auction);
		}
	}
	return;
}
unsigned int HP_inter_auction_create(struct auction_data *auction) {
	int hIndex = 0;
	unsigned int retVal___ = 0;
	if (HPMHooks.count.HP_inter_auction_create_pre > 0) {
		unsigned int (*preHookFunc) (struct auction_data **auction);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_create_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_auction_create_pre[hIndex].func;
			retVal___ = preHookFunc(&auction);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_auction.create(auction);
	}
	if (HPMHooks.count.HP_inter_auction_create_post > 0) {
		unsigned int (*postHookFunc) (unsigned int retVal___, struct auction_data *auction);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_create_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_auction_create_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, auction);
		}
	}
	return retVal___;
}
int HP_inter_auction_end_timer(int tid, int64 tick, int id, intptr_t data) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_auction_end_timer_pre > 0) {
		int (*preHookFunc) (int *tid, int64 *tick, int *id, intptr_t *data);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_end_timer_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_auction_end_timer_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &tick, &id, &data);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_auction.end_timer(tid, tick, id, data);
	}
	if (HPMHooks.count.HP_inter_auction_end_timer_post > 0) {
		int (*postHookFunc) (int retVal___, int tid, int64 tick, int id, intptr_t data);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_end_timer_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_auction_end_timer_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, tid, tick, id, data);
		}
	}
	return retVal___;
}
void HP_inter_auction_delete_(struct auction_data *auction) {
	int hIndex = 0;
	if (HPMHooks.count.HP_inter_auction_delete__pre > 0) {
		void (*preHookFunc) (struct auction_data **auction);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_delete__pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_auction_delete__pre[hIndex].func;
			preHookFunc(&auction);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_auction.delete_(auction);
	}
	if (HPMHooks.count.HP_inter_auction_delete__post > 0) {
		void (*postHookFunc) (struct auction_data *auction);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_delete__post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_auction_delete__post[hIndex].func;
			postHookFunc(auction);
		}
	}
	return;
}
void HP_inter_auction_fromsql(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_inter_auction_fromsql_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_fromsql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_auction_fromsql_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_auction.fromsql();
	}
	if (HPMHooks.count.HP_inter_auction_fromsql_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_fromsql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_auction_fromsql_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_inter_auction_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_auction_parse_frommap_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_parse_frommap_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_auction_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_auction.parse_frommap(fd);
	}
	if (HPMHooks.count.HP_inter_auction_parse_frommap_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_parse_frommap_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_auction_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_inter_auction_sql_init(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_auction_sql_init_pre > 0) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_sql_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_auction_sql_init_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_auction.sql_init();
	}
	if (HPMHooks.count.HP_inter_auction_sql_init_post > 0) {
		int (*postHookFunc) (int retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_sql_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_auction_sql_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_inter_auction_sql_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_inter_auction_sql_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_sql_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_auction_sql_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_auction.sql_final();
	}
	if (HPMHooks.count.HP_inter_auction_sql_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_sql_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_auction_sql_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
/* inter_elemental_interface */
void HP_inter_elemental_sql_init(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_inter_elemental_sql_init_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_elemental_sql_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_elemental_sql_init_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_elemental.sql_init();
	}
	if (HPMHooks.count.HP_inter_elemental_sql_init_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_elemental_sql_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_elemental_sql_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_inter_elemental_sql_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_inter_elemental_sql_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_elemental_sql_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_elemental_sql_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_elemental.sql_final();
	}
	if (HPMHooks.count.HP_inter_elemental_sql_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_elemental_sql_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_elemental_sql_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_inter_elemental_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_elemental_parse_frommap_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_elemental_parse_frommap_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_elemental_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_elemental.parse_frommap(fd);
	}
	if (HPMHooks.count.HP_inter_elemental_parse_frommap_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_elemental_parse_frommap_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_elemental_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
/* inter_guild_interface */
int HP_inter_guild_save_timer(int tid, int64 tick, int id, intptr_t data) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_guild_save_timer_pre > 0) {
		int (*preHookFunc) (int *tid, int64 *tick, int *id, intptr_t *data);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_save_timer_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_guild_save_timer_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &tick, &id, &data);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.save_timer(tid, tick, id, data);
	}
	if (HPMHooks.count.HP_inter_guild_save_timer_post > 0) {
		int (*postHookFunc) (int retVal___, int tid, int64 tick, int id, intptr_t data);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_save_timer_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_guild_save_timer_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, tid, tick, id, data);
		}
	}
	return retVal___;
}
int HP_inter_guild_removemember_tosql(int account_id, int char_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_guild_removemember_tosql_pre > 0) {
		int (*preHookFunc) (int *account_id, int *char_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_removemember_tosql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_guild_removemember_tosql_pre[hIndex].func;
			retVal___ = preHookFunc(&account_id, &char_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.removemember_tosql(account_id, char_id);
	}
	if (HPMHooks.count.HP_inter_guild_removemember_tosql_post > 0) {
		int (*postHookFunc) (int retVal___, int account_id, int char_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_removemember_tosql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_guild_removemember_tosql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, account_id, char_id);
		}
	}
	return retVal___;
}
int HP_inter_guild_tosql(struct guild *g, int flag) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_guild_tosql_pre > 0) {
		int (*preHookFunc) (struct guild **g, int *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_tosql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_guild_tosql_pre[hIndex].func;
			retVal___ = preHookFunc(&g, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.tosql(g, flag);
	}
	if (HPMHooks.count.HP_inter_guild_tosql_post > 0) {
		int (*postHookFunc) (int retVal___, struct guild *g, int flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_tosql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_guild_tosql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, g, flag);
		}
	}
	return retVal___;
}
struct guild* HP_inter_guild_fromsql(int guild_id) {
	int hIndex = 0;
	struct guild* retVal___ = NULL;
	if (HPMHooks.count.HP_inter_guild_fromsql_pre > 0) {
		struct guild* (*preHookFunc) (int *guild_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_fromsql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_guild_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.fromsql(guild_id);
	}
	if (HPMHooks.count.HP_inter_guild_fromsql_post > 0) {
		struct guild* (*postHookFunc) (struct guild* retVal___, int guild_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_fromsql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_guild_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, guild_id);
		}
	}
	return retVal___;
}
int HP_inter_guild_castle_tosql(struct guild_castle *gc) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_guild_castle_tosql_pre > 0) {
		int (*preHookFunc) (struct guild_castle **gc);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_castle_tosql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_guild_castle_tosql_pre[hIndex].func;
			retVal___ = preHookFunc(&gc);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.castle_tosql(gc);
	}
	if (HPMHooks.count.HP_inter_guild_castle_tosql_post > 0) {
		int (*postHookFunc) (int retVal___, struct guild_castle *gc);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_castle_tosql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_guild_castle_tosql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, gc);
		}
	}
	return retVal___;
}
struct guild_castle* HP_inter_guild_castle_fromsql(int castle_id) {
	int hIndex = 0;
	struct guild_castle* retVal___ = NULL;
	if (HPMHooks.count.HP_inter_guild_castle_fromsql_pre > 0) {
		struct guild_castle* (*preHookFunc) (int *castle_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_castle_fromsql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_guild_castle_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(&castle_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.castle_fromsql(castle_id);
	}
	if (HPMHooks.count.HP_inter_guild_castle_fromsql_post > 0) {
		struct guild_castle* (*postHookFunc) (struct guild_castle* retVal___, int castle_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_castle_fromsql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_guild_castle_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, castle_id);
		}
	}
	return retVal___;
}
bool HP_inter_guild_exp_parse_row(char *split[], int column, int current) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_inter_guild_exp_parse_row_pre > 0) {
		bool (*preHookFunc) (char **split[], int *column, int *current);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_exp_parse_row_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_guild_exp_parse_row_pre[hIndex].func;
			retVal___ = preHookFunc(&split, &column, &current);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.exp_parse_row(split, column, current);
	}
	if (HPMHooks.count.HP_inter_guild_exp_parse_row_post > 0) {
		bool (*postHookFunc) (bool retVal___, char *split[], int column, int current);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_exp_parse_row_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_guild_exp_parse_row_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, split, column, current);
		}
	}
	return retVal___;
}
int HP_inter_guild_CharOnline(int char_id, int guild_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_guild_CharOnline_pre > 0) {
		int (*preHookFunc) (int *char_id, int *guild_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_CharOnline_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_guild_CharOnline_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &guild_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.CharOnline(char_id, guild_id);
	}
	if (HPMHooks.count.HP_inter_guild_CharOnline_post > 0) {
		int (*postHookFunc) (int retVal___, int char_id, int guild_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_CharOnline_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_guild_CharOnline_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, char_id, guild_id);
		}
	}
	return retVal___;
}
int HP_inter_guild_CharOffline(int char_id, int guild_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_guild_CharOffline_pre > 0) {
		int (*preHookFunc) (int *char_id, int *guild_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_CharOffline_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_guild_CharOffline_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &guild_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.CharOffline(char_id, guild_id);
	}
	if (HPMHooks.count.HP_inter_guild_CharOffline_post > 0) {
		int (*postHookFunc) (int retVal___, int char_id, int guild_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_CharOffline_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_guild_CharOffline_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, char_id, guild_id);
		}
	}
	return retVal___;
}
int HP_inter_guild_sql_init(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_guild_sql_init_pre > 0) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_sql_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_guild_sql_init_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.sql_init();
	}
	if (HPMHooks.count.HP_inter_guild_sql_init_post > 0) {
		int (*postHookFunc) (int retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_sql_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_guild_sql_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
int HP_inter_guild_db_final(union DBKey key, struct DBData *data, va_list ap) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_guild_db_final_pre > 0) {
		int (*preHookFunc) (union DBKey *key, struct DBData **data, va_list ap);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_db_final_pre; hIndex++) {
			va_list ap___copy; va_copy(ap___copy, ap);
			preHookFunc = HPMHooks.list.HP_inter_guild_db_final_pre[hIndex].func;
			retVal___ = preHookFunc(&key, &data, ap___copy);
			va_end(ap___copy);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		va_list ap___copy; va_copy(ap___copy, ap);
		retVal___ = HPMHooks.source.inter_guild.db_final(key, data, ap___copy);
		va_end(ap___copy);
	}
	if (HPMHooks.count.HP_inter_guild_db_final_post > 0) {
		int (*postHookFunc) (int retVal___, union DBKey key, struct DBData *data, va_list ap);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_db_final_post; hIndex++) {
			va_list ap___copy; va_copy(ap___copy, ap);
			postHookFunc = HPMHooks.list.HP_inter_guild_db_final_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, key, data, ap___copy);
			va_end(ap___copy);
		}
	}
	return retVal___;
}
void HP_inter_guild_sql_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_inter_guild_sql_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_sql_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_guild_sql_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_guild.sql_final();
	}
	if (HPMHooks.count.HP_inter_guild_sql_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_sql_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_guild_sql_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_inter_guild_search_guildname(const char *str) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_guild_search_guildname_pre > 0) {
		int (*preHookFunc) (const char **str);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_search_guildname_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_guild_search_guildname_pre[hIndex].func;
			retVal___ = preHookFunc(&str);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.search_guildname(str);
	}
	if (HPMHooks.count.HP_inter_guild_search_guildname_post > 0) {
		int (*postHookFunc) (int retVal___, const char *str);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_search_guildname_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_guild_search_guildname_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, str);
		}
	}
	return retVal___;
}
bool HP_inter_guild_check_empty(struct guild *g) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_inter_guild_check_empty_pre > 0) {
		bool (*preHookFunc) (struct guild **g);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_check_empty_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_guild_check_empty_pre[hIndex].func;
			retVal___ = preHookFunc(&g);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.check_empty(g);
	}
	if (HPMHooks.count.HP_inter_guild_check_empty_post > 0) {
		bool (*postHookFunc) (bool retVal___, struct guild *g);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_check_empty_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_guild_check_empty_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, g);
		}
	}
	return retVal___;
}
unsigned int HP_inter_guild_nextexp(int level) {
	int hIndex = 0;
	unsigned int retVal___ = 0;
	if (HPMHooks.count.HP_inter_guild_nextexp_pre > 0) {
		unsigned int (*preHookFunc) (int *level);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_nextexp_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_guild_nextexp_pre[hIndex].func;
			retVal___ = preHookFunc(&level);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.nextexp(level);
	}
	if (HPMHooks.count.HP_inter_guild_nextexp_post > 0) {
		unsigned int (*postHookFunc) (unsigned int retVal___, int level);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_nextexp_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_guild_nextexp_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, level);
		}
	}
	return retVal___;
}
int HP_inter_guild_checkskill(struct guild *g, int id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_guild_checkskill_pre > 0) {
		int (*preHookFunc) (struct guild **g, int *id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_checkskill_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_guild_checkskill_pre[hIndex].func;
			retVal___ = preHookFunc(&g, &id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.checkskill(g, id);
	}
	if (HPMHooks.count.HP_inter_guild_checkskill_post > 0) {
		int (*postHookFunc) (int retVal___, struct guild *g, int id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_checkskill_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_guild_checkskill_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, g, id);
		}
	}
	return retVal___;
}
int HP_inter_guild_calcinfo(struct guild *g) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_guild_calcinfo_pre > 0) {
		int (*preHookFunc) (struct guild **g);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_calcinfo_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_guild_calcinfo_pre[hIndex].func;
			retVal___ = preHookFunc(&g);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.calcinfo(g);
	}
	if (HPMHooks.count.HP_inter_guild_calcinfo_post > 0) {
		int (*postHookFunc) (int retVal___, struct guild *g);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_calcinfo_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_guild_calcinfo_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, g);
		}
	}
	return retVal___;
}
int HP_inter_guild_sex_changed(int guild_id, int account_id, int char_id, short gender) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_guild_sex_changed_pre > 0) {
		int (*preHookFunc) (int *guild_id, int *account_id, int *char_id, short *gender);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_sex_changed_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_guild_sex_changed_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id, &account_id, &char_id, &gender);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.sex_changed(guild_id, account_id, char_id, gender);
	}
	if (HPMHooks.count.HP_inter_guild_sex_changed_post > 0) {
		int (*postHookFunc) (int retVal___, int guild_id, int account_id, int char_id, short gender);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_sex_changed_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_guild_sex_changed_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, guild_id, account_id, char_id, gender);
		}
	}
	return retVal___;
}
int HP_inter_guild_charname_changed(int guild_id, int account_id, int char_id, char *name) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_guild_charname_changed_pre > 0) {
		int (*preHookFunc) (int *guild_id, int *account_id, int *char_id, char **name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_charname_changed_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_guild_charname_changed_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id, &account_id, &char_id, &name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.charname_changed(guild_id, account_id, char_id, name);
	}
	if (HPMHooks.count.HP_inter_guild_charname_changed_post > 0) {
		int (*postHookFunc) (int retVal___, int guild_id, int account_id, int char_id, char *name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_charname_changed_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_guild_charname_changed_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, guild_id, account_id, char_id, name);
		}
	}
	return retVal___;
}
int HP_inter_guild_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_guild_parse_frommap_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_parse_frommap_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_guild_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.parse_frommap(fd);
	}
	if (HPMHooks.count.HP_inter_guild_parse_frommap_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_parse_frommap_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_guild_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_inter_guild_leave(int guild_id, int account_id, int char_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_guild_leave_pre > 0) {
		int (*preHookFunc) (int *guild_id, int *account_id, int *char_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_leave_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_guild_leave_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id, &account_id, &char_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.leave(guild_id, account_id, char_id);
	}
	if (HPMHooks.count.HP_inter_guild_leave_post > 0) {
		int (*postHookFunc) (int retVal___, int guild_id, int account_id, int char_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_leave_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_guild_leave_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, guild_id, account_id, char_id);
		}
	}
	return retVal___;
}
int HP_inter_guild_broken(int guild_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_guild_broken_pre > 0) {
		int (*preHookFunc) (int *guild_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_broken_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_guild_broken_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.broken(guild_id);
	}
	if (HPMHooks.count.HP_inter_guild_broken_post > 0) {
		int (*postHookFunc) (int retVal___, int guild_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_broken_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_guild_broken_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, guild_id);
		}
	}
	return retVal___;
}
/* inter_homunculus_interface */
int HP_inter_homunculus_sql_init(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_homunculus_sql_init_pre > 0) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_homunculus_sql_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_homunculus_sql_init_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_homunculus.sql_init();
	}
	if (HPMHooks.count.HP_inter_homunculus_sql_init_post > 0) {
		int (*postHookFunc) (int retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_homunculus_sql_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_homunculus_sql_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_inter_homunculus_sql_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_inter_homunculus_sql_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_homunculus_sql_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_homunculus_sql_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_homunculus.sql_final();
	}
	if (HPMHooks.count.HP_inter_homunculus_sql_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_homunculus_sql_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_homunculus_sql_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_inter_homunculus_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_homunculus_parse_frommap_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_homunculus_parse_frommap_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_homunculus_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_homunculus.parse_frommap(fd);
	}
	if (HPMHooks.count.HP_inter_homunculus_parse_frommap_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_homunculus_parse_frommap_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_homunculus_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
/* inter_interface */
const char* HP_inter_msg_txt(int msg_number) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if (HPMHooks.count.HP_inter_msg_txt_pre > 0) {
		const char* (*preHookFunc) (int *msg_number);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_msg_txt_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_msg_txt_pre[hIndex].func;
			retVal___ = preHookFunc(&msg_number);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter.msg_txt(msg_number);
	}
	if (HPMHooks.count.HP_inter_msg_txt_post > 0) {
		const char* (*postHookFunc) (const char* retVal___, int msg_number);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_msg_txt_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_msg_txt_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, msg_number);
		}
	}
	return retVal___;
}
bool HP_inter_msg_config_read(const char *cfg_name, bool allow_override) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_inter_msg_config_read_pre > 0) {
		bool (*preHookFunc) (const char **cfg_name, bool *allow_override);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_msg_config_read_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_msg_config_read_pre[hIndex].func;
			retVal___ = preHookFunc(&cfg_name, &allow_override);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter.msg_config_read(cfg_name, allow_override);
	}
	if (HPMHooks.count.HP_inter_msg_config_read_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *cfg_name, bool allow_override);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_msg_config_read_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_msg_config_read_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, cfg_name, allow_override);
		}
	}
	return retVal___;
}
void HP_inter_do_final_msg(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_inter_do_final_msg_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_do_final_msg_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_do_final_msg_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter.do_final_msg();
	}
	if (HPMHooks.count.HP_inter_do_final_msg_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_do_final_msg_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_do_final_msg_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
const char* HP_inter_job_name(int class) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if (HPMHooks.count.HP_inter_job_name_pre > 0) {
		const char* (*preHookFunc) (int *class);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_job_name_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_job_name_pre[hIndex].func;
			retVal___ = preHookFunc(&class);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter.job_name(class);
	}
	if (HPMHooks.count.HP_inter_job_name_post > 0) {
		const char* (*postHookFunc) (const char* retVal___, int class);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_job_name_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_job_name_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, class);
		}
	}
	return retVal___;
}
void HP_inter_vmsg_to_fd(int fd, int u_fd, int aid, char *msg, va_list ap) {
	int hIndex = 0;
	if (HPMHooks.count.HP_inter_vmsg_to_fd_pre > 0) {
		void (*preHookFunc) (int *fd, int *u_fd, int *aid, char **msg, va_list ap);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_vmsg_to_fd_pre; hIndex++) {
			va_list ap___copy; va_copy(ap___copy, ap);
			preHookFunc = HPMHooks.list.HP_inter_vmsg_to_fd_pre[hIndex].func;
			preHookFunc(&fd, &u_fd, &aid, &msg, ap___copy);
			va_end(ap___copy);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		va_list ap___copy; va_copy(ap___copy, ap);
		HPMHooks.source.inter.vmsg_to_fd(fd, u_fd, aid, msg, ap___copy);
		va_end(ap___copy);
	}
	if (HPMHooks.count.HP_inter_vmsg_to_fd_post > 0) {
		void (*postHookFunc) (int fd, int u_fd, int aid, char *msg, va_list ap);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_vmsg_to_fd_post; hIndex++) {
			va_list ap___copy; va_copy(ap___copy, ap);
			postHookFunc = HPMHooks.list.HP_inter_vmsg_to_fd_post[hIndex].func;
			postHookFunc(fd, u_fd, aid, msg, ap___copy);
			va_end(ap___copy);
		}
	}
	return;
}
void HP_inter_savereg(int account_id, int char_id, const char *key, unsigned int index, intptr_t val, bool is_string) {
	int hIndex = 0;
	if (HPMHooks.count.HP_inter_savereg_pre > 0) {
		void (*preHookFunc) (int *account_id, int *char_id, const char **key, unsigned int *index, intptr_t *val, bool *is_string);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_savereg_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_savereg_pre[hIndex].func;
			preHookFunc(&account_id, &char_id, &key, &index, &val, &is_string);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter.savereg(account_id, char_id, key, index, val, is_string);
	}
	if (HPMHooks.count.HP_inter_savereg_post > 0) {
		void (*postHookFunc) (int account_id, int char_id, const char *key, unsigned int index, intptr_t val, bool is_string);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_savereg_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_savereg_post[hIndex].func;
			postHookFunc(account_id, char_id, key, index, val, is_string);
		}
	}
	return;
}
int HP_inter_accreg_fromsql(int account_id, int char_id, int fd, int type) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_accreg_fromsql_pre > 0) {
		int (*preHookFunc) (int *account_id, int *char_id, int *fd, int *type);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_accreg_fromsql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_accreg_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(&account_id, &char_id, &fd, &type);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter.accreg_fromsql(account_id, char_id, fd, type);
	}
	if (HPMHooks.count.HP_inter_accreg_fromsql_post > 0) {
		int (*postHookFunc) (int retVal___, int account_id, int char_id, int fd, int type);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_accreg_fromsql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_accreg_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, account_id, char_id, fd, type);
		}
	}
	return retVal___;
}
int HP_inter_vlog(char *fmt, va_list ap) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_vlog_pre > 0) {
		int (*preHookFunc) (char **fmt, va_list ap);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_vlog_pre; hIndex++) {
			va_list ap___copy; va_copy(ap___copy, ap);
			preHookFunc = HPMHooks.list.HP_inter_vlog_pre[hIndex].func;
			retVal___ = preHookFunc(&fmt, ap___copy);
			va_end(ap___copy);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		va_list ap___copy; va_copy(ap___copy, ap);
		retVal___ = HPMHooks.source.inter.vlog(fmt, ap___copy);
		va_end(ap___copy);
	}
	if (HPMHooks.count.HP_inter_vlog_post > 0) {
		int (*postHookFunc) (int retVal___, char *fmt, va_list ap);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_vlog_post; hIndex++) {
			va_list ap___copy; va_copy(ap___copy, ap);
			postHookFunc = HPMHooks.list.HP_inter_vlog_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fmt, ap___copy);
			va_end(ap___copy);
		}
	}
	return retVal___;
}
int HP_inter_init_sql(const char *file) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_init_sql_pre > 0) {
		int (*preHookFunc) (const char **file);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_init_sql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_init_sql_pre[hIndex].func;
			retVal___ = preHookFunc(&file);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter.init_sql(file);
	}
	if (HPMHooks.count.HP_inter_init_sql_post > 0) {
		int (*postHookFunc) (int retVal___, const char *file);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_init_sql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_init_sql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, file);
		}
	}
	return retVal___;
}
int HP_inter_mapif_init(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_mapif_init_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mapif_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_mapif_init_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter.mapif_init(fd);
	}
	if (HPMHooks.count.HP_inter_mapif_init_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mapif_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_mapif_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_inter_check_ttl_wisdata_sub(union DBKey key, struct DBData *data, va_list ap) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_check_ttl_wisdata_sub_pre > 0) {
		int (*preHookFunc) (union DBKey *key, struct DBData **data, va_list ap);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_check_ttl_wisdata_sub_pre; hIndex++) {
			va_list ap___copy; va_copy(ap___copy, ap);
			preHookFunc = HPMHooks.list.HP_inter_check_ttl_wisdata_sub_pre[hIndex].func;
			retVal___ = preHookFunc(&key, &data, ap___copy);
			va_end(ap___copy);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		va_list ap___copy; va_copy(ap___copy, ap);
		retVal___ = HPMHooks.source.inter.check_ttl_wisdata_sub(key, data, ap___copy);
		va_end(ap___copy);
	}
	if (HPMHooks.count.HP_inter_check_ttl_wisdata_sub_post > 0) {
		int (*postHookFunc) (int retVal___, union DBKey key, struct DBData *data, va_list ap);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_check_ttl_wisdata_sub_post; hIndex++) {
			va_list ap___copy; va_copy(ap___copy, ap);
			postHookFunc = HPMHooks.list.HP_inter_check_ttl_wisdata_sub_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, key, data, ap___copy);
			va_end(ap___copy);
		}
	}
	return retVal___;
}
int HP_inter_check_ttl_wisdata(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_check_ttl_wisdata_pre > 0) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_check_ttl_wisdata_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_check_ttl_wisdata_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter.check_ttl_wisdata();
	}
	if (HPMHooks.count.HP_inter_check_ttl_wisdata_post > 0) {
		int (*postHookFunc) (int retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_check_ttl_wisdata_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_check_ttl_wisdata_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
int HP_inter_check_length(int fd, int length) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_check_length_pre > 0) {
		int (*preHookFunc) (int *fd, int *length);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_check_length_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_check_length_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &length);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter.check_length(fd, length);
	}
	if (HPMHooks.count.HP_inter_check_length_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int length);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_check_length_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_check_length_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, length);
		}
	}
	return retVal___;
}
int HP_inter_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_parse_frommap_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_parse_frommap_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter.parse_frommap(fd);
	}
	if (HPMHooks.count.HP_inter_parse_frommap_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_parse_frommap_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
void HP_inter_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_inter_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter.final();
	}
	if (HPMHooks.count.HP_inter_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
bool HP_inter_config_read(const char *filename, bool imported) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_inter_config_read_pre > 0) {
		bool (*preHookFunc) (const char **filename, bool *imported);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_config_read_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_config_read_pre[hIndex].func;
			retVal___ = preHookFunc(&filename, &imported);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter.config_read(filename, imported);
	}
	if (HPMHooks.count.HP_inter_config_read_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *filename, bool imported);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_config_read_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_config_read_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, filename, imported);
		}
	}
	return retVal___;
}
bool HP_inter_config_read_log(const char *filename, const struct config_t *config, bool imported) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_inter_config_read_log_pre > 0) {
		bool (*preHookFunc) (const char **filename, const struct config_t **config, bool *imported);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_config_read_log_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_config_read_log_pre[hIndex].func;
			retVal___ = preHookFunc(&filename, &config, &imported);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter.config_read_log(filename, config, imported);
	}
	if (HPMHooks.count.HP_inter_config_read_log_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *filename, const struct config_t *config, bool imported);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_config_read_log_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_config_read_log_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, filename, config, imported);
		}
	}
	return retVal___;
}
bool HP_inter_config_read_connection(const char *filename, const struct config_t *config, bool imported) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_inter_config_read_connection_pre > 0) {
		bool (*preHookFunc) (const char **filename, const struct config_t **config, bool *imported);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_config_read_connection_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_config_read_connection_pre[hIndex].func;
			retVal___ = preHookFunc(&filename, &config, &imported);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter.config_read_connection(filename, config, imported);
	}
	if (HPMHooks.count.HP_inter_config_read_connection_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *filename, const struct config_t *config, bool imported);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_config_read_connection_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_config_read_connection_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, filename, config, imported);
		}
	}
	return retVal___;
}
/* inter_mail_interface */
int HP_inter_mail_sql_init(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_mail_sql_init_pre > 0) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_sql_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_mail_sql_init_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_mail.sql_init();
	}
	if (HPMHooks.count.HP_inter_mail_sql_init_post > 0) {
		int (*postHookFunc) (int retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_sql_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_mail_sql_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_inter_mail_sql_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_inter_mail_sql_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_sql_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_mail_sql_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_mail.sql_final();
	}
	if (HPMHooks.count.HP_inter_mail_sql_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_sql_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_mail_sql_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_inter_mail_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_mail_parse_frommap_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_parse_frommap_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_mail_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_mail.parse_frommap(fd);
	}
	if (HPMHooks.count.HP_inter_mail_parse_frommap_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_parse_frommap_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_mail_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_inter_mail_fromsql(int char_id, struct mail_data *md) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_mail_fromsql_pre > 0) {
		int (*preHookFunc) (int *char_id, struct mail_data **md);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_fromsql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_mail_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &md);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_mail.fromsql(char_id, md);
	}
	if (HPMHooks.count.HP_inter_mail_fromsql_post > 0) {
		int (*postHookFunc) (int retVal___, int char_id, struct mail_data *md);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_fromsql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_mail_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, char_id, md);
		}
	}
	return retVal___;
}
int HP_inter_mail_savemessage(struct mail_message *msg) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_mail_savemessage_pre > 0) {
		int (*preHookFunc) (struct mail_message **msg);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_savemessage_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_mail_savemessage_pre[hIndex].func;
			retVal___ = preHookFunc(&msg);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_mail.savemessage(msg);
	}
	if (HPMHooks.count.HP_inter_mail_savemessage_post > 0) {
		int (*postHookFunc) (int retVal___, struct mail_message *msg);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_savemessage_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_mail_savemessage_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, msg);
		}
	}
	return retVal___;
}
bool HP_inter_mail_loadmessage(int mail_id, struct mail_message *msg) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_inter_mail_loadmessage_pre > 0) {
		bool (*preHookFunc) (int *mail_id, struct mail_message **msg);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_loadmessage_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_mail_loadmessage_pre[hIndex].func;
			retVal___ = preHookFunc(&mail_id, &msg);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_mail.loadmessage(mail_id, msg);
	}
	if (HPMHooks.count.HP_inter_mail_loadmessage_post > 0) {
		bool (*postHookFunc) (bool retVal___, int mail_id, struct mail_message *msg);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_loadmessage_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_mail_loadmessage_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, mail_id, msg);
		}
	}
	return retVal___;
}
bool HP_inter_mail_DeleteAttach(int mail_id) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_inter_mail_DeleteAttach_pre > 0) {
		bool (*preHookFunc) (int *mail_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_DeleteAttach_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_mail_DeleteAttach_pre[hIndex].func;
			retVal___ = preHookFunc(&mail_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_mail.DeleteAttach(mail_id);
	}
	if (HPMHooks.count.HP_inter_mail_DeleteAttach_post > 0) {
		bool (*postHookFunc) (bool retVal___, int mail_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_DeleteAttach_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_mail_DeleteAttach_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, mail_id);
		}
	}
	return retVal___;
}
void HP_inter_mail_sendmail(int send_id, const char *send_name, int dest_id, const char *dest_name, const char *title, const char *body, int zeny, struct item *item) {
	int hIndex = 0;
	if (HPMHooks.count.HP_inter_mail_sendmail_pre > 0) {
		void (*preHookFunc) (int *send_id, const char **send_name, int *dest_id, const char **dest_name, const char **title, const char **body, int *zeny, struct item **item);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_sendmail_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_mail_sendmail_pre[hIndex].func;
			preHookFunc(&send_id, &send_name, &dest_id, &dest_name, &title, &body, &zeny, &item);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_mail.sendmail(send_id, send_name, dest_id, dest_name, title, body, zeny, item);
	}
	if (HPMHooks.count.HP_inter_mail_sendmail_post > 0) {
		void (*postHookFunc) (int send_id, const char *send_name, int dest_id, const char *dest_name, const char *title, const char *body, int zeny, struct item *item);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_sendmail_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_mail_sendmail_post[hIndex].func;
			postHookFunc(send_id, send_name, dest_id, dest_name, title, body, zeny, item);
		}
	}
	return;
}
/* inter_mercenary_interface */
bool HP_inter_mercenary_owner_fromsql(int char_id, struct mmo_charstatus *status) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_inter_mercenary_owner_fromsql_pre > 0) {
		bool (*preHookFunc) (int *char_id, struct mmo_charstatus **status);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mercenary_owner_fromsql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_mercenary_owner_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &status);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_mercenary.owner_fromsql(char_id, status);
	}
	if (HPMHooks.count.HP_inter_mercenary_owner_fromsql_post > 0) {
		bool (*postHookFunc) (bool retVal___, int char_id, struct mmo_charstatus *status);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mercenary_owner_fromsql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_mercenary_owner_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, char_id, status);
		}
	}
	return retVal___;
}
bool HP_inter_mercenary_owner_tosql(int char_id, struct mmo_charstatus *status) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_inter_mercenary_owner_tosql_pre > 0) {
		bool (*preHookFunc) (int *char_id, struct mmo_charstatus **status);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mercenary_owner_tosql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_mercenary_owner_tosql_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &status);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_mercenary.owner_tosql(char_id, status);
	}
	if (HPMHooks.count.HP_inter_mercenary_owner_tosql_post > 0) {
		bool (*postHookFunc) (bool retVal___, int char_id, struct mmo_charstatus *status);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mercenary_owner_tosql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_mercenary_owner_tosql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, char_id, status);
		}
	}
	return retVal___;
}
bool HP_inter_mercenary_owner_delete(int char_id) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_inter_mercenary_owner_delete_pre > 0) {
		bool (*preHookFunc) (int *char_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mercenary_owner_delete_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_mercenary_owner_delete_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_mercenary.owner_delete(char_id);
	}
	if (HPMHooks.count.HP_inter_mercenary_owner_delete_post > 0) {
		bool (*postHookFunc) (bool retVal___, int char_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mercenary_owner_delete_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_mercenary_owner_delete_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, char_id);
		}
	}
	return retVal___;
}
int HP_inter_mercenary_sql_init(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_mercenary_sql_init_pre > 0) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mercenary_sql_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_mercenary_sql_init_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_mercenary.sql_init();
	}
	if (HPMHooks.count.HP_inter_mercenary_sql_init_post > 0) {
		int (*postHookFunc) (int retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mercenary_sql_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_mercenary_sql_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_inter_mercenary_sql_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_inter_mercenary_sql_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mercenary_sql_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_mercenary_sql_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_mercenary.sql_final();
	}
	if (HPMHooks.count.HP_inter_mercenary_sql_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mercenary_sql_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_mercenary_sql_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_inter_mercenary_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_mercenary_parse_frommap_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mercenary_parse_frommap_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_mercenary_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_mercenary.parse_frommap(fd);
	}
	if (HPMHooks.count.HP_inter_mercenary_parse_frommap_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_mercenary_parse_frommap_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_mercenary_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
/* inter_party_interface */
int HP_inter_party_check_lv(struct party_data *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_party_check_lv_pre > 0) {
		int (*preHookFunc) (struct party_data **p);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_check_lv_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_party_check_lv_pre[hIndex].func;
			retVal___ = preHookFunc(&p);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_party.check_lv(p);
	}
	if (HPMHooks.count.HP_inter_party_check_lv_post > 0) {
		int (*postHookFunc) (int retVal___, struct party_data *p);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_check_lv_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_party_check_lv_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, p);
		}
	}
	return retVal___;
}
void HP_inter_party_calc_state(struct party_data *p) {
	int hIndex = 0;
	if (HPMHooks.count.HP_inter_party_calc_state_pre > 0) {
		void (*preHookFunc) (struct party_data **p);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_calc_state_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_party_calc_state_pre[hIndex].func;
			preHookFunc(&p);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_party.calc_state(p);
	}
	if (HPMHooks.count.HP_inter_party_calc_state_post > 0) {
		void (*postHookFunc) (struct party_data *p);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_calc_state_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_party_calc_state_post[hIndex].func;
			postHookFunc(p);
		}
	}
	return;
}
int HP_inter_party_tosql(struct party *p, int flag, int index) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_party_tosql_pre > 0) {
		int (*preHookFunc) (struct party **p, int *flag, int *index);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_tosql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_party_tosql_pre[hIndex].func;
			retVal___ = preHookFunc(&p, &flag, &index);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_party.tosql(p, flag, index);
	}
	if (HPMHooks.count.HP_inter_party_tosql_post > 0) {
		int (*postHookFunc) (int retVal___, struct party *p, int flag, int index);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_tosql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_party_tosql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, p, flag, index);
		}
	}
	return retVal___;
}
struct party_data* HP_inter_party_fromsql(int party_id) {
	int hIndex = 0;
	struct party_data* retVal___ = NULL;
	if (HPMHooks.count.HP_inter_party_fromsql_pre > 0) {
		struct party_data* (*preHookFunc) (int *party_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_fromsql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_party_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(&party_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_party.fromsql(party_id);
	}
	if (HPMHooks.count.HP_inter_party_fromsql_post > 0) {
		struct party_data* (*postHookFunc) (struct party_data* retVal___, int party_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_fromsql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_party_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, party_id);
		}
	}
	return retVal___;
}
int HP_inter_party_sql_init(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_party_sql_init_pre > 0) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_sql_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_party_sql_init_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_party.sql_init();
	}
	if (HPMHooks.count.HP_inter_party_sql_init_post > 0) {
		int (*postHookFunc) (int retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_sql_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_party_sql_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_inter_party_sql_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_inter_party_sql_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_sql_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_party_sql_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_party.sql_final();
	}
	if (HPMHooks.count.HP_inter_party_sql_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_sql_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_party_sql_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
struct party_data* HP_inter_party_search_partyname(const char *str) {
	int hIndex = 0;
	struct party_data* retVal___ = NULL;
	if (HPMHooks.count.HP_inter_party_search_partyname_pre > 0) {
		struct party_data* (*preHookFunc) (const char **str);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_search_partyname_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_party_search_partyname_pre[hIndex].func;
			retVal___ = preHookFunc(&str);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_party.search_partyname(str);
	}
	if (HPMHooks.count.HP_inter_party_search_partyname_post > 0) {
		struct party_data* (*postHookFunc) (struct party_data* retVal___, const char *str);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_search_partyname_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_party_search_partyname_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, str);
		}
	}
	return retVal___;
}
int HP_inter_party_check_exp_share(struct party_data *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_party_check_exp_share_pre > 0) {
		int (*preHookFunc) (struct party_data **p);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_check_exp_share_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_party_check_exp_share_pre[hIndex].func;
			retVal___ = preHookFunc(&p);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_party.check_exp_share(p);
	}
	if (HPMHooks.count.HP_inter_party_check_exp_share_post > 0) {
		int (*postHookFunc) (int retVal___, struct party_data *p);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_check_exp_share_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_party_check_exp_share_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, p);
		}
	}
	return retVal___;
}
int HP_inter_party_check_empty(struct party_data *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_party_check_empty_pre > 0) {
		int (*preHookFunc) (struct party_data **p);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_check_empty_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_party_check_empty_pre[hIndex].func;
			retVal___ = preHookFunc(&p);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_party.check_empty(p);
	}
	if (HPMHooks.count.HP_inter_party_check_empty_post > 0) {
		int (*postHookFunc) (int retVal___, struct party_data *p);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_check_empty_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_party_check_empty_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, p);
		}
	}
	return retVal___;
}
int HP_inter_party_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_party_parse_frommap_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_parse_frommap_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_party_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_party.parse_frommap(fd);
	}
	if (HPMHooks.count.HP_inter_party_parse_frommap_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_parse_frommap_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_party_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_inter_party_leave(int party_id, int account_id, int char_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_party_leave_pre > 0) {
		int (*preHookFunc) (int *party_id, int *account_id, int *char_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_leave_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_party_leave_pre[hIndex].func;
			retVal___ = preHookFunc(&party_id, &account_id, &char_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_party.leave(party_id, account_id, char_id);
	}
	if (HPMHooks.count.HP_inter_party_leave_post > 0) {
		int (*postHookFunc) (int retVal___, int party_id, int account_id, int char_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_leave_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_party_leave_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, party_id, account_id, char_id);
		}
	}
	return retVal___;
}
int HP_inter_party_CharOnline(int char_id, int party_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_party_CharOnline_pre > 0) {
		int (*preHookFunc) (int *char_id, int *party_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_CharOnline_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_party_CharOnline_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &party_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_party.CharOnline(char_id, party_id);
	}
	if (HPMHooks.count.HP_inter_party_CharOnline_post > 0) {
		int (*postHookFunc) (int retVal___, int char_id, int party_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_CharOnline_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_party_CharOnline_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, char_id, party_id);
		}
	}
	return retVal___;
}
int HP_inter_party_CharOffline(int char_id, int party_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_party_CharOffline_pre > 0) {
		int (*preHookFunc) (int *char_id, int *party_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_CharOffline_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_party_CharOffline_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &party_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_party.CharOffline(char_id, party_id);
	}
	if (HPMHooks.count.HP_inter_party_CharOffline_post > 0) {
		int (*postHookFunc) (int retVal___, int char_id, int party_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_CharOffline_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_party_CharOffline_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, char_id, party_id);
		}
	}
	return retVal___;
}
/* inter_pet_interface */
int HP_inter_pet_tosql(const struct s_pet *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_pet_tosql_pre > 0) {
		int (*preHookFunc) (const struct s_pet **p);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_pet_tosql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_pet_tosql_pre[hIndex].func;
			retVal___ = preHookFunc(&p);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_pet.tosql(p);
	}
	if (HPMHooks.count.HP_inter_pet_tosql_post > 0) {
		int (*postHookFunc) (int retVal___, const struct s_pet *p);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_pet_tosql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_pet_tosql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, p);
		}
	}
	return retVal___;
}
int HP_inter_pet_fromsql(int pet_id, struct s_pet *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_pet_fromsql_pre > 0) {
		int (*preHookFunc) (int *pet_id, struct s_pet **p);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_pet_fromsql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_pet_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(&pet_id, &p);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_pet.fromsql(pet_id, p);
	}
	if (HPMHooks.count.HP_inter_pet_fromsql_post > 0) {
		int (*postHookFunc) (int retVal___, int pet_id, struct s_pet *p);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_pet_fromsql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_pet_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, pet_id, p);
		}
	}
	return retVal___;
}
int HP_inter_pet_sql_init(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_pet_sql_init_pre > 0) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_pet_sql_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_pet_sql_init_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_pet.sql_init();
	}
	if (HPMHooks.count.HP_inter_pet_sql_init_post > 0) {
		int (*postHookFunc) (int retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_pet_sql_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_pet_sql_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_inter_pet_sql_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_inter_pet_sql_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_pet_sql_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_pet_sql_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_pet.sql_final();
	}
	if (HPMHooks.count.HP_inter_pet_sql_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_pet_sql_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_pet_sql_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_inter_pet_delete_(int pet_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_pet_delete__pre > 0) {
		int (*preHookFunc) (int *pet_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_pet_delete__pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_pet_delete__pre[hIndex].func;
			retVal___ = preHookFunc(&pet_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_pet.delete_(pet_id);
	}
	if (HPMHooks.count.HP_inter_pet_delete__post > 0) {
		int (*postHookFunc) (int retVal___, int pet_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_pet_delete__post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_pet_delete__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, pet_id);
		}
	}
	return retVal___;
}
int HP_inter_pet_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_pet_parse_frommap_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_pet_parse_frommap_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_pet_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_pet.parse_frommap(fd);
	}
	if (HPMHooks.count.HP_inter_pet_parse_frommap_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_pet_parse_frommap_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_pet_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
/* inter_quest_interface */
int HP_inter_quest_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_quest_parse_frommap_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_quest_parse_frommap_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_quest_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_quest.parse_frommap(fd);
	}
	if (HPMHooks.count.HP_inter_quest_parse_frommap_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_quest_parse_frommap_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_quest_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
/* inter_rodex_interface */
int HP_inter_rodex_sql_init(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_rodex_sql_init_pre > 0) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_rodex_sql_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_rodex_sql_init_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_rodex.sql_init();
	}
	if (HPMHooks.count.HP_inter_rodex_sql_init_post > 0) {
		int (*postHookFunc) (int retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_rodex_sql_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_rodex_sql_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_inter_rodex_sql_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_inter_rodex_sql_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_rodex_sql_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_rodex_sql_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_rodex.sql_final();
	}
	if (HPMHooks.count.HP_inter_rodex_sql_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_rodex_sql_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_rodex_sql_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_inter_rodex_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_rodex_parse_frommap_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_rodex_parse_frommap_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_rodex_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_rodex.parse_frommap(fd);
	}
	if (HPMHooks.count.HP_inter_rodex_parse_frommap_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_rodex_parse_frommap_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_rodex_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_inter_rodex_fromsql(int char_id, int account_id, int8 opentype, int64 mail_id, struct rodex_maillist *mails) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_rodex_fromsql_pre > 0) {
		int (*preHookFunc) (int *char_id, int *account_id, int8 *opentype, int64 *mail_id, struct rodex_maillist **mails);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_rodex_fromsql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_rodex_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &account_id, &opentype, &mail_id, &mails);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_rodex.fromsql(char_id, account_id, opentype, mail_id, mails);
	}
	if (HPMHooks.count.HP_inter_rodex_fromsql_post > 0) {
		int (*postHookFunc) (int retVal___, int char_id, int account_id, int8 opentype, int64 mail_id, struct rodex_maillist *mails);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_rodex_fromsql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_rodex_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, char_id, account_id, opentype, mail_id, mails);
		}
	}
	return retVal___;
}
bool HP_inter_rodex_hasnew(int char_id, int account_id) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_inter_rodex_hasnew_pre > 0) {
		bool (*preHookFunc) (int *char_id, int *account_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_rodex_hasnew_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_rodex_hasnew_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &account_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_rodex.hasnew(char_id, account_id);
	}
	if (HPMHooks.count.HP_inter_rodex_hasnew_post > 0) {
		bool (*postHookFunc) (bool retVal___, int char_id, int account_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_rodex_hasnew_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_rodex_hasnew_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, char_id, account_id);
		}
	}
	return retVal___;
}
bool HP_inter_rodex_checkname(const char *name, int *target_char_id, short *target_class, int *target_level) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_inter_rodex_checkname_pre > 0) {
		bool (*preHookFunc) (const char **name, int **target_char_id, short **target_class, int **target_level);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_rodex_checkname_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_rodex_checkname_pre[hIndex].func;
			retVal___ = preHookFunc(&name, &target_char_id, &target_class, &target_level);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_rodex.checkname(name, target_char_id, target_class, target_level);
	}
	if (HPMHooks.count.HP_inter_rodex_checkname_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *name, int *target_char_id, short *target_class, int *target_level);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_rodex_checkname_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_rodex_checkname_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, name, target_char_id, target_class, target_level);
		}
	}
	return retVal___;
}
int64 HP_inter_rodex_savemessage(struct rodex_message *msg) {
	int hIndex = 0;
	int64 retVal___ = 0;
	if (HPMHooks.count.HP_inter_rodex_savemessage_pre > 0) {
		int64 (*preHookFunc) (struct rodex_message **msg);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_rodex_savemessage_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_rodex_savemessage_pre[hIndex].func;
			retVal___ = preHookFunc(&msg);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_rodex.savemessage(msg);
	}
	if (HPMHooks.count.HP_inter_rodex_savemessage_post > 0) {
		int64 (*postHookFunc) (int64 retVal___, struct rodex_message *msg);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_rodex_savemessage_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_rodex_savemessage_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, msg);
		}
	}
	return retVal___;
}
/* inter_storage_interface */
int HP_inter_storage_tosql(int account_id, const struct storage_data *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_storage_tosql_pre > 0) {
		int (*preHookFunc) (int *account_id, const struct storage_data **p);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_tosql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_storage_tosql_pre[hIndex].func;
			retVal___ = preHookFunc(&account_id, &p);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_storage.tosql(account_id, p);
	}
	if (HPMHooks.count.HP_inter_storage_tosql_post > 0) {
		int (*postHookFunc) (int retVal___, int account_id, const struct storage_data *p);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_tosql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_storage_tosql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, account_id, p);
		}
	}
	return retVal___;
}
int HP_inter_storage_fromsql(int account_id, struct storage_data *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_storage_fromsql_pre > 0) {
		int (*preHookFunc) (int *account_id, struct storage_data **p);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_fromsql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_storage_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(&account_id, &p);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_storage.fromsql(account_id, p);
	}
	if (HPMHooks.count.HP_inter_storage_fromsql_post > 0) {
		int (*postHookFunc) (int retVal___, int account_id, struct storage_data *p);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_fromsql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_storage_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, account_id, p);
		}
	}
	return retVal___;
}
int HP_inter_storage_guild_storage_tosql(int guild_id, const struct guild_storage *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_storage_guild_storage_tosql_pre > 0) {
		int (*preHookFunc) (int *guild_id, const struct guild_storage **p);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_guild_storage_tosql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_storage_guild_storage_tosql_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id, &p);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_storage.guild_storage_tosql(guild_id, p);
	}
	if (HPMHooks.count.HP_inter_storage_guild_storage_tosql_post > 0) {
		int (*postHookFunc) (int retVal___, int guild_id, const struct guild_storage *p);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_guild_storage_tosql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_storage_guild_storage_tosql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, guild_id, p);
		}
	}
	return retVal___;
}
int HP_inter_storage_guild_storage_fromsql(int guild_id, struct guild_storage *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_storage_guild_storage_fromsql_pre > 0) {
		int (*preHookFunc) (int *guild_id, struct guild_storage **p);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_guild_storage_fromsql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_storage_guild_storage_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id, &p);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_storage.guild_storage_fromsql(guild_id, p);
	}
	if (HPMHooks.count.HP_inter_storage_guild_storage_fromsql_post > 0) {
		int (*postHookFunc) (int retVal___, int guild_id, struct guild_storage *p);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_guild_storage_fromsql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_storage_guild_storage_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, guild_id, p);
		}
	}
	return retVal___;
}
int HP_inter_storage_sql_init(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_storage_sql_init_pre > 0) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_sql_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_storage_sql_init_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_storage.sql_init();
	}
	if (HPMHooks.count.HP_inter_storage_sql_init_post > 0) {
		int (*postHookFunc) (int retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_sql_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_storage_sql_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_inter_storage_sql_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_inter_storage_sql_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_sql_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_storage_sql_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_storage.sql_final();
	}
	if (HPMHooks.count.HP_inter_storage_sql_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_sql_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_storage_sql_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_inter_storage_delete_(int account_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_storage_delete__pre > 0) {
		int (*preHookFunc) (int *account_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_delete__pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_storage_delete__pre[hIndex].func;
			retVal___ = preHookFunc(&account_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_storage.delete_(account_id);
	}
	if (HPMHooks.count.HP_inter_storage_delete__post > 0) {
		int (*postHookFunc) (int retVal___, int account_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_delete__post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_storage_delete__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, account_id);
		}
	}
	return retVal___;
}
int HP_inter_storage_guild_storage_delete(int guild_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_storage_guild_storage_delete_pre > 0) {
		int (*preHookFunc) (int *guild_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_guild_storage_delete_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_storage_guild_storage_delete_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_storage.guild_storage_delete(guild_id);
	}
	if (HPMHooks.count.HP_inter_storage_guild_storage_delete_post > 0) {
		int (*postHookFunc) (int retVal___, int guild_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_guild_storage_delete_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_storage_guild_storage_delete_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, guild_id);
		}
	}
	return retVal___;
}
int HP_inter_storage_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_inter_storage_parse_frommap_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_parse_frommap_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_inter_storage_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_storage.parse_frommap(fd);
	}
	if (HPMHooks.count.HP_inter_storage_parse_frommap_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_parse_frommap_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_inter_storage_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
/* libconfig_interface */
int HP_libconfig_read(struct config_t *config, FILE *stream) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_read_pre > 0) {
		int (*preHookFunc) (struct config_t **config, FILE **stream);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_read_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_read_pre[hIndex].func;
			retVal___ = preHookFunc(&config, &stream);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.read(config, stream);
	}
	if (HPMHooks.count.HP_libconfig_read_post > 0) {
		int (*postHookFunc) (int retVal___, struct config_t *config, FILE *stream);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_read_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_read_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config, stream);
		}
	}
	return retVal___;
}
void HP_libconfig_write(const struct config_t *config, FILE *stream) {
	int hIndex = 0;
	if (HPMHooks.count.HP_libconfig_write_pre > 0) {
		void (*preHookFunc) (const struct config_t **config, FILE **stream);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_write_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_write_pre[hIndex].func;
			preHookFunc(&config, &stream);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.libconfig.write(config, stream);
	}
	if (HPMHooks.count.HP_libconfig_write_post > 0) {
		void (*postHookFunc) (const struct config_t *config, FILE *stream);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_write_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_write_post[hIndex].func;
			postHookFunc(config, stream);
		}
	}
	return;
}
void HP_libconfig_set_options(struct config_t *config, int options) {
	int hIndex = 0;
	if (HPMHooks.count.HP_libconfig_set_options_pre > 0) {
		void (*preHookFunc) (struct config_t **config, int *options);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_set_options_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_set_options_pre[hIndex].func;
			preHookFunc(&config, &options);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.libconfig.set_options(config, options);
	}
	if (HPMHooks.count.HP_libconfig_set_options_post > 0) {
		void (*postHookFunc) (struct config_t *config, int options);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_set_options_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_set_options_post[hIndex].func;
			postHookFunc(config, options);
		}
	}
	return;
}
int HP_libconfig_get_options(const struct config_t *config) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_get_options_pre > 0) {
		int (*preHookFunc) (const struct config_t **config);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_get_options_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_get_options_pre[hIndex].func;
			retVal___ = preHookFunc(&config);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.get_options(config);
	}
	if (HPMHooks.count.HP_libconfig_get_options_post > 0) {
		int (*postHookFunc) (int retVal___, const struct config_t *config);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_get_options_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_get_options_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config);
		}
	}
	return retVal___;
}
int HP_libconfig_read_string(struct config_t *config, const char *str) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_read_string_pre > 0) {
		int (*preHookFunc) (struct config_t **config, const char **str);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_read_string_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_read_string_pre[hIndex].func;
			retVal___ = preHookFunc(&config, &str);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.read_string(config, str);
	}
	if (HPMHooks.count.HP_libconfig_read_string_post > 0) {
		int (*postHookFunc) (int retVal___, struct config_t *config, const char *str);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_read_string_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_read_string_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config, str);
		}
	}
	return retVal___;
}
int HP_libconfig_read_file_src(struct config_t *config, const char *filename) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_read_file_src_pre > 0) {
		int (*preHookFunc) (struct config_t **config, const char **filename);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_read_file_src_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_read_file_src_pre[hIndex].func;
			retVal___ = preHookFunc(&config, &filename);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.read_file_src(config, filename);
	}
	if (HPMHooks.count.HP_libconfig_read_file_src_post > 0) {
		int (*postHookFunc) (int retVal___, struct config_t *config, const char *filename);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_read_file_src_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_read_file_src_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config, filename);
		}
	}
	return retVal___;
}
int HP_libconfig_write_file(struct config_t *config, const char *filename) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_write_file_pre > 0) {
		int (*preHookFunc) (struct config_t **config, const char **filename);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_write_file_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_write_file_pre[hIndex].func;
			retVal___ = preHookFunc(&config, &filename);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.write_file(config, filename);
	}
	if (HPMHooks.count.HP_libconfig_write_file_post > 0) {
		int (*postHookFunc) (int retVal___, struct config_t *config, const char *filename);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_write_file_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_write_file_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config, filename);
		}
	}
	return retVal___;
}
void HP_libconfig_set_destructor(struct config_t *config, void ( *destructor ) (void *)) {
	int hIndex = 0;
	if (HPMHooks.count.HP_libconfig_set_destructor_pre > 0) {
		void (*preHookFunc) (struct config_t **config, void ( **destructor ) (void *));
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_set_destructor_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_set_destructor_pre[hIndex].func;
			preHookFunc(&config, &destructor);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.libconfig.set_destructor(config, destructor);
	}
	if (HPMHooks.count.HP_libconfig_set_destructor_post > 0) {
		void (*postHookFunc) (struct config_t *config, void ( *destructor ) (void *));
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_set_destructor_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_set_destructor_post[hIndex].func;
			postHookFunc(config, destructor);
		}
	}
	return;
}
void HP_libconfig_set_include_dir(struct config_t *config, const char *include_dir) {
	int hIndex = 0;
	if (HPMHooks.count.HP_libconfig_set_include_dir_pre > 0) {
		void (*preHookFunc) (struct config_t **config, const char **include_dir);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_set_include_dir_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_set_include_dir_pre[hIndex].func;
			preHookFunc(&config, &include_dir);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.libconfig.set_include_dir(config, include_dir);
	}
	if (HPMHooks.count.HP_libconfig_set_include_dir_post > 0) {
		void (*postHookFunc) (struct config_t *config, const char *include_dir);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_set_include_dir_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_set_include_dir_post[hIndex].func;
			postHookFunc(config, include_dir);
		}
	}
	return;
}
void HP_libconfig_init(struct config_t *config) {
	int hIndex = 0;
	if (HPMHooks.count.HP_libconfig_init_pre > 0) {
		void (*preHookFunc) (struct config_t **config);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_init_pre[hIndex].func;
			preHookFunc(&config);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.libconfig.init(config);
	}
	if (HPMHooks.count.HP_libconfig_init_post > 0) {
		void (*postHookFunc) (struct config_t *config);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_init_post[hIndex].func;
			postHookFunc(config);
		}
	}
	return;
}
void HP_libconfig_destroy(struct config_t *config) {
	int hIndex = 0;
	if (HPMHooks.count.HP_libconfig_destroy_pre > 0) {
		void (*preHookFunc) (struct config_t **config);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_destroy_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_destroy_pre[hIndex].func;
			preHookFunc(&config);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.libconfig.destroy(config);
	}
	if (HPMHooks.count.HP_libconfig_destroy_post > 0) {
		void (*postHookFunc) (struct config_t *config);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_destroy_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_destroy_post[hIndex].func;
			postHookFunc(config);
		}
	}
	return;
}
int HP_libconfig_setting_get_int(const struct config_setting_t *setting) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_get_int_pre > 0) {
		int (*preHookFunc) (const struct config_setting_t **setting);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_int_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_int_pre[hIndex].func;
			retVal___ = preHookFunc(&setting);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_int(setting);
	}
	if (HPMHooks.count.HP_libconfig_setting_get_int_post > 0) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_int_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_int_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting);
		}
	}
	return retVal___;
}
int64 HP_libconfig_setting_get_int64(const struct config_setting_t *setting) {
	int hIndex = 0;
	int64 retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_get_int64_pre > 0) {
		int64 (*preHookFunc) (const struct config_setting_t **setting);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_int64_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_int64_pre[hIndex].func;
			retVal___ = preHookFunc(&setting);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_int64(setting);
	}
	if (HPMHooks.count.HP_libconfig_setting_get_int64_post > 0) {
		int64 (*postHookFunc) (int64 retVal___, const struct config_setting_t *setting);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_int64_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_int64_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting);
		}
	}
	return retVal___;
}
double HP_libconfig_setting_get_float(const struct config_setting_t *setting) {
	int hIndex = 0;
	double retVal___ = 0.;
	if (HPMHooks.count.HP_libconfig_setting_get_float_pre > 0) {
		double (*preHookFunc) (const struct config_setting_t **setting);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_float_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_float_pre[hIndex].func;
			retVal___ = preHookFunc(&setting);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_float(setting);
	}
	if (HPMHooks.count.HP_libconfig_setting_get_float_post > 0) {
		double (*postHookFunc) (double retVal___, const struct config_setting_t *setting);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_float_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_float_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_get_bool(const struct config_setting_t *setting) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_get_bool_pre > 0) {
		int (*preHookFunc) (const struct config_setting_t **setting);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_bool_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_bool_pre[hIndex].func;
			retVal___ = preHookFunc(&setting);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_bool(setting);
	}
	if (HPMHooks.count.HP_libconfig_setting_get_bool_post > 0) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_bool_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_bool_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting);
		}
	}
	return retVal___;
}
const char* HP_libconfig_setting_get_string(const struct config_setting_t *setting) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if (HPMHooks.count.HP_libconfig_setting_get_string_pre > 0) {
		const char* (*preHookFunc) (const struct config_setting_t **setting);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_string_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_string_pre[hIndex].func;
			retVal___ = preHookFunc(&setting);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_string(setting);
	}
	if (HPMHooks.count.HP_libconfig_setting_get_string_post > 0) {
		const char* (*postHookFunc) (const char* retVal___, const struct config_setting_t *setting);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_string_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_string_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting);
		}
	}
	return retVal___;
}
struct config_setting_t* HP_libconfig_setting_lookup(struct config_setting_t *setting, const char *name) {
	int hIndex = 0;
	struct config_setting_t* retVal___ = NULL;
	if (HPMHooks.count.HP_libconfig_setting_lookup_pre > 0) {
		struct config_setting_t* (*preHookFunc) (struct config_setting_t **setting, const char **name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_lookup(setting, name);
	}
	if (HPMHooks.count.HP_libconfig_setting_lookup_post > 0) {
		struct config_setting_t* (*postHookFunc) (struct config_setting_t* retVal___, struct config_setting_t *setting, const char *name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, name);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_lookup_int(const struct config_setting_t *setting, const char *name, int *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_lookup_int_pre > 0) {
		int (*preHookFunc) (const struct config_setting_t **setting, const char **name, int **value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_int_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_int_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &name, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_lookup_int(setting, name, value);
	}
	if (HPMHooks.count.HP_libconfig_setting_lookup_int_post > 0) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting, const char *name, int *value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_int_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_int_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, name, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_lookup_int64(const struct config_setting_t *setting, const char *name, int64 *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_lookup_int64_pre > 0) {
		int (*preHookFunc) (const struct config_setting_t **setting, const char **name, int64 **value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_int64_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_int64_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &name, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_lookup_int64(setting, name, value);
	}
	if (HPMHooks.count.HP_libconfig_setting_lookup_int64_post > 0) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting, const char *name, int64 *value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_int64_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_int64_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, name, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_lookup_float(const struct config_setting_t *setting, const char *name, double *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_lookup_float_pre > 0) {
		int (*preHookFunc) (const struct config_setting_t **setting, const char **name, double **value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_float_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_float_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &name, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_lookup_float(setting, name, value);
	}
	if (HPMHooks.count.HP_libconfig_setting_lookup_float_post > 0) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting, const char *name, double *value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_float_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_float_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, name, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_lookup_bool(const struct config_setting_t *setting, const char *name, int *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_lookup_bool_pre > 0) {
		int (*preHookFunc) (const struct config_setting_t **setting, const char **name, int **value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_bool_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_bool_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &name, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_lookup_bool(setting, name, value);
	}
	if (HPMHooks.count.HP_libconfig_setting_lookup_bool_post > 0) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting, const char *name, int *value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_bool_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_bool_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, name, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_lookup_string(const struct config_setting_t *setting, const char *name, const char **value) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_lookup_string_pre > 0) {
		int (*preHookFunc) (const struct config_setting_t **setting, const char **name, const char ***value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_string_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_string_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &name, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_lookup_string(setting, name, value);
	}
	if (HPMHooks.count.HP_libconfig_setting_lookup_string_post > 0) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting, const char *name, const char **value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_string_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_string_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, name, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_set_int(struct config_setting_t *setting, int value) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_set_int_pre > 0) {
		int (*preHookFunc) (struct config_setting_t **setting, int *value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_int_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_set_int_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_set_int(setting, value);
	}
	if (HPMHooks.count.HP_libconfig_setting_set_int_post > 0) {
		int (*postHookFunc) (int retVal___, struct config_setting_t *setting, int value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_int_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_set_int_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_set_int64(struct config_setting_t *setting, int64 value) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_set_int64_pre > 0) {
		int (*preHookFunc) (struct config_setting_t **setting, int64 *value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_int64_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_set_int64_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_set_int64(setting, value);
	}
	if (HPMHooks.count.HP_libconfig_setting_set_int64_post > 0) {
		int (*postHookFunc) (int retVal___, struct config_setting_t *setting, int64 value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_int64_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_set_int64_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_set_float(struct config_setting_t *setting, double value) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_set_float_pre > 0) {
		int (*preHookFunc) (struct config_setting_t **setting, double *value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_float_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_set_float_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_set_float(setting, value);
	}
	if (HPMHooks.count.HP_libconfig_setting_set_float_post > 0) {
		int (*postHookFunc) (int retVal___, struct config_setting_t *setting, double value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_float_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_set_float_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_set_bool(struct config_setting_t *setting, int value) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_set_bool_pre > 0) {
		int (*preHookFunc) (struct config_setting_t **setting, int *value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_bool_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_set_bool_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_set_bool(setting, value);
	}
	if (HPMHooks.count.HP_libconfig_setting_set_bool_post > 0) {
		int (*postHookFunc) (int retVal___, struct config_setting_t *setting, int value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_bool_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_set_bool_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_set_string(struct config_setting_t *setting, const char *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_set_string_pre > 0) {
		int (*preHookFunc) (struct config_setting_t **setting, const char **value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_string_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_set_string_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_set_string(setting, value);
	}
	if (HPMHooks.count.HP_libconfig_setting_set_string_post > 0) {
		int (*postHookFunc) (int retVal___, struct config_setting_t *setting, const char *value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_string_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_set_string_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_set_format(struct config_setting_t *setting, short format) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_set_format_pre > 0) {
		int (*preHookFunc) (struct config_setting_t **setting, short *format);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_format_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_set_format_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &format);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_set_format(setting, format);
	}
	if (HPMHooks.count.HP_libconfig_setting_set_format_post > 0) {
		int (*postHookFunc) (int retVal___, struct config_setting_t *setting, short format);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_format_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_set_format_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, format);
		}
	}
	return retVal___;
}
short HP_libconfig_setting_get_format(const struct config_setting_t *setting) {
	int hIndex = 0;
	short retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_get_format_pre > 0) {
		short (*preHookFunc) (const struct config_setting_t **setting);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_format_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_format_pre[hIndex].func;
			retVal___ = preHookFunc(&setting);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_format(setting);
	}
	if (HPMHooks.count.HP_libconfig_setting_get_format_post > 0) {
		short (*postHookFunc) (short retVal___, const struct config_setting_t *setting);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_format_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_format_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_get_int_elem(const struct config_setting_t *setting, int idx) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_get_int_elem_pre > 0) {
		int (*preHookFunc) (const struct config_setting_t **setting, int *idx);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_int_elem_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_int_elem_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &idx);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_int_elem(setting, idx);
	}
	if (HPMHooks.count.HP_libconfig_setting_get_int_elem_post > 0) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting, int idx);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_int_elem_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_int_elem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, idx);
		}
	}
	return retVal___;
}
int64 HP_libconfig_setting_get_int64_elem(const struct config_setting_t *setting, int idx) {
	int hIndex = 0;
	int64 retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_get_int64_elem_pre > 0) {
		int64 (*preHookFunc) (const struct config_setting_t **setting, int *idx);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_int64_elem_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_int64_elem_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &idx);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_int64_elem(setting, idx);
	}
	if (HPMHooks.count.HP_libconfig_setting_get_int64_elem_post > 0) {
		int64 (*postHookFunc) (int64 retVal___, const struct config_setting_t *setting, int idx);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_int64_elem_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_int64_elem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, idx);
		}
	}
	return retVal___;
}
double HP_libconfig_setting_get_float_elem(const struct config_setting_t *setting, int idx) {
	int hIndex = 0;
	double retVal___ = 0.;
	if (HPMHooks.count.HP_libconfig_setting_get_float_elem_pre > 0) {
		double (*preHookFunc) (const struct config_setting_t **setting, int *idx);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_float_elem_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_float_elem_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &idx);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_float_elem(setting, idx);
	}
	if (HPMHooks.count.HP_libconfig_setting_get_float_elem_post > 0) {
		double (*postHookFunc) (double retVal___, const struct config_setting_t *setting, int idx);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_float_elem_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_float_elem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, idx);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_get_bool_elem(const struct config_setting_t *setting, int idx) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_get_bool_elem_pre > 0) {
		int (*preHookFunc) (const struct config_setting_t **setting, int *idx);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_bool_elem_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_bool_elem_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &idx);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_bool_elem(setting, idx);
	}
	if (HPMHooks.count.HP_libconfig_setting_get_bool_elem_post > 0) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting, int idx);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_bool_elem_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_bool_elem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, idx);
		}
	}
	return retVal___;
}
const char* HP_libconfig_setting_get_string_elem(const struct config_setting_t *setting, int idx) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if (HPMHooks.count.HP_libconfig_setting_get_string_elem_pre > 0) {
		const char* (*preHookFunc) (const struct config_setting_t **setting, int *idx);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_string_elem_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_string_elem_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &idx);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_string_elem(setting, idx);
	}
	if (HPMHooks.count.HP_libconfig_setting_get_string_elem_post > 0) {
		const char* (*postHookFunc) (const char* retVal___, const struct config_setting_t *setting, int idx);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_string_elem_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_string_elem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, idx);
		}
	}
	return retVal___;
}
struct config_setting_t* HP_libconfig_setting_set_int_elem(struct config_setting_t *setting, int idx, int value) {
	int hIndex = 0;
	struct config_setting_t* retVal___ = NULL;
	if (HPMHooks.count.HP_libconfig_setting_set_int_elem_pre > 0) {
		struct config_setting_t* (*preHookFunc) (struct config_setting_t **setting, int *idx, int *value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_int_elem_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_set_int_elem_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &idx, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_set_int_elem(setting, idx, value);
	}
	if (HPMHooks.count.HP_libconfig_setting_set_int_elem_post > 0) {
		struct config_setting_t* (*postHookFunc) (struct config_setting_t* retVal___, struct config_setting_t *setting, int idx, int value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_int_elem_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_set_int_elem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, idx, value);
		}
	}
	return retVal___;
}
struct config_setting_t* HP_libconfig_setting_set_int64_elem(struct config_setting_t *setting, int idx, int64 value) {
	int hIndex = 0;
	struct config_setting_t* retVal___ = NULL;
	if (HPMHooks.count.HP_libconfig_setting_set_int64_elem_pre > 0) {
		struct config_setting_t* (*preHookFunc) (struct config_setting_t **setting, int *idx, int64 *value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_int64_elem_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_set_int64_elem_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &idx, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_set_int64_elem(setting, idx, value);
	}
	if (HPMHooks.count.HP_libconfig_setting_set_int64_elem_post > 0) {
		struct config_setting_t* (*postHookFunc) (struct config_setting_t* retVal___, struct config_setting_t *setting, int idx, int64 value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_int64_elem_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_set_int64_elem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, idx, value);
		}
	}
	return retVal___;
}
struct config_setting_t* HP_libconfig_setting_set_float_elem(struct config_setting_t *setting, int idx, double value) {
	int hIndex = 0;
	struct config_setting_t* retVal___ = NULL;
	if (HPMHooks.count.HP_libconfig_setting_set_float_elem_pre > 0) {
		struct config_setting_t* (*preHookFunc) (struct config_setting_t **setting, int *idx, double *value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_float_elem_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_set_float_elem_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &idx, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_set_float_elem(setting, idx, value);
	}
	if (HPMHooks.count.HP_libconfig_setting_set_float_elem_post > 0) {
		struct config_setting_t* (*postHookFunc) (struct config_setting_t* retVal___, struct config_setting_t *setting, int idx, double value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_float_elem_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_set_float_elem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, idx, value);
		}
	}
	return retVal___;
}
struct config_setting_t* HP_libconfig_setting_set_bool_elem(struct config_setting_t *setting, int idx, int value) {
	int hIndex = 0;
	struct config_setting_t* retVal___ = NULL;
	if (HPMHooks.count.HP_libconfig_setting_set_bool_elem_pre > 0) {
		struct config_setting_t* (*preHookFunc) (struct config_setting_t **setting, int *idx, int *value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_bool_elem_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_set_bool_elem_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &idx, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_set_bool_elem(setting, idx, value);
	}
	if (HPMHooks.count.HP_libconfig_setting_set_bool_elem_post > 0) {
		struct config_setting_t* (*postHookFunc) (struct config_setting_t* retVal___, struct config_setting_t *setting, int idx, int value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_bool_elem_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_set_bool_elem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, idx, value);
		}
	}
	return retVal___;
}
struct config_setting_t* HP_libconfig_setting_set_string_elem(struct config_setting_t *setting, int idx, const char *value) {
	int hIndex = 0;
	struct config_setting_t* retVal___ = NULL;
	if (HPMHooks.count.HP_libconfig_setting_set_string_elem_pre > 0) {
		struct config_setting_t* (*preHookFunc) (struct config_setting_t **setting, int *idx, const char **value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_string_elem_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_set_string_elem_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &idx, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_set_string_elem(setting, idx, value);
	}
	if (HPMHooks.count.HP_libconfig_setting_set_string_elem_post > 0) {
		struct config_setting_t* (*postHookFunc) (struct config_setting_t* retVal___, struct config_setting_t *setting, int idx, const char *value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_string_elem_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_set_string_elem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, idx, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_index(const struct config_setting_t *setting) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_index_pre > 0) {
		int (*preHookFunc) (const struct config_setting_t **setting);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_index_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_index_pre[hIndex].func;
			retVal___ = preHookFunc(&setting);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_index(setting);
	}
	if (HPMHooks.count.HP_libconfig_setting_index_post > 0) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_index_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_index_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_length(const struct config_setting_t *setting) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_length_pre > 0) {
		int (*preHookFunc) (const struct config_setting_t **setting);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_length_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_length_pre[hIndex].func;
			retVal___ = preHookFunc(&setting);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_length(setting);
	}
	if (HPMHooks.count.HP_libconfig_setting_length_post > 0) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_length_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_length_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting);
		}
	}
	return retVal___;
}
struct config_setting_t* HP_libconfig_setting_get_elem(const struct config_setting_t *setting, unsigned int idx) {
	int hIndex = 0;
	struct config_setting_t* retVal___ = NULL;
	if (HPMHooks.count.HP_libconfig_setting_get_elem_pre > 0) {
		struct config_setting_t* (*preHookFunc) (const struct config_setting_t **setting, unsigned int *idx);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_elem_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_elem_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &idx);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_elem(setting, idx);
	}
	if (HPMHooks.count.HP_libconfig_setting_get_elem_post > 0) {
		struct config_setting_t* (*postHookFunc) (struct config_setting_t* retVal___, const struct config_setting_t *setting, unsigned int idx);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_elem_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_elem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, idx);
		}
	}
	return retVal___;
}
struct config_setting_t* HP_libconfig_setting_get_member(const struct config_setting_t *setting, const char *name) {
	int hIndex = 0;
	struct config_setting_t* retVal___ = NULL;
	if (HPMHooks.count.HP_libconfig_setting_get_member_pre > 0) {
		struct config_setting_t* (*preHookFunc) (const struct config_setting_t **setting, const char **name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_member_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_member_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_member(setting, name);
	}
	if (HPMHooks.count.HP_libconfig_setting_get_member_post > 0) {
		struct config_setting_t* (*postHookFunc) (struct config_setting_t* retVal___, const struct config_setting_t *setting, const char *name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_member_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_member_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, name);
		}
	}
	return retVal___;
}
struct config_setting_t* HP_libconfig_setting_add(struct config_setting_t *parent, const char *name, int type) {
	int hIndex = 0;
	struct config_setting_t* retVal___ = NULL;
	if (HPMHooks.count.HP_libconfig_setting_add_pre > 0) {
		struct config_setting_t* (*preHookFunc) (struct config_setting_t **parent, const char **name, int *type);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_add_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_add_pre[hIndex].func;
			retVal___ = preHookFunc(&parent, &name, &type);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_add(parent, name, type);
	}
	if (HPMHooks.count.HP_libconfig_setting_add_post > 0) {
		struct config_setting_t* (*postHookFunc) (struct config_setting_t* retVal___, struct config_setting_t *parent, const char *name, int type);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_add_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_add_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent, name, type);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_remove(struct config_setting_t *parent, const char *name) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_remove_pre > 0) {
		int (*preHookFunc) (struct config_setting_t **parent, const char **name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_remove_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_remove_pre[hIndex].func;
			retVal___ = preHookFunc(&parent, &name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_remove(parent, name);
	}
	if (HPMHooks.count.HP_libconfig_setting_remove_post > 0) {
		int (*postHookFunc) (int retVal___, struct config_setting_t *parent, const char *name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_remove_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_remove_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent, name);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_remove_elem(struct config_setting_t *parent, unsigned int idx) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_remove_elem_pre > 0) {
		int (*preHookFunc) (struct config_setting_t **parent, unsigned int *idx);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_remove_elem_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_remove_elem_pre[hIndex].func;
			retVal___ = preHookFunc(&parent, &idx);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_remove_elem(parent, idx);
	}
	if (HPMHooks.count.HP_libconfig_setting_remove_elem_post > 0) {
		int (*postHookFunc) (int retVal___, struct config_setting_t *parent, unsigned int idx);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_remove_elem_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_remove_elem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent, idx);
		}
	}
	return retVal___;
}
void HP_libconfig_setting_set_hook(struct config_setting_t *setting, void *hook) {
	int hIndex = 0;
	if (HPMHooks.count.HP_libconfig_setting_set_hook_pre > 0) {
		void (*preHookFunc) (struct config_setting_t **setting, void **hook);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_hook_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_set_hook_pre[hIndex].func;
			preHookFunc(&setting, &hook);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.libconfig.setting_set_hook(setting, hook);
	}
	if (HPMHooks.count.HP_libconfig_setting_set_hook_post > 0) {
		void (*postHookFunc) (struct config_setting_t *setting, void *hook);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_set_hook_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_set_hook_post[hIndex].func;
			postHookFunc(setting, hook);
		}
	}
	return;
}
struct config_setting_t* HP_libconfig_lookup(const struct config_t *config, const char *filepath) {
	int hIndex = 0;
	struct config_setting_t* retVal___ = NULL;
	if (HPMHooks.count.HP_libconfig_lookup_pre > 0) {
		struct config_setting_t* (*preHookFunc) (const struct config_t **config, const char **filepath);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_lookup_pre[hIndex].func;
			retVal___ = preHookFunc(&config, &filepath);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.lookup(config, filepath);
	}
	if (HPMHooks.count.HP_libconfig_lookup_post > 0) {
		struct config_setting_t* (*postHookFunc) (struct config_setting_t* retVal___, const struct config_t *config, const char *filepath);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_lookup_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config, filepath);
		}
	}
	return retVal___;
}
int HP_libconfig_lookup_int(const struct config_t *config, const char *filepath, int *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_lookup_int_pre > 0) {
		int (*preHookFunc) (const struct config_t **config, const char **filepath, int **value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_int_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_lookup_int_pre[hIndex].func;
			retVal___ = preHookFunc(&config, &filepath, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.lookup_int(config, filepath, value);
	}
	if (HPMHooks.count.HP_libconfig_lookup_int_post > 0) {
		int (*postHookFunc) (int retVal___, const struct config_t *config, const char *filepath, int *value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_int_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_lookup_int_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config, filepath, value);
		}
	}
	return retVal___;
}
int HP_libconfig_lookup_int64(const struct config_t *config, const char *filepath, int64 *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_lookup_int64_pre > 0) {
		int (*preHookFunc) (const struct config_t **config, const char **filepath, int64 **value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_int64_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_lookup_int64_pre[hIndex].func;
			retVal___ = preHookFunc(&config, &filepath, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.lookup_int64(config, filepath, value);
	}
	if (HPMHooks.count.HP_libconfig_lookup_int64_post > 0) {
		int (*postHookFunc) (int retVal___, const struct config_t *config, const char *filepath, int64 *value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_int64_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_lookup_int64_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config, filepath, value);
		}
	}
	return retVal___;
}
int HP_libconfig_lookup_float(const struct config_t *config, const char *filepath, double *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_lookup_float_pre > 0) {
		int (*preHookFunc) (const struct config_t **config, const char **filepath, double **value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_float_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_lookup_float_pre[hIndex].func;
			retVal___ = preHookFunc(&config, &filepath, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.lookup_float(config, filepath, value);
	}
	if (HPMHooks.count.HP_libconfig_lookup_float_post > 0) {
		int (*postHookFunc) (int retVal___, const struct config_t *config, const char *filepath, double *value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_float_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_lookup_float_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config, filepath, value);
		}
	}
	return retVal___;
}
int HP_libconfig_lookup_bool(const struct config_t *config, const char *filepath, int *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_lookup_bool_pre > 0) {
		int (*preHookFunc) (const struct config_t **config, const char **filepath, int **value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_bool_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_lookup_bool_pre[hIndex].func;
			retVal___ = preHookFunc(&config, &filepath, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.lookup_bool(config, filepath, value);
	}
	if (HPMHooks.count.HP_libconfig_lookup_bool_post > 0) {
		int (*postHookFunc) (int retVal___, const struct config_t *config, const char *filepath, int *value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_bool_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_lookup_bool_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config, filepath, value);
		}
	}
	return retVal___;
}
int HP_libconfig_lookup_string(const struct config_t *config, const char *filepath, const char **value) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_lookup_string_pre > 0) {
		int (*preHookFunc) (const struct config_t **config, const char **filepath, const char ***value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_string_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_lookup_string_pre[hIndex].func;
			retVal___ = preHookFunc(&config, &filepath, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.lookup_string(config, filepath, value);
	}
	if (HPMHooks.count.HP_libconfig_lookup_string_post > 0) {
		int (*postHookFunc) (int retVal___, const struct config_t *config, const char *filepath, const char **value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_string_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_lookup_string_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config, filepath, value);
		}
	}
	return retVal___;
}
int HP_libconfig_load_file(struct config_t *config, const char *config_filename) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_load_file_pre > 0) {
		int (*preHookFunc) (struct config_t **config, const char **config_filename);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_load_file_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_load_file_pre[hIndex].func;
			retVal___ = preHookFunc(&config, &config_filename);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.load_file(config, config_filename);
	}
	if (HPMHooks.count.HP_libconfig_load_file_post > 0) {
		int (*postHookFunc) (int retVal___, struct config_t *config, const char *config_filename);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_load_file_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_load_file_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config, config_filename);
		}
	}
	return retVal___;
}
void HP_libconfig_setting_copy_simple(struct config_setting_t *parent, const struct config_setting_t *src) {
	int hIndex = 0;
	if (HPMHooks.count.HP_libconfig_setting_copy_simple_pre > 0) {
		void (*preHookFunc) (struct config_setting_t **parent, const struct config_setting_t **src);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_copy_simple_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_copy_simple_pre[hIndex].func;
			preHookFunc(&parent, &src);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.libconfig.setting_copy_simple(parent, src);
	}
	if (HPMHooks.count.HP_libconfig_setting_copy_simple_post > 0) {
		void (*postHookFunc) (struct config_setting_t *parent, const struct config_setting_t *src);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_copy_simple_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_copy_simple_post[hIndex].func;
			postHookFunc(parent, src);
		}
	}
	return;
}
void HP_libconfig_setting_copy_elem(struct config_setting_t *parent, const struct config_setting_t *src) {
	int hIndex = 0;
	if (HPMHooks.count.HP_libconfig_setting_copy_elem_pre > 0) {
		void (*preHookFunc) (struct config_setting_t **parent, const struct config_setting_t **src);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_copy_elem_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_copy_elem_pre[hIndex].func;
			preHookFunc(&parent, &src);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.libconfig.setting_copy_elem(parent, src);
	}
	if (HPMHooks.count.HP_libconfig_setting_copy_elem_post > 0) {
		void (*postHookFunc) (struct config_setting_t *parent, const struct config_setting_t *src);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_copy_elem_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_copy_elem_post[hIndex].func;
			postHookFunc(parent, src);
		}
	}
	return;
}
void HP_libconfig_setting_copy_aggregate(struct config_setting_t *parent, const struct config_setting_t *src) {
	int hIndex = 0;
	if (HPMHooks.count.HP_libconfig_setting_copy_aggregate_pre > 0) {
		void (*preHookFunc) (struct config_setting_t **parent, const struct config_setting_t **src);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_copy_aggregate_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_copy_aggregate_pre[hIndex].func;
			preHookFunc(&parent, &src);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.libconfig.setting_copy_aggregate(parent, src);
	}
	if (HPMHooks.count.HP_libconfig_setting_copy_aggregate_post > 0) {
		void (*postHookFunc) (struct config_setting_t *parent, const struct config_setting_t *src);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_copy_aggregate_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_copy_aggregate_post[hIndex].func;
			postHookFunc(parent, src);
		}
	}
	return;
}
int HP_libconfig_setting_copy(struct config_setting_t *parent, const struct config_setting_t *src) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_copy_pre > 0) {
		int (*preHookFunc) (struct config_setting_t **parent, const struct config_setting_t **src);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_copy_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_copy_pre[hIndex].func;
			retVal___ = preHookFunc(&parent, &src);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_copy(parent, src);
	}
	if (HPMHooks.count.HP_libconfig_setting_copy_post > 0) {
		int (*postHookFunc) (int retVal___, struct config_setting_t *parent, const struct config_setting_t *src);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_copy_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_copy_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent, src);
		}
	}
	return retVal___;
}
bool HP_libconfig_setting_get_bool_real(const struct config_setting_t *setting) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_libconfig_setting_get_bool_real_pre > 0) {
		bool (*preHookFunc) (const struct config_setting_t **setting);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_bool_real_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_bool_real_pre[hIndex].func;
			retVal___ = preHookFunc(&setting);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_bool_real(setting);
	}
	if (HPMHooks.count.HP_libconfig_setting_get_bool_real_post > 0) {
		bool (*postHookFunc) (bool retVal___, const struct config_setting_t *setting);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_bool_real_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_bool_real_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting);
		}
	}
	return retVal___;
}
uint32 HP_libconfig_setting_get_uint32(const struct config_setting_t *setting) {
	int hIndex = 0;
	uint32 retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_get_uint32_pre > 0) {
		uint32 (*preHookFunc) (const struct config_setting_t **setting);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_uint32_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_uint32_pre[hIndex].func;
			retVal___ = preHookFunc(&setting);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_uint32(setting);
	}
	if (HPMHooks.count.HP_libconfig_setting_get_uint32_post > 0) {
		uint32 (*postHookFunc) (uint32 retVal___, const struct config_setting_t *setting);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_uint32_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_uint32_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting);
		}
	}
	return retVal___;
}
uint16 HP_libconfig_setting_get_uint16(const struct config_setting_t *setting) {
	int hIndex = 0;
	uint16 retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_get_uint16_pre > 0) {
		uint16 (*preHookFunc) (const struct config_setting_t **setting);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_uint16_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_uint16_pre[hIndex].func;
			retVal___ = preHookFunc(&setting);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_uint16(setting);
	}
	if (HPMHooks.count.HP_libconfig_setting_get_uint16_post > 0) {
		uint16 (*postHookFunc) (uint16 retVal___, const struct config_setting_t *setting);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_uint16_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_uint16_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting);
		}
	}
	return retVal___;
}
int16 HP_libconfig_setting_get_int16(const struct config_setting_t *setting) {
	int hIndex = 0;
	int16 retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_get_int16_pre > 0) {
		int16 (*preHookFunc) (const struct config_setting_t **setting);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_int16_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_get_int16_pre[hIndex].func;
			retVal___ = preHookFunc(&setting);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_get_int16(setting);
	}
	if (HPMHooks.count.HP_libconfig_setting_get_int16_post > 0) {
		int16 (*postHookFunc) (int16 retVal___, const struct config_setting_t *setting);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_get_int16_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_get_int16_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_lookup_bool_real(const struct config_setting_t *setting, const char *name, bool *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_lookup_bool_real_pre > 0) {
		int (*preHookFunc) (const struct config_setting_t **setting, const char **name, bool **value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_bool_real_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_bool_real_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &name, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_lookup_bool_real(setting, name, value);
	}
	if (HPMHooks.count.HP_libconfig_setting_lookup_bool_real_post > 0) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting, const char *name, bool *value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_bool_real_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_bool_real_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, name, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_lookup_uint32(const struct config_setting_t *setting, const char *name, uint32 *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_lookup_uint32_pre > 0) {
		int (*preHookFunc) (const struct config_setting_t **setting, const char **name, uint32 **value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_uint32_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_uint32_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &name, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_lookup_uint32(setting, name, value);
	}
	if (HPMHooks.count.HP_libconfig_setting_lookup_uint32_post > 0) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting, const char *name, uint32 *value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_uint32_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_uint32_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, name, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_lookup_uint16(const struct config_setting_t *setting, const char *name, uint16 *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_lookup_uint16_pre > 0) {
		int (*preHookFunc) (const struct config_setting_t **setting, const char **name, uint16 **value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_uint16_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_uint16_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &name, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_lookup_uint16(setting, name, value);
	}
	if (HPMHooks.count.HP_libconfig_setting_lookup_uint16_post > 0) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting, const char *name, uint16 *value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_uint16_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_uint16_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, name, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_lookup_int16(const struct config_setting_t *setting, const char *name, int16 *value) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_lookup_int16_pre > 0) {
		int (*preHookFunc) (const struct config_setting_t **setting, const char **name, int16 **value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_int16_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_int16_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &name, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_lookup_int16(setting, name, value);
	}
	if (HPMHooks.count.HP_libconfig_setting_lookup_int16_post > 0) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting, const char *name, int16 *value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_int16_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_int16_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, name, value);
		}
	}
	return retVal___;
}
int HP_libconfig_setting_lookup_mutable_string(const struct config_setting_t *setting, const char *name, char *out, size_t out_size) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_setting_lookup_mutable_string_pre > 0) {
		int (*preHookFunc) (const struct config_setting_t **setting, const char **name, char **out, size_t *out_size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_mutable_string_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_mutable_string_pre[hIndex].func;
			retVal___ = preHookFunc(&setting, &name, &out, &out_size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.setting_lookup_mutable_string(setting, name, out, out_size);
	}
	if (HPMHooks.count.HP_libconfig_setting_lookup_mutable_string_post > 0) {
		int (*postHookFunc) (int retVal___, const struct config_setting_t *setting, const char *name, char *out, size_t out_size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_setting_lookup_mutable_string_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_setting_lookup_mutable_string_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, setting, name, out, out_size);
		}
	}
	return retVal___;
}
int HP_libconfig_lookup_mutable_string(const struct config_t *config, const char *name, char *out, size_t out_size) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_libconfig_lookup_mutable_string_pre > 0) {
		int (*preHookFunc) (const struct config_t **config, const char **name, char **out, size_t *out_size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_mutable_string_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_lookup_mutable_string_pre[hIndex].func;
			retVal___ = preHookFunc(&config, &name, &out, &out_size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.libconfig.lookup_mutable_string(config, name, out, out_size);
	}
	if (HPMHooks.count.HP_libconfig_lookup_mutable_string_post > 0) {
		int (*postHookFunc) (int retVal___, const struct config_t *config, const char *name, char *out, size_t out_size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_lookup_mutable_string_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_lookup_mutable_string_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, config, name, out, out_size);
		}
	}
	return retVal___;
}
/* loginif_interface */
void HP_loginif_init(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_loginif_init_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_loginif_init_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.init();
	}
	if (HPMHooks.count.HP_loginif_init_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_loginif_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_loginif_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_loginif_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_loginif_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.final();
	}
	if (HPMHooks.count.HP_loginif_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_loginif_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_loginif_reset(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_loginif_reset_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_reset_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_loginif_reset_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.reset();
	}
	if (HPMHooks.count.HP_loginif_reset_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_reset_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_loginif_reset_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_loginif_check_shutdown(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_loginif_check_shutdown_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_check_shutdown_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_loginif_check_shutdown_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.check_shutdown();
	}
	if (HPMHooks.count.HP_loginif_check_shutdown_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_check_shutdown_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_loginif_check_shutdown_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_loginif_on_disconnect(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_loginif_on_disconnect_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_on_disconnect_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_loginif_on_disconnect_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.on_disconnect();
	}
	if (HPMHooks.count.HP_loginif_on_disconnect_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_on_disconnect_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_loginif_on_disconnect_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_loginif_on_ready(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_loginif_on_ready_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_on_ready_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_loginif_on_ready_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.on_ready();
	}
	if (HPMHooks.count.HP_loginif_on_ready_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_on_ready_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_loginif_on_ready_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_loginif_block_account(int account_id, int flag) {
	int hIndex = 0;
	if (HPMHooks.count.HP_loginif_block_account_pre > 0) {
		void (*preHookFunc) (int *account_id, int *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_block_account_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_loginif_block_account_pre[hIndex].func;
			preHookFunc(&account_id, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.block_account(account_id, flag);
	}
	if (HPMHooks.count.HP_loginif_block_account_post > 0) {
		void (*postHookFunc) (int account_id, int flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_block_account_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_loginif_block_account_post[hIndex].func;
			postHookFunc(account_id, flag);
		}
	}
	return;
}
void HP_loginif_ban_account(int account_id, short year, short month, short day, short hour, short minute, short second) {
	int hIndex = 0;
	if (HPMHooks.count.HP_loginif_ban_account_pre > 0) {
		void (*preHookFunc) (int *account_id, short *year, short *month, short *day, short *hour, short *minute, short *second);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_ban_account_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_loginif_ban_account_pre[hIndex].func;
			preHookFunc(&account_id, &year, &month, &day, &hour, &minute, &second);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.ban_account(account_id, year, month, day, hour, minute, second);
	}
	if (HPMHooks.count.HP_loginif_ban_account_post > 0) {
		void (*postHookFunc) (int account_id, short year, short month, short day, short hour, short minute, short second);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_ban_account_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_loginif_ban_account_post[hIndex].func;
			postHookFunc(account_id, year, month, day, hour, minute, second);
		}
	}
	return;
}
void HP_loginif_unban_account(int account_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_loginif_unban_account_pre > 0) {
		void (*preHookFunc) (int *account_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_unban_account_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_loginif_unban_account_pre[hIndex].func;
			preHookFunc(&account_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.unban_account(account_id);
	}
	if (HPMHooks.count.HP_loginif_unban_account_post > 0) {
		void (*postHookFunc) (int account_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_unban_account_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_loginif_unban_account_post[hIndex].func;
			postHookFunc(account_id);
		}
	}
	return;
}
void HP_loginif_changesex(int account_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_loginif_changesex_pre > 0) {
		void (*preHookFunc) (int *account_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_changesex_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_loginif_changesex_pre[hIndex].func;
			preHookFunc(&account_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.changesex(account_id);
	}
	if (HPMHooks.count.HP_loginif_changesex_post > 0) {
		void (*postHookFunc) (int account_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_changesex_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_loginif_changesex_post[hIndex].func;
			postHookFunc(account_id);
		}
	}
	return;
}
void HP_loginif_auth(int fd, struct char_session_data *sd, uint32 ipl) {
	int hIndex = 0;
	if (HPMHooks.count.HP_loginif_auth_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd, uint32 *ipl);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_auth_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_loginif_auth_pre[hIndex].func;
			preHookFunc(&fd, &sd, &ipl);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.auth(fd, sd, ipl);
	}
	if (HPMHooks.count.HP_loginif_auth_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd, uint32 ipl);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_auth_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_loginif_auth_post[hIndex].func;
			postHookFunc(fd, sd, ipl);
		}
	}
	return;
}
void HP_loginif_send_users_count(int users) {
	int hIndex = 0;
	if (HPMHooks.count.HP_loginif_send_users_count_pre > 0) {
		void (*preHookFunc) (int *users);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_send_users_count_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_loginif_send_users_count_pre[hIndex].func;
			preHookFunc(&users);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.send_users_count(users);
	}
	if (HPMHooks.count.HP_loginif_send_users_count_post > 0) {
		void (*postHookFunc) (int users);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_send_users_count_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_loginif_send_users_count_post[hIndex].func;
			postHookFunc(users);
		}
	}
	return;
}
void HP_loginif_connect_to_server(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_loginif_connect_to_server_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_connect_to_server_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_loginif_connect_to_server_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.connect_to_server();
	}
	if (HPMHooks.count.HP_loginif_connect_to_server_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_loginif_connect_to_server_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_loginif_connect_to_server_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
/* mapif_interface */
void HP_mapif_ban(int id, unsigned int flag, int status) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_ban_pre > 0) {
		void (*preHookFunc) (int *id, unsigned int *flag, int *status);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_ban_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_ban_pre[hIndex].func;
			preHookFunc(&id, &flag, &status);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.ban(id, flag, status);
	}
	if (HPMHooks.count.HP_mapif_ban_post > 0) {
		void (*postHookFunc) (int id, unsigned int flag, int status);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_ban_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_ban_post[hIndex].func;
			postHookFunc(id, flag, status);
		}
	}
	return;
}
void HP_mapif_server_init(int id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_server_init_pre > 0) {
		void (*preHookFunc) (int *id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_server_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_server_init_pre[hIndex].func;
			preHookFunc(&id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.server_init(id);
	}
	if (HPMHooks.count.HP_mapif_server_init_post > 0) {
		void (*postHookFunc) (int id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_server_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_server_init_post[hIndex].func;
			postHookFunc(id);
		}
	}
	return;
}
void HP_mapif_server_destroy(int id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_server_destroy_pre > 0) {
		void (*preHookFunc) (int *id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_server_destroy_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_server_destroy_pre[hIndex].func;
			preHookFunc(&id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.server_destroy(id);
	}
	if (HPMHooks.count.HP_mapif_server_destroy_post > 0) {
		void (*postHookFunc) (int id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_server_destroy_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_server_destroy_post[hIndex].func;
			postHookFunc(id);
		}
	}
	return;
}
void HP_mapif_server_reset(int id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_server_reset_pre > 0) {
		void (*preHookFunc) (int *id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_server_reset_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_server_reset_pre[hIndex].func;
			preHookFunc(&id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.server_reset(id);
	}
	if (HPMHooks.count.HP_mapif_server_reset_post > 0) {
		void (*postHookFunc) (int id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_server_reset_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_server_reset_post[hIndex].func;
			postHookFunc(id);
		}
	}
	return;
}
void HP_mapif_on_disconnect(int id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_on_disconnect_pre > 0) {
		void (*preHookFunc) (int *id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_on_disconnect_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_on_disconnect_pre[hIndex].func;
			preHookFunc(&id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.on_disconnect(id);
	}
	if (HPMHooks.count.HP_mapif_on_disconnect_post > 0) {
		void (*postHookFunc) (int id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_on_disconnect_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_on_disconnect_post[hIndex].func;
			postHookFunc(id);
		}
	}
	return;
}
void HP_mapif_on_parse_accinfo(int account_id, int u_fd, int u_aid, int u_group, int map_fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_on_parse_accinfo_pre > 0) {
		void (*preHookFunc) (int *account_id, int *u_fd, int *u_aid, int *u_group, int *map_fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_on_parse_accinfo_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_on_parse_accinfo_pre[hIndex].func;
			preHookFunc(&account_id, &u_fd, &u_aid, &u_group, &map_fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.on_parse_accinfo(account_id, u_fd, u_aid, u_group, map_fd);
	}
	if (HPMHooks.count.HP_mapif_on_parse_accinfo_post > 0) {
		void (*postHookFunc) (int account_id, int u_fd, int u_aid, int u_group, int map_fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_on_parse_accinfo_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_on_parse_accinfo_post[hIndex].func;
			postHookFunc(account_id, u_fd, u_aid, u_group, map_fd);
		}
	}
	return;
}
void HP_mapif_char_ban(int char_id, time_t timestamp) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_char_ban_pre > 0) {
		void (*preHookFunc) (int *char_id, time_t *timestamp);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_char_ban_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_char_ban_pre[hIndex].func;
			preHookFunc(&char_id, &timestamp);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.char_ban(char_id, timestamp);
	}
	if (HPMHooks.count.HP_mapif_char_ban_post > 0) {
		void (*postHookFunc) (int char_id, time_t timestamp);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_char_ban_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_char_ban_post[hIndex].func;
			postHookFunc(char_id, timestamp);
		}
	}
	return;
}
int HP_mapif_sendall(const unsigned char *buf, unsigned int len) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_sendall_pre > 0) {
		int (*preHookFunc) (const unsigned char **buf, unsigned int *len);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_sendall_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_sendall_pre[hIndex].func;
			retVal___ = preHookFunc(&buf, &len);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.sendall(buf, len);
	}
	if (HPMHooks.count.HP_mapif_sendall_post > 0) {
		int (*postHookFunc) (int retVal___, const unsigned char *buf, unsigned int len);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_sendall_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_sendall_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, buf, len);
		}
	}
	return retVal___;
}
int HP_mapif_sendallwos(int sfd, unsigned char *buf, unsigned int len) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_sendallwos_pre > 0) {
		int (*preHookFunc) (int *sfd, unsigned char **buf, unsigned int *len);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_sendallwos_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_sendallwos_pre[hIndex].func;
			retVal___ = preHookFunc(&sfd, &buf, &len);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.sendallwos(sfd, buf, len);
	}
	if (HPMHooks.count.HP_mapif_sendallwos_post > 0) {
		int (*postHookFunc) (int retVal___, int sfd, unsigned char *buf, unsigned int len);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_sendallwos_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_sendallwos_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, sfd, buf, len);
		}
	}
	return retVal___;
}
int HP_mapif_send(int fd, unsigned char *buf, unsigned int len) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_send_pre > 0) {
		int (*preHookFunc) (int *fd, unsigned char **buf, unsigned int *len);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_send_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_send_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &buf, &len);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.send(fd, buf, len);
	}
	if (HPMHooks.count.HP_mapif_send_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, unsigned char *buf, unsigned int len);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_send_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_send_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, buf, len);
		}
	}
	return retVal___;
}
void HP_mapif_send_users_count(int users) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_send_users_count_pre > 0) {
		void (*preHookFunc) (int *users);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_send_users_count_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_send_users_count_pre[hIndex].func;
			preHookFunc(&users);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.send_users_count(users);
	}
	if (HPMHooks.count.HP_mapif_send_users_count_post > 0) {
		void (*postHookFunc) (int users);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_send_users_count_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_send_users_count_post[hIndex].func;
			postHookFunc(users);
		}
	}
	return;
}
void HP_mapif_auction_message(int char_id, unsigned char result) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_auction_message_pre > 0) {
		void (*preHookFunc) (int *char_id, unsigned char *result);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_auction_message_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_auction_message_pre[hIndex].func;
			preHookFunc(&char_id, &result);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.auction_message(char_id, result);
	}
	if (HPMHooks.count.HP_mapif_auction_message_post > 0) {
		void (*postHookFunc) (int char_id, unsigned char result);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_auction_message_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_auction_message_post[hIndex].func;
			postHookFunc(char_id, result);
		}
	}
	return;
}
void HP_mapif_auction_sendlist(int fd, int char_id, short count, short pages, unsigned char *buf) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_auction_sendlist_pre > 0) {
		void (*preHookFunc) (int *fd, int *char_id, short *count, short *pages, unsigned char **buf);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_auction_sendlist_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_auction_sendlist_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &count, &pages, &buf);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.auction_sendlist(fd, char_id, count, pages, buf);
	}
	if (HPMHooks.count.HP_mapif_auction_sendlist_post > 0) {
		void (*postHookFunc) (int fd, int char_id, short count, short pages, unsigned char *buf);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_auction_sendlist_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_auction_sendlist_post[hIndex].func;
			postHookFunc(fd, char_id, count, pages, buf);
		}
	}
	return;
}
void HP_mapif_parse_auction_requestlist(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_auction_requestlist_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_auction_requestlist_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_auction_requestlist_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_auction_requestlist(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_auction_requestlist_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_auction_requestlist_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_auction_requestlist_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_mapif_auction_register(int fd, struct auction_data *auction) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_auction_register_pre > 0) {
		void (*preHookFunc) (int *fd, struct auction_data **auction);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_auction_register_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_auction_register_pre[hIndex].func;
			preHookFunc(&fd, &auction);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.auction_register(fd, auction);
	}
	if (HPMHooks.count.HP_mapif_auction_register_post > 0) {
		void (*postHookFunc) (int fd, struct auction_data *auction);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_auction_register_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_auction_register_post[hIndex].func;
			postHookFunc(fd, auction);
		}
	}
	return;
}
void HP_mapif_parse_auction_register(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_auction_register_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_auction_register_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_auction_register_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_auction_register(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_auction_register_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_auction_register_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_auction_register_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_mapif_auction_cancel(int fd, int char_id, unsigned char result) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_auction_cancel_pre > 0) {
		void (*preHookFunc) (int *fd, int *char_id, unsigned char *result);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_auction_cancel_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_auction_cancel_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &result);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.auction_cancel(fd, char_id, result);
	}
	if (HPMHooks.count.HP_mapif_auction_cancel_post > 0) {
		void (*postHookFunc) (int fd, int char_id, unsigned char result);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_auction_cancel_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_auction_cancel_post[hIndex].func;
			postHookFunc(fd, char_id, result);
		}
	}
	return;
}
void HP_mapif_parse_auction_cancel(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_auction_cancel_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_auction_cancel_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_auction_cancel_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_auction_cancel(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_auction_cancel_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_auction_cancel_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_auction_cancel_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_mapif_auction_close(int fd, int char_id, unsigned char result) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_auction_close_pre > 0) {
		void (*preHookFunc) (int *fd, int *char_id, unsigned char *result);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_auction_close_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_auction_close_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &result);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.auction_close(fd, char_id, result);
	}
	if (HPMHooks.count.HP_mapif_auction_close_post > 0) {
		void (*postHookFunc) (int fd, int char_id, unsigned char result);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_auction_close_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_auction_close_post[hIndex].func;
			postHookFunc(fd, char_id, result);
		}
	}
	return;
}
void HP_mapif_parse_auction_close(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_auction_close_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_auction_close_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_auction_close_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_auction_close(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_auction_close_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_auction_close_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_auction_close_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_mapif_auction_bid(int fd, int char_id, int bid, unsigned char result) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_auction_bid_pre > 0) {
		void (*preHookFunc) (int *fd, int *char_id, int *bid, unsigned char *result);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_auction_bid_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_auction_bid_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &bid, &result);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.auction_bid(fd, char_id, bid, result);
	}
	if (HPMHooks.count.HP_mapif_auction_bid_post > 0) {
		void (*postHookFunc) (int fd, int char_id, int bid, unsigned char result);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_auction_bid_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_auction_bid_post[hIndex].func;
			postHookFunc(fd, char_id, bid, result);
		}
	}
	return;
}
void HP_mapif_parse_auction_bid(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_auction_bid_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_auction_bid_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_auction_bid_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_auction_bid(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_auction_bid_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_auction_bid_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_auction_bid_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
bool HP_mapif_elemental_create(struct s_elemental *ele) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_mapif_elemental_create_pre > 0) {
		bool (*preHookFunc) (struct s_elemental **ele);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_create_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_elemental_create_pre[hIndex].func;
			retVal___ = preHookFunc(&ele);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.elemental_create(ele);
	}
	if (HPMHooks.count.HP_mapif_elemental_create_post > 0) {
		bool (*postHookFunc) (bool retVal___, struct s_elemental *ele);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_create_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_elemental_create_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, ele);
		}
	}
	return retVal___;
}
bool HP_mapif_elemental_save(const struct s_elemental *ele) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_mapif_elemental_save_pre > 0) {
		bool (*preHookFunc) (const struct s_elemental **ele);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_save_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_elemental_save_pre[hIndex].func;
			retVal___ = preHookFunc(&ele);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.elemental_save(ele);
	}
	if (HPMHooks.count.HP_mapif_elemental_save_post > 0) {
		bool (*postHookFunc) (bool retVal___, const struct s_elemental *ele);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_save_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_elemental_save_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, ele);
		}
	}
	return retVal___;
}
bool HP_mapif_elemental_load(int ele_id, int char_id, struct s_elemental *ele) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_mapif_elemental_load_pre > 0) {
		bool (*preHookFunc) (int *ele_id, int *char_id, struct s_elemental **ele);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_load_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_elemental_load_pre[hIndex].func;
			retVal___ = preHookFunc(&ele_id, &char_id, &ele);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.elemental_load(ele_id, char_id, ele);
	}
	if (HPMHooks.count.HP_mapif_elemental_load_post > 0) {
		bool (*postHookFunc) (bool retVal___, int ele_id, int char_id, struct s_elemental *ele);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_load_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_elemental_load_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, ele_id, char_id, ele);
		}
	}
	return retVal___;
}
bool HP_mapif_elemental_delete(int ele_id) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_mapif_elemental_delete_pre > 0) {
		bool (*preHookFunc) (int *ele_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_delete_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_elemental_delete_pre[hIndex].func;
			retVal___ = preHookFunc(&ele_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.elemental_delete(ele_id);
	}
	if (HPMHooks.count.HP_mapif_elemental_delete_post > 0) {
		bool (*postHookFunc) (bool retVal___, int ele_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_delete_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_elemental_delete_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, ele_id);
		}
	}
	return retVal___;
}
void HP_mapif_elemental_send(int fd, struct s_elemental *ele, unsigned char flag) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_elemental_send_pre > 0) {
		void (*preHookFunc) (int *fd, struct s_elemental **ele, unsigned char *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_send_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_elemental_send_pre[hIndex].func;
			preHookFunc(&fd, &ele, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.elemental_send(fd, ele, flag);
	}
	if (HPMHooks.count.HP_mapif_elemental_send_post > 0) {
		void (*postHookFunc) (int fd, struct s_elemental *ele, unsigned char flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_send_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_elemental_send_post[hIndex].func;
			postHookFunc(fd, ele, flag);
		}
	}
	return;
}
void HP_mapif_parse_elemental_create(int fd, const struct s_elemental *ele) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_elemental_create_pre > 0) {
		void (*preHookFunc) (int *fd, const struct s_elemental **ele);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_elemental_create_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_elemental_create_pre[hIndex].func;
			preHookFunc(&fd, &ele);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_elemental_create(fd, ele);
	}
	if (HPMHooks.count.HP_mapif_parse_elemental_create_post > 0) {
		void (*postHookFunc) (int fd, const struct s_elemental *ele);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_elemental_create_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_elemental_create_post[hIndex].func;
			postHookFunc(fd, ele);
		}
	}
	return;
}
void HP_mapif_parse_elemental_load(int fd, int ele_id, int char_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_elemental_load_pre > 0) {
		void (*preHookFunc) (int *fd, int *ele_id, int *char_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_elemental_load_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_elemental_load_pre[hIndex].func;
			preHookFunc(&fd, &ele_id, &char_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_elemental_load(fd, ele_id, char_id);
	}
	if (HPMHooks.count.HP_mapif_parse_elemental_load_post > 0) {
		void (*postHookFunc) (int fd, int ele_id, int char_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_elemental_load_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_elemental_load_post[hIndex].func;
			postHookFunc(fd, ele_id, char_id);
		}
	}
	return;
}
void HP_mapif_elemental_deleted(int fd, unsigned char flag) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_elemental_deleted_pre > 0) {
		void (*preHookFunc) (int *fd, unsigned char *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_deleted_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_elemental_deleted_pre[hIndex].func;
			preHookFunc(&fd, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.elemental_deleted(fd, flag);
	}
	if (HPMHooks.count.HP_mapif_elemental_deleted_post > 0) {
		void (*postHookFunc) (int fd, unsigned char flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_deleted_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_elemental_deleted_post[hIndex].func;
			postHookFunc(fd, flag);
		}
	}
	return;
}
void HP_mapif_parse_elemental_delete(int fd, int ele_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_elemental_delete_pre > 0) {
		void (*preHookFunc) (int *fd, int *ele_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_elemental_delete_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_elemental_delete_pre[hIndex].func;
			preHookFunc(&fd, &ele_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_elemental_delete(fd, ele_id);
	}
	if (HPMHooks.count.HP_mapif_parse_elemental_delete_post > 0) {
		void (*postHookFunc) (int fd, int ele_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_elemental_delete_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_elemental_delete_post[hIndex].func;
			postHookFunc(fd, ele_id);
		}
	}
	return;
}
void HP_mapif_elemental_saved(int fd, unsigned char flag) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_elemental_saved_pre > 0) {
		void (*preHookFunc) (int *fd, unsigned char *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_saved_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_elemental_saved_pre[hIndex].func;
			preHookFunc(&fd, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.elemental_saved(fd, flag);
	}
	if (HPMHooks.count.HP_mapif_elemental_saved_post > 0) {
		void (*postHookFunc) (int fd, unsigned char flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_saved_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_elemental_saved_post[hIndex].func;
			postHookFunc(fd, flag);
		}
	}
	return;
}
void HP_mapif_parse_elemental_save(int fd, const struct s_elemental *ele) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_elemental_save_pre > 0) {
		void (*preHookFunc) (int *fd, const struct s_elemental **ele);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_elemental_save_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_elemental_save_pre[hIndex].func;
			preHookFunc(&fd, &ele);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_elemental_save(fd, ele);
	}
	if (HPMHooks.count.HP_mapif_parse_elemental_save_post > 0) {
		void (*postHookFunc) (int fd, const struct s_elemental *ele);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_elemental_save_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_elemental_save_post[hIndex].func;
			postHookFunc(fd, ele);
		}
	}
	return;
}
int HP_mapif_guild_created(int fd, int account_id, struct guild *g) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_guild_created_pre > 0) {
		int (*preHookFunc) (int *fd, int *account_id, struct guild **g);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_created_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_created_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, &g);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_created(fd, account_id, g);
	}
	if (HPMHooks.count.HP_mapif_guild_created_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int account_id, struct guild *g);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_created_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_created_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, account_id, g);
		}
	}
	return retVal___;
}
int HP_mapif_guild_noinfo(int fd, int guild_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_guild_noinfo_pre > 0) {
		int (*preHookFunc) (int *fd, int *guild_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_noinfo_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_noinfo_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_noinfo(fd, guild_id);
	}
	if (HPMHooks.count.HP_mapif_guild_noinfo_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int guild_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_noinfo_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_noinfo_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, guild_id);
		}
	}
	return retVal___;
}
int HP_mapif_guild_info(int fd, struct guild *g) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_guild_info_pre > 0) {
		int (*preHookFunc) (int *fd, struct guild **g);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_info_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_info_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &g);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_info(fd, g);
	}
	if (HPMHooks.count.HP_mapif_guild_info_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, struct guild *g);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_info_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_info_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, g);
		}
	}
	return retVal___;
}
int HP_mapif_guild_memberadded(int fd, int guild_id, int account_id, int char_id, int flag) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_guild_memberadded_pre > 0) {
		int (*preHookFunc) (int *fd, int *guild_id, int *account_id, int *char_id, int *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_memberadded_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_memberadded_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id, &account_id, &char_id, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_memberadded(fd, guild_id, account_id, char_id, flag);
	}
	if (HPMHooks.count.HP_mapif_guild_memberadded_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int guild_id, int account_id, int char_id, int flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_memberadded_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_memberadded_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, guild_id, account_id, char_id, flag);
		}
	}
	return retVal___;
}
int HP_mapif_guild_withdraw(int guild_id, int account_id, int char_id, int flag, const char *name, const char *mes) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_guild_withdraw_pre > 0) {
		int (*preHookFunc) (int *guild_id, int *account_id, int *char_id, int *flag, const char **name, const char **mes);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_withdraw_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_withdraw_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id, &account_id, &char_id, &flag, &name, &mes);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_withdraw(guild_id, account_id, char_id, flag, name, mes);
	}
	if (HPMHooks.count.HP_mapif_guild_withdraw_post > 0) {
		int (*postHookFunc) (int retVal___, int guild_id, int account_id, int char_id, int flag, const char *name, const char *mes);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_withdraw_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_withdraw_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, guild_id, account_id, char_id, flag, name, mes);
		}
	}
	return retVal___;
}
int HP_mapif_guild_memberinfoshort(struct guild *g, int idx) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_guild_memberinfoshort_pre > 0) {
		int (*preHookFunc) (struct guild **g, int *idx);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_memberinfoshort_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_memberinfoshort_pre[hIndex].func;
			retVal___ = preHookFunc(&g, &idx);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_memberinfoshort(g, idx);
	}
	if (HPMHooks.count.HP_mapif_guild_memberinfoshort_post > 0) {
		int (*postHookFunc) (int retVal___, struct guild *g, int idx);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_memberinfoshort_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_memberinfoshort_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, g, idx);
		}
	}
	return retVal___;
}
int HP_mapif_guild_broken(int guild_id, int flag) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_guild_broken_pre > 0) {
		int (*preHookFunc) (int *guild_id, int *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_broken_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_broken_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_broken(guild_id, flag);
	}
	if (HPMHooks.count.HP_mapif_guild_broken_post > 0) {
		int (*postHookFunc) (int retVal___, int guild_id, int flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_broken_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_broken_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, guild_id, flag);
		}
	}
	return retVal___;
}
int HP_mapif_guild_message(int guild_id, int account_id, const char *mes, int len, int sfd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_guild_message_pre > 0) {
		int (*preHookFunc) (int *guild_id, int *account_id, const char **mes, int *len, int *sfd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_message_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_message_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id, &account_id, &mes, &len, &sfd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_message(guild_id, account_id, mes, len, sfd);
	}
	if (HPMHooks.count.HP_mapif_guild_message_post > 0) {
		int (*postHookFunc) (int retVal___, int guild_id, int account_id, const char *mes, int len, int sfd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_message_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_message_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, guild_id, account_id, mes, len, sfd);
		}
	}
	return retVal___;
}
int HP_mapif_guild_basicinfochanged(int guild_id, int type, const void *data, int len) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_guild_basicinfochanged_pre > 0) {
		int (*preHookFunc) (int *guild_id, int *type, const void **data, int *len);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_basicinfochanged_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_basicinfochanged_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id, &type, &data, &len);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_basicinfochanged(guild_id, type, data, len);
	}
	if (HPMHooks.count.HP_mapif_guild_basicinfochanged_post > 0) {
		int (*postHookFunc) (int retVal___, int guild_id, int type, const void *data, int len);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_basicinfochanged_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_basicinfochanged_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, guild_id, type, data, len);
		}
	}
	return retVal___;
}
int HP_mapif_guild_memberinfochanged(int guild_id, int account_id, int char_id, int type, const void *data, int len) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_guild_memberinfochanged_pre > 0) {
		int (*preHookFunc) (int *guild_id, int *account_id, int *char_id, int *type, const void **data, int *len);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_memberinfochanged_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_memberinfochanged_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id, &account_id, &char_id, &type, &data, &len);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_memberinfochanged(guild_id, account_id, char_id, type, data, len);
	}
	if (HPMHooks.count.HP_mapif_guild_memberinfochanged_post > 0) {
		int (*postHookFunc) (int retVal___, int guild_id, int account_id, int char_id, int type, const void *data, int len);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_memberinfochanged_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_memberinfochanged_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, guild_id, account_id, char_id, type, data, len);
		}
	}
	return retVal___;
}
int HP_mapif_guild_skillupack(int guild_id, uint16 skill_id, int account_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_guild_skillupack_pre > 0) {
		int (*preHookFunc) (int *guild_id, uint16 *skill_id, int *account_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_skillupack_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_skillupack_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id, &skill_id, &account_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_skillupack(guild_id, skill_id, account_id);
	}
	if (HPMHooks.count.HP_mapif_guild_skillupack_post > 0) {
		int (*postHookFunc) (int retVal___, int guild_id, uint16 skill_id, int account_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_skillupack_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_skillupack_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, guild_id, skill_id, account_id);
		}
	}
	return retVal___;
}
int HP_mapif_guild_alliance(int guild_id1, int guild_id2, int account_id1, int account_id2, int flag, const char *name1, const char *name2) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_guild_alliance_pre > 0) {
		int (*preHookFunc) (int *guild_id1, int *guild_id2, int *account_id1, int *account_id2, int *flag, const char **name1, const char **name2);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_alliance_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_alliance_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id1, &guild_id2, &account_id1, &account_id2, &flag, &name1, &name2);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_alliance(guild_id1, guild_id2, account_id1, account_id2, flag, name1, name2);
	}
	if (HPMHooks.count.HP_mapif_guild_alliance_post > 0) {
		int (*postHookFunc) (int retVal___, int guild_id1, int guild_id2, int account_id1, int account_id2, int flag, const char *name1, const char *name2);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_alliance_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_alliance_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, guild_id1, guild_id2, account_id1, account_id2, flag, name1, name2);
		}
	}
	return retVal___;
}
int HP_mapif_guild_position(struct guild *g, int idx) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_guild_position_pre > 0) {
		int (*preHookFunc) (struct guild **g, int *idx);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_position_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_position_pre[hIndex].func;
			retVal___ = preHookFunc(&g, &idx);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_position(g, idx);
	}
	if (HPMHooks.count.HP_mapif_guild_position_post > 0) {
		int (*postHookFunc) (int retVal___, struct guild *g, int idx);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_position_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_position_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, g, idx);
		}
	}
	return retVal___;
}
int HP_mapif_guild_notice(struct guild *g) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_guild_notice_pre > 0) {
		int (*preHookFunc) (struct guild **g);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_notice_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_notice_pre[hIndex].func;
			retVal___ = preHookFunc(&g);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_notice(g);
	}
	if (HPMHooks.count.HP_mapif_guild_notice_post > 0) {
		int (*postHookFunc) (int retVal___, struct guild *g);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_notice_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_notice_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, g);
		}
	}
	return retVal___;
}
int HP_mapif_guild_emblem(struct guild *g) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_guild_emblem_pre > 0) {
		int (*preHookFunc) (struct guild **g);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_emblem_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_emblem_pre[hIndex].func;
			retVal___ = preHookFunc(&g);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_emblem(g);
	}
	if (HPMHooks.count.HP_mapif_guild_emblem_post > 0) {
		int (*postHookFunc) (int retVal___, struct guild *g);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_emblem_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_emblem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, g);
		}
	}
	return retVal___;
}
int HP_mapif_guild_master_changed(struct guild *g, int aid, int cid) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_guild_master_changed_pre > 0) {
		int (*preHookFunc) (struct guild **g, int *aid, int *cid);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_master_changed_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_master_changed_pre[hIndex].func;
			retVal___ = preHookFunc(&g, &aid, &cid);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_master_changed(g, aid, cid);
	}
	if (HPMHooks.count.HP_mapif_guild_master_changed_post > 0) {
		int (*postHookFunc) (int retVal___, struct guild *g, int aid, int cid);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_master_changed_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_master_changed_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, g, aid, cid);
		}
	}
	return retVal___;
}
int HP_mapif_guild_castle_dataload(int fd, int sz, const int *castle_ids) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_guild_castle_dataload_pre > 0) {
		int (*preHookFunc) (int *fd, int *sz, const int **castle_ids);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_castle_dataload_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_castle_dataload_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &sz, &castle_ids);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_castle_dataload(fd, sz, castle_ids);
	}
	if (HPMHooks.count.HP_mapif_guild_castle_dataload_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int sz, const int *castle_ids);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_castle_dataload_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_castle_dataload_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, sz, castle_ids);
		}
	}
	return retVal___;
}
int HP_mapif_parse_CreateGuild(int fd, int account_id, const char *name, const struct guild_member *master) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_CreateGuild_pre > 0) {
		int (*preHookFunc) (int *fd, int *account_id, const char **name, const struct guild_member **master);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_CreateGuild_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_CreateGuild_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, &name, &master);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_CreateGuild(fd, account_id, name, master);
	}
	if (HPMHooks.count.HP_mapif_parse_CreateGuild_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int account_id, const char *name, const struct guild_member *master);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_CreateGuild_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_CreateGuild_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, account_id, name, master);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildInfo(int fd, int guild_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_GuildInfo_pre > 0) {
		int (*preHookFunc) (int *fd, int *guild_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildInfo_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildInfo_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildInfo(fd, guild_id);
	}
	if (HPMHooks.count.HP_mapif_parse_GuildInfo_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int guild_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildInfo_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildInfo_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, guild_id);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildAddMember(int fd, int guild_id, const struct guild_member *m) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_GuildAddMember_pre > 0) {
		int (*preHookFunc) (int *fd, int *guild_id, const struct guild_member **m);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildAddMember_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildAddMember_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id, &m);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildAddMember(fd, guild_id, m);
	}
	if (HPMHooks.count.HP_mapif_parse_GuildAddMember_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int guild_id, const struct guild_member *m);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildAddMember_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildAddMember_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, guild_id, m);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildLeave(int fd, int guild_id, int account_id, int char_id, int flag, const char *mes) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_GuildLeave_pre > 0) {
		int (*preHookFunc) (int *fd, int *guild_id, int *account_id, int *char_id, int *flag, const char **mes);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildLeave_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildLeave_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id, &account_id, &char_id, &flag, &mes);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildLeave(fd, guild_id, account_id, char_id, flag, mes);
	}
	if (HPMHooks.count.HP_mapif_parse_GuildLeave_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int guild_id, int account_id, int char_id, int flag, const char *mes);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildLeave_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildLeave_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, guild_id, account_id, char_id, flag, mes);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildChangeMemberInfoShort(int fd, int guild_id, int account_id, int char_id, int online, int lv, int16 class) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_GuildChangeMemberInfoShort_pre > 0) {
		int (*preHookFunc) (int *fd, int *guild_id, int *account_id, int *char_id, int *online, int *lv, int16 *class);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildChangeMemberInfoShort_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildChangeMemberInfoShort_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id, &account_id, &char_id, &online, &lv, &class);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildChangeMemberInfoShort(fd, guild_id, account_id, char_id, online, lv, class);
	}
	if (HPMHooks.count.HP_mapif_parse_GuildChangeMemberInfoShort_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int guild_id, int account_id, int char_id, int online, int lv, int16 class);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildChangeMemberInfoShort_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildChangeMemberInfoShort_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, guild_id, account_id, char_id, online, lv, class);
		}
	}
	return retVal___;
}
int HP_mapif_parse_BreakGuild(int fd, int guild_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_BreakGuild_pre > 0) {
		int (*preHookFunc) (int *fd, int *guild_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_BreakGuild_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_BreakGuild_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_BreakGuild(fd, guild_id);
	}
	if (HPMHooks.count.HP_mapif_parse_BreakGuild_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int guild_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_BreakGuild_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_BreakGuild_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, guild_id);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildMessage(int fd, int guild_id, int account_id, const char *mes, int len) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_GuildMessage_pre > 0) {
		int (*preHookFunc) (int *fd, int *guild_id, int *account_id, const char **mes, int *len);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildMessage_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildMessage_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id, &account_id, &mes, &len);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildMessage(fd, guild_id, account_id, mes, len);
	}
	if (HPMHooks.count.HP_mapif_parse_GuildMessage_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int guild_id, int account_id, const char *mes, int len);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildMessage_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildMessage_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, guild_id, account_id, mes, len);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildBasicInfoChange(int fd, int guild_id, int type, const void *data, int len) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_GuildBasicInfoChange_pre > 0) {
		int (*preHookFunc) (int *fd, int *guild_id, int *type, const void **data, int *len);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildBasicInfoChange_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildBasicInfoChange_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id, &type, &data, &len);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildBasicInfoChange(fd, guild_id, type, data, len);
	}
	if (HPMHooks.count.HP_mapif_parse_GuildBasicInfoChange_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int guild_id, int type, const void *data, int len);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildBasicInfoChange_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildBasicInfoChange_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, guild_id, type, data, len);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildMemberInfoChange(int fd, int guild_id, int account_id, int char_id, int type, const char *data, int len) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_GuildMemberInfoChange_pre > 0) {
		int (*preHookFunc) (int *fd, int *guild_id, int *account_id, int *char_id, int *type, const char **data, int *len);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildMemberInfoChange_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildMemberInfoChange_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id, &account_id, &char_id, &type, &data, &len);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildMemberInfoChange(fd, guild_id, account_id, char_id, type, data, len);
	}
	if (HPMHooks.count.HP_mapif_parse_GuildMemberInfoChange_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int guild_id, int account_id, int char_id, int type, const char *data, int len);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildMemberInfoChange_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildMemberInfoChange_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, guild_id, account_id, char_id, type, data, len);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildPosition(int fd, int guild_id, int idx, const struct guild_position *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_GuildPosition_pre > 0) {
		int (*preHookFunc) (int *fd, int *guild_id, int *idx, const struct guild_position **p);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildPosition_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildPosition_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id, &idx, &p);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildPosition(fd, guild_id, idx, p);
	}
	if (HPMHooks.count.HP_mapif_parse_GuildPosition_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int guild_id, int idx, const struct guild_position *p);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildPosition_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildPosition_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, guild_id, idx, p);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildSkillUp(int fd, int guild_id, uint16 skill_id, int account_id, int max) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_GuildSkillUp_pre > 0) {
		int (*preHookFunc) (int *fd, int *guild_id, uint16 *skill_id, int *account_id, int *max);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildSkillUp_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildSkillUp_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id, &skill_id, &account_id, &max);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildSkillUp(fd, guild_id, skill_id, account_id, max);
	}
	if (HPMHooks.count.HP_mapif_parse_GuildSkillUp_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int guild_id, uint16 skill_id, int account_id, int max);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildSkillUp_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildSkillUp_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, guild_id, skill_id, account_id, max);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildDeleteAlliance(struct guild *g, int guild_id, int account_id1, int account_id2, int flag) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_GuildDeleteAlliance_pre > 0) {
		int (*preHookFunc) (struct guild **g, int *guild_id, int *account_id1, int *account_id2, int *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildDeleteAlliance_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildDeleteAlliance_pre[hIndex].func;
			retVal___ = preHookFunc(&g, &guild_id, &account_id1, &account_id2, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildDeleteAlliance(g, guild_id, account_id1, account_id2, flag);
	}
	if (HPMHooks.count.HP_mapif_parse_GuildDeleteAlliance_post > 0) {
		int (*postHookFunc) (int retVal___, struct guild *g, int guild_id, int account_id1, int account_id2, int flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildDeleteAlliance_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildDeleteAlliance_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, g, guild_id, account_id1, account_id2, flag);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildAlliance(int fd, int guild_id1, int guild_id2, int account_id1, int account_id2, int flag) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_GuildAlliance_pre > 0) {
		int (*preHookFunc) (int *fd, int *guild_id1, int *guild_id2, int *account_id1, int *account_id2, int *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildAlliance_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildAlliance_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id1, &guild_id2, &account_id1, &account_id2, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildAlliance(fd, guild_id1, guild_id2, account_id1, account_id2, flag);
	}
	if (HPMHooks.count.HP_mapif_parse_GuildAlliance_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int guild_id1, int guild_id2, int account_id1, int account_id2, int flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildAlliance_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildAlliance_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, guild_id1, guild_id2, account_id1, account_id2, flag);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildNotice(int fd, int guild_id, const char *mes1, const char *mes2) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_GuildNotice_pre > 0) {
		int (*preHookFunc) (int *fd, int *guild_id, const char **mes1, const char **mes2);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildNotice_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildNotice_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id, &mes1, &mes2);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildNotice(fd, guild_id, mes1, mes2);
	}
	if (HPMHooks.count.HP_mapif_parse_GuildNotice_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int guild_id, const char *mes1, const char *mes2);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildNotice_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildNotice_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, guild_id, mes1, mes2);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildEmblem(int fd, int len, int guild_id, int dummy, const char *data) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_GuildEmblem_pre > 0) {
		int (*preHookFunc) (int *fd, int *len, int *guild_id, int *dummy, const char **data);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildEmblem_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildEmblem_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &len, &guild_id, &dummy, &data);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildEmblem(fd, len, guild_id, dummy, data);
	}
	if (HPMHooks.count.HP_mapif_parse_GuildEmblem_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int len, int guild_id, int dummy, const char *data);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildEmblem_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildEmblem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, len, guild_id, dummy, data);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildCastleDataLoad(int fd, int len, const int *castle_ids) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_GuildCastleDataLoad_pre > 0) {
		int (*preHookFunc) (int *fd, int *len, const int **castle_ids);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildCastleDataLoad_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildCastleDataLoad_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &len, &castle_ids);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildCastleDataLoad(fd, len, castle_ids);
	}
	if (HPMHooks.count.HP_mapif_parse_GuildCastleDataLoad_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int len, const int *castle_ids);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildCastleDataLoad_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildCastleDataLoad_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, len, castle_ids);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildCastleDataSave(int fd, int castle_id, int index, int value) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_GuildCastleDataSave_pre > 0) {
		int (*preHookFunc) (int *fd, int *castle_id, int *index, int *value);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildCastleDataSave_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildCastleDataSave_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &castle_id, &index, &value);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildCastleDataSave(fd, castle_id, index, value);
	}
	if (HPMHooks.count.HP_mapif_parse_GuildCastleDataSave_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int castle_id, int index, int value);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildCastleDataSave_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildCastleDataSave_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, castle_id, index, value);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildMasterChange(int fd, int guild_id, const char *name, int len) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_GuildMasterChange_pre > 0) {
		int (*preHookFunc) (int *fd, int *guild_id, const char **name, int *len);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildMasterChange_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildMasterChange_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id, &name, &len);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildMasterChange(fd, guild_id, name, len);
	}
	if (HPMHooks.count.HP_mapif_parse_GuildMasterChange_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int guild_id, const char *name, int len);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildMasterChange_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildMasterChange_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, guild_id, name, len);
		}
	}
	return retVal___;
}
void HP_mapif_homunculus_created(int fd, int account_id, const struct s_homunculus *sh, unsigned char flag) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_homunculus_created_pre > 0) {
		void (*preHookFunc) (int *fd, int *account_id, const struct s_homunculus **sh, unsigned char *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_created_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_homunculus_created_pre[hIndex].func;
			preHookFunc(&fd, &account_id, &sh, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.homunculus_created(fd, account_id, sh, flag);
	}
	if (HPMHooks.count.HP_mapif_homunculus_created_post > 0) {
		void (*postHookFunc) (int fd, int account_id, const struct s_homunculus *sh, unsigned char flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_created_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_homunculus_created_post[hIndex].func;
			postHookFunc(fd, account_id, sh, flag);
		}
	}
	return;
}
void HP_mapif_homunculus_deleted(int fd, int flag) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_homunculus_deleted_pre > 0) {
		void (*preHookFunc) (int *fd, int *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_deleted_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_homunculus_deleted_pre[hIndex].func;
			preHookFunc(&fd, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.homunculus_deleted(fd, flag);
	}
	if (HPMHooks.count.HP_mapif_homunculus_deleted_post > 0) {
		void (*postHookFunc) (int fd, int flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_deleted_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_homunculus_deleted_post[hIndex].func;
			postHookFunc(fd, flag);
		}
	}
	return;
}
void HP_mapif_homunculus_loaded(int fd, int account_id, struct s_homunculus *hd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_homunculus_loaded_pre > 0) {
		void (*preHookFunc) (int *fd, int *account_id, struct s_homunculus **hd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_loaded_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_homunculus_loaded_pre[hIndex].func;
			preHookFunc(&fd, &account_id, &hd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.homunculus_loaded(fd, account_id, hd);
	}
	if (HPMHooks.count.HP_mapif_homunculus_loaded_post > 0) {
		void (*postHookFunc) (int fd, int account_id, struct s_homunculus *hd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_loaded_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_homunculus_loaded_post[hIndex].func;
			postHookFunc(fd, account_id, hd);
		}
	}
	return;
}
void HP_mapif_homunculus_saved(int fd, int account_id, bool flag) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_homunculus_saved_pre > 0) {
		void (*preHookFunc) (int *fd, int *account_id, bool *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_saved_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_homunculus_saved_pre[hIndex].func;
			preHookFunc(&fd, &account_id, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.homunculus_saved(fd, account_id, flag);
	}
	if (HPMHooks.count.HP_mapif_homunculus_saved_post > 0) {
		void (*postHookFunc) (int fd, int account_id, bool flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_saved_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_homunculus_saved_post[hIndex].func;
			postHookFunc(fd, account_id, flag);
		}
	}
	return;
}
void HP_mapif_homunculus_renamed(int fd, int account_id, int char_id, unsigned char flag, const char *name) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_homunculus_renamed_pre > 0) {
		void (*preHookFunc) (int *fd, int *account_id, int *char_id, unsigned char *flag, const char **name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_renamed_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_homunculus_renamed_pre[hIndex].func;
			preHookFunc(&fd, &account_id, &char_id, &flag, &name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.homunculus_renamed(fd, account_id, char_id, flag, name);
	}
	if (HPMHooks.count.HP_mapif_homunculus_renamed_post > 0) {
		void (*postHookFunc) (int fd, int account_id, int char_id, unsigned char flag, const char *name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_renamed_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_homunculus_renamed_post[hIndex].func;
			postHookFunc(fd, account_id, char_id, flag, name);
		}
	}
	return;
}
bool HP_mapif_homunculus_create(struct s_homunculus *hd) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_mapif_homunculus_create_pre > 0) {
		bool (*preHookFunc) (struct s_homunculus **hd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_create_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_homunculus_create_pre[hIndex].func;
			retVal___ = preHookFunc(&hd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.homunculus_create(hd);
	}
	if (HPMHooks.count.HP_mapif_homunculus_create_post > 0) {
		bool (*postHookFunc) (bool retVal___, struct s_homunculus *hd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_create_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_homunculus_create_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, hd);
		}
	}
	return retVal___;
}
bool HP_mapif_homunculus_save(const struct s_homunculus *hd) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_mapif_homunculus_save_pre > 0) {
		bool (*preHookFunc) (const struct s_homunculus **hd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_save_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_homunculus_save_pre[hIndex].func;
			retVal___ = preHookFunc(&hd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.homunculus_save(hd);
	}
	if (HPMHooks.count.HP_mapif_homunculus_save_post > 0) {
		bool (*postHookFunc) (bool retVal___, const struct s_homunculus *hd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_save_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_homunculus_save_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, hd);
		}
	}
	return retVal___;
}
bool HP_mapif_homunculus_load(int homun_id, struct s_homunculus *hd) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_mapif_homunculus_load_pre > 0) {
		bool (*preHookFunc) (int *homun_id, struct s_homunculus **hd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_load_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_homunculus_load_pre[hIndex].func;
			retVal___ = preHookFunc(&homun_id, &hd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.homunculus_load(homun_id, hd);
	}
	if (HPMHooks.count.HP_mapif_homunculus_load_post > 0) {
		bool (*postHookFunc) (bool retVal___, int homun_id, struct s_homunculus *hd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_load_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_homunculus_load_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, homun_id, hd);
		}
	}
	return retVal___;
}
bool HP_mapif_homunculus_delete(int homun_id) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_mapif_homunculus_delete_pre > 0) {
		bool (*preHookFunc) (int *homun_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_delete_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_homunculus_delete_pre[hIndex].func;
			retVal___ = preHookFunc(&homun_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.homunculus_delete(homun_id);
	}
	if (HPMHooks.count.HP_mapif_homunculus_delete_post > 0) {
		bool (*postHookFunc) (bool retVal___, int homun_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_delete_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_homunculus_delete_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, homun_id);
		}
	}
	return retVal___;
}
bool HP_mapif_homunculus_rename(const char *name) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_mapif_homunculus_rename_pre > 0) {
		bool (*preHookFunc) (const char **name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_rename_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_homunculus_rename_pre[hIndex].func;
			retVal___ = preHookFunc(&name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.homunculus_rename(name);
	}
	if (HPMHooks.count.HP_mapif_homunculus_rename_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_rename_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_homunculus_rename_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, name);
		}
	}
	return retVal___;
}
void HP_mapif_parse_homunculus_create(int fd, int len, int account_id, const struct s_homunculus *phd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_homunculus_create_pre > 0) {
		void (*preHookFunc) (int *fd, int *len, int *account_id, const struct s_homunculus **phd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_homunculus_create_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_homunculus_create_pre[hIndex].func;
			preHookFunc(&fd, &len, &account_id, &phd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_homunculus_create(fd, len, account_id, phd);
	}
	if (HPMHooks.count.HP_mapif_parse_homunculus_create_post > 0) {
		void (*postHookFunc) (int fd, int len, int account_id, const struct s_homunculus *phd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_homunculus_create_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_homunculus_create_post[hIndex].func;
			postHookFunc(fd, len, account_id, phd);
		}
	}
	return;
}
void HP_mapif_parse_homunculus_delete(int fd, int homun_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_homunculus_delete_pre > 0) {
		void (*preHookFunc) (int *fd, int *homun_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_homunculus_delete_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_homunculus_delete_pre[hIndex].func;
			preHookFunc(&fd, &homun_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_homunculus_delete(fd, homun_id);
	}
	if (HPMHooks.count.HP_mapif_parse_homunculus_delete_post > 0) {
		void (*postHookFunc) (int fd, int homun_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_homunculus_delete_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_homunculus_delete_post[hIndex].func;
			postHookFunc(fd, homun_id);
		}
	}
	return;
}
void HP_mapif_parse_homunculus_load(int fd, int account_id, int homun_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_homunculus_load_pre > 0) {
		void (*preHookFunc) (int *fd, int *account_id, int *homun_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_homunculus_load_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_homunculus_load_pre[hIndex].func;
			preHookFunc(&fd, &account_id, &homun_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_homunculus_load(fd, account_id, homun_id);
	}
	if (HPMHooks.count.HP_mapif_parse_homunculus_load_post > 0) {
		void (*postHookFunc) (int fd, int account_id, int homun_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_homunculus_load_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_homunculus_load_post[hIndex].func;
			postHookFunc(fd, account_id, homun_id);
		}
	}
	return;
}
void HP_mapif_parse_homunculus_save(int fd, int len, int account_id, const struct s_homunculus *phd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_homunculus_save_pre > 0) {
		void (*preHookFunc) (int *fd, int *len, int *account_id, const struct s_homunculus **phd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_homunculus_save_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_homunculus_save_pre[hIndex].func;
			preHookFunc(&fd, &len, &account_id, &phd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_homunculus_save(fd, len, account_id, phd);
	}
	if (HPMHooks.count.HP_mapif_parse_homunculus_save_post > 0) {
		void (*postHookFunc) (int fd, int len, int account_id, const struct s_homunculus *phd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_homunculus_save_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_homunculus_save_post[hIndex].func;
			postHookFunc(fd, len, account_id, phd);
		}
	}
	return;
}
void HP_mapif_parse_homunculus_rename(int fd, int account_id, int char_id, const char *name) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_homunculus_rename_pre > 0) {
		void (*preHookFunc) (int *fd, int *account_id, int *char_id, const char **name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_homunculus_rename_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_homunculus_rename_pre[hIndex].func;
			preHookFunc(&fd, &account_id, &char_id, &name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_homunculus_rename(fd, account_id, char_id, name);
	}
	if (HPMHooks.count.HP_mapif_parse_homunculus_rename_post > 0) {
		void (*postHookFunc) (int fd, int account_id, int char_id, const char *name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_homunculus_rename_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_homunculus_rename_post[hIndex].func;
			postHookFunc(fd, account_id, char_id, name);
		}
	}
	return;
}
void HP_mapif_mail_sendinbox(int fd, int char_id, unsigned char flag, struct mail_data *md) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_mail_sendinbox_pre > 0) {
		void (*preHookFunc) (int *fd, int *char_id, unsigned char *flag, struct mail_data **md);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_sendinbox_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_mail_sendinbox_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &flag, &md);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.mail_sendinbox(fd, char_id, flag, md);
	}
	if (HPMHooks.count.HP_mapif_mail_sendinbox_post > 0) {
		void (*postHookFunc) (int fd, int char_id, unsigned char flag, struct mail_data *md);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_sendinbox_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_mail_sendinbox_post[hIndex].func;
			postHookFunc(fd, char_id, flag, md);
		}
	}
	return;
}
void HP_mapif_parse_mail_requestinbox(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_mail_requestinbox_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mail_requestinbox_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_mail_requestinbox_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_mail_requestinbox(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_mail_requestinbox_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mail_requestinbox_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_mail_requestinbox_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_mapif_parse_mail_read(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_mail_read_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mail_read_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_mail_read_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_mail_read(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_mail_read_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mail_read_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_mail_read_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_mapif_mail_sendattach(int fd, int char_id, struct mail_message *msg) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_mail_sendattach_pre > 0) {
		void (*preHookFunc) (int *fd, int *char_id, struct mail_message **msg);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_sendattach_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_mail_sendattach_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &msg);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.mail_sendattach(fd, char_id, msg);
	}
	if (HPMHooks.count.HP_mapif_mail_sendattach_post > 0) {
		void (*postHookFunc) (int fd, int char_id, struct mail_message *msg);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_sendattach_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_mail_sendattach_post[hIndex].func;
			postHookFunc(fd, char_id, msg);
		}
	}
	return;
}
void HP_mapif_mail_getattach(int fd, int char_id, int mail_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_mail_getattach_pre > 0) {
		void (*preHookFunc) (int *fd, int *char_id, int *mail_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_getattach_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_mail_getattach_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &mail_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.mail_getattach(fd, char_id, mail_id);
	}
	if (HPMHooks.count.HP_mapif_mail_getattach_post > 0) {
		void (*postHookFunc) (int fd, int char_id, int mail_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_getattach_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_mail_getattach_post[hIndex].func;
			postHookFunc(fd, char_id, mail_id);
		}
	}
	return;
}
void HP_mapif_parse_mail_getattach(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_mail_getattach_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mail_getattach_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_mail_getattach_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_mail_getattach(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_mail_getattach_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mail_getattach_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_mail_getattach_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_mapif_mail_delete(int fd, int char_id, int mail_id, bool failed) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_mail_delete_pre > 0) {
		void (*preHookFunc) (int *fd, int *char_id, int *mail_id, bool *failed);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_delete_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_mail_delete_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &mail_id, &failed);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.mail_delete(fd, char_id, mail_id, failed);
	}
	if (HPMHooks.count.HP_mapif_mail_delete_post > 0) {
		void (*postHookFunc) (int fd, int char_id, int mail_id, bool failed);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_delete_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_mail_delete_post[hIndex].func;
			postHookFunc(fd, char_id, mail_id, failed);
		}
	}
	return;
}
void HP_mapif_parse_mail_delete(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_mail_delete_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mail_delete_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_mail_delete_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_mail_delete(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_mail_delete_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mail_delete_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_mail_delete_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_mapif_mail_new(struct mail_message *msg) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_mail_new_pre > 0) {
		void (*preHookFunc) (struct mail_message **msg);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_new_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_mail_new_pre[hIndex].func;
			preHookFunc(&msg);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.mail_new(msg);
	}
	if (HPMHooks.count.HP_mapif_mail_new_post > 0) {
		void (*postHookFunc) (struct mail_message *msg);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_new_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_mail_new_post[hIndex].func;
			postHookFunc(msg);
		}
	}
	return;
}
void HP_mapif_mail_return(int fd, int char_id, int mail_id, int new_mail) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_mail_return_pre > 0) {
		void (*preHookFunc) (int *fd, int *char_id, int *mail_id, int *new_mail);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_return_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_mail_return_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &mail_id, &new_mail);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.mail_return(fd, char_id, mail_id, new_mail);
	}
	if (HPMHooks.count.HP_mapif_mail_return_post > 0) {
		void (*postHookFunc) (int fd, int char_id, int mail_id, int new_mail);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_return_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_mail_return_post[hIndex].func;
			postHookFunc(fd, char_id, mail_id, new_mail);
		}
	}
	return;
}
void HP_mapif_parse_mail_return(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_mail_return_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mail_return_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_mail_return_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_mail_return(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_mail_return_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mail_return_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_mail_return_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_mapif_mail_send(int fd, struct mail_message *msg) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_mail_send_pre > 0) {
		void (*preHookFunc) (int *fd, struct mail_message **msg);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_send_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_mail_send_pre[hIndex].func;
			preHookFunc(&fd, &msg);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.mail_send(fd, msg);
	}
	if (HPMHooks.count.HP_mapif_mail_send_post > 0) {
		void (*postHookFunc) (int fd, struct mail_message *msg);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_send_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_mail_send_post[hIndex].func;
			postHookFunc(fd, msg);
		}
	}
	return;
}
void HP_mapif_parse_mail_send(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_mail_send_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mail_send_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_mail_send_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_mail_send(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_mail_send_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mail_send_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_mail_send_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
bool HP_mapif_mercenary_create(struct s_mercenary *merc) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_mapif_mercenary_create_pre > 0) {
		bool (*preHookFunc) (struct s_mercenary **merc);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_create_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_mercenary_create_pre[hIndex].func;
			retVal___ = preHookFunc(&merc);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.mercenary_create(merc);
	}
	if (HPMHooks.count.HP_mapif_mercenary_create_post > 0) {
		bool (*postHookFunc) (bool retVal___, struct s_mercenary *merc);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_create_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_mercenary_create_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, merc);
		}
	}
	return retVal___;
}
bool HP_mapif_mercenary_save(const struct s_mercenary *merc) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_mapif_mercenary_save_pre > 0) {
		bool (*preHookFunc) (const struct s_mercenary **merc);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_save_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_mercenary_save_pre[hIndex].func;
			retVal___ = preHookFunc(&merc);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.mercenary_save(merc);
	}
	if (HPMHooks.count.HP_mapif_mercenary_save_post > 0) {
		bool (*postHookFunc) (bool retVal___, const struct s_mercenary *merc);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_save_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_mercenary_save_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, merc);
		}
	}
	return retVal___;
}
bool HP_mapif_mercenary_load(int merc_id, int char_id, struct s_mercenary *merc) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_mapif_mercenary_load_pre > 0) {
		bool (*preHookFunc) (int *merc_id, int *char_id, struct s_mercenary **merc);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_load_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_mercenary_load_pre[hIndex].func;
			retVal___ = preHookFunc(&merc_id, &char_id, &merc);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.mercenary_load(merc_id, char_id, merc);
	}
	if (HPMHooks.count.HP_mapif_mercenary_load_post > 0) {
		bool (*postHookFunc) (bool retVal___, int merc_id, int char_id, struct s_mercenary *merc);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_load_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_mercenary_load_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, merc_id, char_id, merc);
		}
	}
	return retVal___;
}
bool HP_mapif_mercenary_delete(int merc_id) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_mapif_mercenary_delete_pre > 0) {
		bool (*preHookFunc) (int *merc_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_delete_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_mercenary_delete_pre[hIndex].func;
			retVal___ = preHookFunc(&merc_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.mercenary_delete(merc_id);
	}
	if (HPMHooks.count.HP_mapif_mercenary_delete_post > 0) {
		bool (*postHookFunc) (bool retVal___, int merc_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_delete_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_mercenary_delete_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, merc_id);
		}
	}
	return retVal___;
}
void HP_mapif_mercenary_send(int fd, struct s_mercenary *merc, unsigned char flag) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_mercenary_send_pre > 0) {
		void (*preHookFunc) (int *fd, struct s_mercenary **merc, unsigned char *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_send_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_mercenary_send_pre[hIndex].func;
			preHookFunc(&fd, &merc, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.mercenary_send(fd, merc, flag);
	}
	if (HPMHooks.count.HP_mapif_mercenary_send_post > 0) {
		void (*postHookFunc) (int fd, struct s_mercenary *merc, unsigned char flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_send_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_mercenary_send_post[hIndex].func;
			postHookFunc(fd, merc, flag);
		}
	}
	return;
}
void HP_mapif_parse_mercenary_create(int fd, const struct s_mercenary *merc) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_mercenary_create_pre > 0) {
		void (*preHookFunc) (int *fd, const struct s_mercenary **merc);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mercenary_create_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_mercenary_create_pre[hIndex].func;
			preHookFunc(&fd, &merc);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_mercenary_create(fd, merc);
	}
	if (HPMHooks.count.HP_mapif_parse_mercenary_create_post > 0) {
		void (*postHookFunc) (int fd, const struct s_mercenary *merc);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mercenary_create_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_mercenary_create_post[hIndex].func;
			postHookFunc(fd, merc);
		}
	}
	return;
}
void HP_mapif_parse_mercenary_load(int fd, int merc_id, int char_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_mercenary_load_pre > 0) {
		void (*preHookFunc) (int *fd, int *merc_id, int *char_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mercenary_load_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_mercenary_load_pre[hIndex].func;
			preHookFunc(&fd, &merc_id, &char_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_mercenary_load(fd, merc_id, char_id);
	}
	if (HPMHooks.count.HP_mapif_parse_mercenary_load_post > 0) {
		void (*postHookFunc) (int fd, int merc_id, int char_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mercenary_load_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_mercenary_load_post[hIndex].func;
			postHookFunc(fd, merc_id, char_id);
		}
	}
	return;
}
void HP_mapif_mercenary_deleted(int fd, unsigned char flag) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_mercenary_deleted_pre > 0) {
		void (*preHookFunc) (int *fd, unsigned char *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_deleted_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_mercenary_deleted_pre[hIndex].func;
			preHookFunc(&fd, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.mercenary_deleted(fd, flag);
	}
	if (HPMHooks.count.HP_mapif_mercenary_deleted_post > 0) {
		void (*postHookFunc) (int fd, unsigned char flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_deleted_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_mercenary_deleted_post[hIndex].func;
			postHookFunc(fd, flag);
		}
	}
	return;
}
void HP_mapif_parse_mercenary_delete(int fd, int merc_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_mercenary_delete_pre > 0) {
		void (*preHookFunc) (int *fd, int *merc_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mercenary_delete_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_mercenary_delete_pre[hIndex].func;
			preHookFunc(&fd, &merc_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_mercenary_delete(fd, merc_id);
	}
	if (HPMHooks.count.HP_mapif_parse_mercenary_delete_post > 0) {
		void (*postHookFunc) (int fd, int merc_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mercenary_delete_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_mercenary_delete_post[hIndex].func;
			postHookFunc(fd, merc_id);
		}
	}
	return;
}
void HP_mapif_mercenary_saved(int fd, unsigned char flag) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_mercenary_saved_pre > 0) {
		void (*preHookFunc) (int *fd, unsigned char *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_saved_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_mercenary_saved_pre[hIndex].func;
			preHookFunc(&fd, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.mercenary_saved(fd, flag);
	}
	if (HPMHooks.count.HP_mapif_mercenary_saved_post > 0) {
		void (*postHookFunc) (int fd, unsigned char flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_saved_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_mercenary_saved_post[hIndex].func;
			postHookFunc(fd, flag);
		}
	}
	return;
}
void HP_mapif_parse_mercenary_save(int fd, const struct s_mercenary *merc) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_mercenary_save_pre > 0) {
		void (*preHookFunc) (int *fd, const struct s_mercenary **merc);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mercenary_save_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_mercenary_save_pre[hIndex].func;
			preHookFunc(&fd, &merc);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_mercenary_save(fd, merc);
	}
	if (HPMHooks.count.HP_mapif_parse_mercenary_save_post > 0) {
		void (*postHookFunc) (int fd, const struct s_mercenary *merc);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mercenary_save_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_mercenary_save_post[hIndex].func;
			postHookFunc(fd, merc);
		}
	}
	return;
}
int HP_mapif_party_created(int fd, int account_id, int char_id, struct party *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_party_created_pre > 0) {
		int (*preHookFunc) (int *fd, int *account_id, int *char_id, struct party **p);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_created_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_party_created_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, &char_id, &p);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.party_created(fd, account_id, char_id, p);
	}
	if (HPMHooks.count.HP_mapif_party_created_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int account_id, int char_id, struct party *p);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_created_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_party_created_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, account_id, char_id, p);
		}
	}
	return retVal___;
}
void HP_mapif_party_noinfo(int fd, int party_id, int char_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_party_noinfo_pre > 0) {
		void (*preHookFunc) (int *fd, int *party_id, int *char_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_noinfo_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_party_noinfo_pre[hIndex].func;
			preHookFunc(&fd, &party_id, &char_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.party_noinfo(fd, party_id, char_id);
	}
	if (HPMHooks.count.HP_mapif_party_noinfo_post > 0) {
		void (*postHookFunc) (int fd, int party_id, int char_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_noinfo_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_party_noinfo_post[hIndex].func;
			postHookFunc(fd, party_id, char_id);
		}
	}
	return;
}
void HP_mapif_party_info(int fd, struct party *p, int char_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_party_info_pre > 0) {
		void (*preHookFunc) (int *fd, struct party **p, int *char_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_info_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_party_info_pre[hIndex].func;
			preHookFunc(&fd, &p, &char_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.party_info(fd, p, char_id);
	}
	if (HPMHooks.count.HP_mapif_party_info_post > 0) {
		void (*postHookFunc) (int fd, struct party *p, int char_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_info_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_party_info_post[hIndex].func;
			postHookFunc(fd, p, char_id);
		}
	}
	return;
}
int HP_mapif_party_memberadded(int fd, int party_id, int account_id, int char_id, int flag) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_party_memberadded_pre > 0) {
		int (*preHookFunc) (int *fd, int *party_id, int *account_id, int *char_id, int *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_memberadded_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_party_memberadded_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &party_id, &account_id, &char_id, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.party_memberadded(fd, party_id, account_id, char_id, flag);
	}
	if (HPMHooks.count.HP_mapif_party_memberadded_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int party_id, int account_id, int char_id, int flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_memberadded_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_party_memberadded_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, party_id, account_id, char_id, flag);
		}
	}
	return retVal___;
}
int HP_mapif_party_optionchanged(int fd, struct party *p, int account_id, int flag) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_party_optionchanged_pre > 0) {
		int (*preHookFunc) (int *fd, struct party **p, int *account_id, int *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_optionchanged_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_party_optionchanged_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &p, &account_id, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.party_optionchanged(fd, p, account_id, flag);
	}
	if (HPMHooks.count.HP_mapif_party_optionchanged_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, struct party *p, int account_id, int flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_optionchanged_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_party_optionchanged_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, p, account_id, flag);
		}
	}
	return retVal___;
}
int HP_mapif_party_withdraw(int party_id, int account_id, int char_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_party_withdraw_pre > 0) {
		int (*preHookFunc) (int *party_id, int *account_id, int *char_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_withdraw_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_party_withdraw_pre[hIndex].func;
			retVal___ = preHookFunc(&party_id, &account_id, &char_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.party_withdraw(party_id, account_id, char_id);
	}
	if (HPMHooks.count.HP_mapif_party_withdraw_post > 0) {
		int (*postHookFunc) (int retVal___, int party_id, int account_id, int char_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_withdraw_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_party_withdraw_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, party_id, account_id, char_id);
		}
	}
	return retVal___;
}
int HP_mapif_party_membermoved(struct party *p, int idx) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_party_membermoved_pre > 0) {
		int (*preHookFunc) (struct party **p, int *idx);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_membermoved_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_party_membermoved_pre[hIndex].func;
			retVal___ = preHookFunc(&p, &idx);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.party_membermoved(p, idx);
	}
	if (HPMHooks.count.HP_mapif_party_membermoved_post > 0) {
		int (*postHookFunc) (int retVal___, struct party *p, int idx);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_membermoved_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_party_membermoved_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, p, idx);
		}
	}
	return retVal___;
}
int HP_mapif_party_broken(int party_id, int flag) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_party_broken_pre > 0) {
		int (*preHookFunc) (int *party_id, int *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_broken_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_party_broken_pre[hIndex].func;
			retVal___ = preHookFunc(&party_id, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.party_broken(party_id, flag);
	}
	if (HPMHooks.count.HP_mapif_party_broken_post > 0) {
		int (*postHookFunc) (int retVal___, int party_id, int flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_broken_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_party_broken_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, party_id, flag);
		}
	}
	return retVal___;
}
int HP_mapif_party_message(int party_id, int account_id, const char *mes, int len, int sfd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_party_message_pre > 0) {
		int (*preHookFunc) (int *party_id, int *account_id, const char **mes, int *len, int *sfd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_message_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_party_message_pre[hIndex].func;
			retVal___ = preHookFunc(&party_id, &account_id, &mes, &len, &sfd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.party_message(party_id, account_id, mes, len, sfd);
	}
	if (HPMHooks.count.HP_mapif_party_message_post > 0) {
		int (*postHookFunc) (int retVal___, int party_id, int account_id, const char *mes, int len, int sfd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_message_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_party_message_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, party_id, account_id, mes, len, sfd);
		}
	}
	return retVal___;
}
int HP_mapif_parse_CreateParty(int fd, const char *name, int item, int item2, const struct party_member *leader) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_CreateParty_pre > 0) {
		int (*preHookFunc) (int *fd, const char **name, int *item, int *item2, const struct party_member **leader);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_CreateParty_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_CreateParty_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &name, &item, &item2, &leader);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_CreateParty(fd, name, item, item2, leader);
	}
	if (HPMHooks.count.HP_mapif_parse_CreateParty_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, const char *name, int item, int item2, const struct party_member *leader);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_CreateParty_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_CreateParty_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, name, item, item2, leader);
		}
	}
	return retVal___;
}
void HP_mapif_parse_PartyInfo(int fd, int party_id, int char_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_PartyInfo_pre > 0) {
		void (*preHookFunc) (int *fd, int *party_id, int *char_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyInfo_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_PartyInfo_pre[hIndex].func;
			preHookFunc(&fd, &party_id, &char_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_PartyInfo(fd, party_id, char_id);
	}
	if (HPMHooks.count.HP_mapif_parse_PartyInfo_post > 0) {
		void (*postHookFunc) (int fd, int party_id, int char_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyInfo_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_PartyInfo_post[hIndex].func;
			postHookFunc(fd, party_id, char_id);
		}
	}
	return;
}
int HP_mapif_parse_PartyAddMember(int fd, int party_id, const struct party_member *member) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_PartyAddMember_pre > 0) {
		int (*preHookFunc) (int *fd, int *party_id, const struct party_member **member);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyAddMember_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_PartyAddMember_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &party_id, &member);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_PartyAddMember(fd, party_id, member);
	}
	if (HPMHooks.count.HP_mapif_parse_PartyAddMember_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int party_id, const struct party_member *member);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyAddMember_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_PartyAddMember_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, party_id, member);
		}
	}
	return retVal___;
}
int HP_mapif_parse_PartyChangeOption(int fd, int party_id, int account_id, int exp, int item) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_PartyChangeOption_pre > 0) {
		int (*preHookFunc) (int *fd, int *party_id, int *account_id, int *exp, int *item);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyChangeOption_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_PartyChangeOption_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &party_id, &account_id, &exp, &item);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_PartyChangeOption(fd, party_id, account_id, exp, item);
	}
	if (HPMHooks.count.HP_mapif_parse_PartyChangeOption_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int party_id, int account_id, int exp, int item);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyChangeOption_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_PartyChangeOption_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, party_id, account_id, exp, item);
		}
	}
	return retVal___;
}
int HP_mapif_parse_PartyLeave(int fd, int party_id, int account_id, int char_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_PartyLeave_pre > 0) {
		int (*preHookFunc) (int *fd, int *party_id, int *account_id, int *char_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyLeave_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_PartyLeave_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &party_id, &account_id, &char_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_PartyLeave(fd, party_id, account_id, char_id);
	}
	if (HPMHooks.count.HP_mapif_parse_PartyLeave_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int party_id, int account_id, int char_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyLeave_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_PartyLeave_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, party_id, account_id, char_id);
		}
	}
	return retVal___;
}
int HP_mapif_parse_PartyChangeMap(int fd, int party_id, int account_id, int char_id, unsigned short map, int online, unsigned int lv) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_PartyChangeMap_pre > 0) {
		int (*preHookFunc) (int *fd, int *party_id, int *account_id, int *char_id, unsigned short *map, int *online, unsigned int *lv);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyChangeMap_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_PartyChangeMap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &party_id, &account_id, &char_id, &map, &online, &lv);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_PartyChangeMap(fd, party_id, account_id, char_id, map, online, lv);
	}
	if (HPMHooks.count.HP_mapif_parse_PartyChangeMap_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int party_id, int account_id, int char_id, unsigned short map, int online, unsigned int lv);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyChangeMap_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_PartyChangeMap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, party_id, account_id, char_id, map, online, lv);
		}
	}
	return retVal___;
}
int HP_mapif_parse_BreakParty(int fd, int party_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_BreakParty_pre > 0) {
		int (*preHookFunc) (int *fd, int *party_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_BreakParty_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_BreakParty_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &party_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_BreakParty(fd, party_id);
	}
	if (HPMHooks.count.HP_mapif_parse_BreakParty_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int party_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_BreakParty_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_BreakParty_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, party_id);
		}
	}
	return retVal___;
}
int HP_mapif_parse_PartyMessage(int fd, int party_id, int account_id, const char *mes, int len) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_PartyMessage_pre > 0) {
		int (*preHookFunc) (int *fd, int *party_id, int *account_id, const char **mes, int *len);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyMessage_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_PartyMessage_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &party_id, &account_id, &mes, &len);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_PartyMessage(fd, party_id, account_id, mes, len);
	}
	if (HPMHooks.count.HP_mapif_parse_PartyMessage_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int party_id, int account_id, const char *mes, int len);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyMessage_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_PartyMessage_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, party_id, account_id, mes, len);
		}
	}
	return retVal___;
}
int HP_mapif_parse_PartyLeaderChange(int fd, int party_id, int account_id, int char_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_PartyLeaderChange_pre > 0) {
		int (*preHookFunc) (int *fd, int *party_id, int *account_id, int *char_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyLeaderChange_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_PartyLeaderChange_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &party_id, &account_id, &char_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_PartyLeaderChange(fd, party_id, account_id, char_id);
	}
	if (HPMHooks.count.HP_mapif_parse_PartyLeaderChange_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int party_id, int account_id, int char_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyLeaderChange_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_PartyLeaderChange_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, party_id, account_id, char_id);
		}
	}
	return retVal___;
}
int HP_mapif_pet_created(int fd, int account_id, struct s_pet *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_pet_created_pre > 0) {
		int (*preHookFunc) (int *fd, int *account_id, struct s_pet **p);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_pet_created_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_pet_created_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, &p);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.pet_created(fd, account_id, p);
	}
	if (HPMHooks.count.HP_mapif_pet_created_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int account_id, struct s_pet *p);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_pet_created_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_pet_created_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, account_id, p);
		}
	}
	return retVal___;
}
int HP_mapif_pet_info(int fd, int account_id, struct s_pet *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_pet_info_pre > 0) {
		int (*preHookFunc) (int *fd, int *account_id, struct s_pet **p);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_pet_info_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_pet_info_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, &p);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.pet_info(fd, account_id, p);
	}
	if (HPMHooks.count.HP_mapif_pet_info_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int account_id, struct s_pet *p);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_pet_info_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_pet_info_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, account_id, p);
		}
	}
	return retVal___;
}
int HP_mapif_pet_noinfo(int fd, int account_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_pet_noinfo_pre > 0) {
		int (*preHookFunc) (int *fd, int *account_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_pet_noinfo_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_pet_noinfo_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.pet_noinfo(fd, account_id);
	}
	if (HPMHooks.count.HP_mapif_pet_noinfo_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int account_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_pet_noinfo_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_pet_noinfo_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, account_id);
		}
	}
	return retVal___;
}
int HP_mapif_save_pet_ack(int fd, int account_id, int flag) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_save_pet_ack_pre > 0) {
		int (*preHookFunc) (int *fd, int *account_id, int *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_save_pet_ack_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_save_pet_ack_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.save_pet_ack(fd, account_id, flag);
	}
	if (HPMHooks.count.HP_mapif_save_pet_ack_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int account_id, int flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_save_pet_ack_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_save_pet_ack_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, account_id, flag);
		}
	}
	return retVal___;
}
int HP_mapif_delete_pet_ack(int fd, int flag) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_delete_pet_ack_pre > 0) {
		int (*preHookFunc) (int *fd, int *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_delete_pet_ack_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_delete_pet_ack_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.delete_pet_ack(fd, flag);
	}
	if (HPMHooks.count.HP_mapif_delete_pet_ack_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_delete_pet_ack_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_delete_pet_ack_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, flag);
		}
	}
	return retVal___;
}
int HP_mapif_create_pet(int fd, int account_id, int char_id, short pet_class, short pet_lv, short pet_egg_id, short pet_equip, short intimate, short hungry, char rename_flag, char incubate, const char *pet_name) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_create_pet_pre > 0) {
		int (*preHookFunc) (int *fd, int *account_id, int *char_id, short *pet_class, short *pet_lv, short *pet_egg_id, short *pet_equip, short *intimate, short *hungry, char *rename_flag, char *incubate, const char **pet_name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_create_pet_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_create_pet_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, &char_id, &pet_class, &pet_lv, &pet_egg_id, &pet_equip, &intimate, &hungry, &rename_flag, &incubate, &pet_name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.create_pet(fd, account_id, char_id, pet_class, pet_lv, pet_egg_id, pet_equip, intimate, hungry, rename_flag, incubate, pet_name);
	}
	if (HPMHooks.count.HP_mapif_create_pet_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int account_id, int char_id, short pet_class, short pet_lv, short pet_egg_id, short pet_equip, short intimate, short hungry, char rename_flag, char incubate, const char *pet_name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_create_pet_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_create_pet_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, account_id, char_id, pet_class, pet_lv, pet_egg_id, pet_equip, intimate, hungry, rename_flag, incubate, pet_name);
		}
	}
	return retVal___;
}
int HP_mapif_load_pet(int fd, int account_id, int char_id, int pet_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_load_pet_pre > 0) {
		int (*preHookFunc) (int *fd, int *account_id, int *char_id, int *pet_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_load_pet_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_load_pet_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, &char_id, &pet_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.load_pet(fd, account_id, char_id, pet_id);
	}
	if (HPMHooks.count.HP_mapif_load_pet_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int account_id, int char_id, int pet_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_load_pet_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_load_pet_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, account_id, char_id, pet_id);
		}
	}
	return retVal___;
}
int HP_mapif_save_pet(int fd, int account_id, const struct s_pet *data) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_save_pet_pre > 0) {
		int (*preHookFunc) (int *fd, int *account_id, const struct s_pet **data);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_save_pet_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_save_pet_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, &data);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.save_pet(fd, account_id, data);
	}
	if (HPMHooks.count.HP_mapif_save_pet_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int account_id, const struct s_pet *data);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_save_pet_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_save_pet_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, account_id, data);
		}
	}
	return retVal___;
}
int HP_mapif_delete_pet(int fd, int pet_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_delete_pet_pre > 0) {
		int (*preHookFunc) (int *fd, int *pet_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_delete_pet_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_delete_pet_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &pet_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.delete_pet(fd, pet_id);
	}
	if (HPMHooks.count.HP_mapif_delete_pet_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int pet_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_delete_pet_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_delete_pet_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, pet_id);
		}
	}
	return retVal___;
}
int HP_mapif_parse_CreatePet(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_CreatePet_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_CreatePet_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_CreatePet_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_CreatePet(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_CreatePet_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_CreatePet_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_CreatePet_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_mapif_parse_LoadPet(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_LoadPet_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_LoadPet_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_LoadPet_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_LoadPet(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_LoadPet_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_LoadPet_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_LoadPet_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_mapif_parse_SavePet(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_SavePet_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_SavePet_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_SavePet_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_SavePet(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_SavePet_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_SavePet_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_SavePet_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_mapif_parse_DeletePet(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_DeletePet_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_DeletePet_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_DeletePet_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_DeletePet(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_DeletePet_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_DeletePet_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_DeletePet_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
struct quest* HP_mapif_quests_fromsql(int char_id, int *count) {
	int hIndex = 0;
	struct quest* retVal___ = NULL;
	if (HPMHooks.count.HP_mapif_quests_fromsql_pre > 0) {
		struct quest* (*preHookFunc) (int *char_id, int **count);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_quests_fromsql_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_quests_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &count);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.quests_fromsql(char_id, count);
	}
	if (HPMHooks.count.HP_mapif_quests_fromsql_post > 0) {
		struct quest* (*postHookFunc) (struct quest* retVal___, int char_id, int *count);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_quests_fromsql_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_quests_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, char_id, count);
		}
	}
	return retVal___;
}
bool HP_mapif_quest_delete(int char_id, int quest_id) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_mapif_quest_delete_pre > 0) {
		bool (*preHookFunc) (int *char_id, int *quest_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_quest_delete_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_quest_delete_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &quest_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.quest_delete(char_id, quest_id);
	}
	if (HPMHooks.count.HP_mapif_quest_delete_post > 0) {
		bool (*postHookFunc) (bool retVal___, int char_id, int quest_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_quest_delete_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_quest_delete_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, char_id, quest_id);
		}
	}
	return retVal___;
}
bool HP_mapif_quest_add(int char_id, struct quest qd) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_mapif_quest_add_pre > 0) {
		bool (*preHookFunc) (int *char_id, struct quest *qd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_quest_add_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_quest_add_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &qd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.quest_add(char_id, qd);
	}
	if (HPMHooks.count.HP_mapif_quest_add_post > 0) {
		bool (*postHookFunc) (bool retVal___, int char_id, struct quest qd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_quest_add_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_quest_add_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, char_id, qd);
		}
	}
	return retVal___;
}
bool HP_mapif_quest_update(int char_id, struct quest qd) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_mapif_quest_update_pre > 0) {
		bool (*preHookFunc) (int *char_id, struct quest *qd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_quest_update_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_quest_update_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &qd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.quest_update(char_id, qd);
	}
	if (HPMHooks.count.HP_mapif_quest_update_post > 0) {
		bool (*postHookFunc) (bool retVal___, int char_id, struct quest qd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_quest_update_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_quest_update_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, char_id, qd);
		}
	}
	return retVal___;
}
void HP_mapif_quest_save_ack(int fd, int char_id, bool success) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_quest_save_ack_pre > 0) {
		void (*preHookFunc) (int *fd, int *char_id, bool *success);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_quest_save_ack_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_quest_save_ack_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &success);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.quest_save_ack(fd, char_id, success);
	}
	if (HPMHooks.count.HP_mapif_quest_save_ack_post > 0) {
		void (*postHookFunc) (int fd, int char_id, bool success);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_quest_save_ack_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_quest_save_ack_post[hIndex].func;
			postHookFunc(fd, char_id, success);
		}
	}
	return;
}
int HP_mapif_parse_quest_save(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_quest_save_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_quest_save_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_quest_save_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_quest_save(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_quest_save_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_quest_save_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_quest_save_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
void HP_mapif_send_quests(int fd, int char_id, struct quest *tmp_questlog, int num_quests) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_send_quests_pre > 0) {
		void (*preHookFunc) (int *fd, int *char_id, struct quest **tmp_questlog, int *num_quests);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_send_quests_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_send_quests_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &tmp_questlog, &num_quests);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.send_quests(fd, char_id, tmp_questlog, num_quests);
	}
	if (HPMHooks.count.HP_mapif_send_quests_post > 0) {
		void (*postHookFunc) (int fd, int char_id, struct quest *tmp_questlog, int num_quests);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_send_quests_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_send_quests_post[hIndex].func;
			postHookFunc(fd, char_id, tmp_questlog, num_quests);
		}
	}
	return;
}
int HP_mapif_parse_quest_load(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_quest_load_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_quest_load_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_quest_load_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_quest_load(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_quest_load_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_quest_load_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_quest_load_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_mapif_parse_rodex_requestinbox(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_rodex_requestinbox_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_rodex_requestinbox_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_rodex_requestinbox_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_rodex_requestinbox(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_rodex_requestinbox_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_rodex_requestinbox_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_rodex_requestinbox_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
void HP_mapif_rodex_sendinbox(int fd, int char_id, int8 opentype, int8 flag, int count, struct rodex_maillist *mails) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_rodex_sendinbox_pre > 0) {
		void (*preHookFunc) (int *fd, int *char_id, int8 *opentype, int8 *flag, int *count, struct rodex_maillist **mails);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_rodex_sendinbox_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_rodex_sendinbox_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &opentype, &flag, &count, &mails);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.rodex_sendinbox(fd, char_id, opentype, flag, count, mails);
	}
	if (HPMHooks.count.HP_mapif_rodex_sendinbox_post > 0) {
		void (*postHookFunc) (int fd, int char_id, int8 opentype, int8 flag, int count, struct rodex_maillist *mails);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_rodex_sendinbox_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_rodex_sendinbox_post[hIndex].func;
			postHookFunc(fd, char_id, opentype, flag, count, mails);
		}
	}
	return;
}
int HP_mapif_parse_rodex_checkhasnew(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_rodex_checkhasnew_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_rodex_checkhasnew_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_rodex_checkhasnew_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_rodex_checkhasnew(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_rodex_checkhasnew_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_rodex_checkhasnew_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_rodex_checkhasnew_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
void HP_mapif_rodex_sendhasnew(int fd, int char_id, bool has_new) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_rodex_sendhasnew_pre > 0) {
		void (*preHookFunc) (int *fd, int *char_id, bool *has_new);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_rodex_sendhasnew_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_rodex_sendhasnew_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &has_new);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.rodex_sendhasnew(fd, char_id, has_new);
	}
	if (HPMHooks.count.HP_mapif_rodex_sendhasnew_post > 0) {
		void (*postHookFunc) (int fd, int char_id, bool has_new);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_rodex_sendhasnew_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_rodex_sendhasnew_post[hIndex].func;
			postHookFunc(fd, char_id, has_new);
		}
	}
	return;
}
int HP_mapif_parse_rodex_updatemail(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_rodex_updatemail_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_rodex_updatemail_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_rodex_updatemail_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_rodex_updatemail(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_rodex_updatemail_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_rodex_updatemail_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_rodex_updatemail_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_mapif_parse_rodex_send(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_rodex_send_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_rodex_send_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_rodex_send_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_rodex_send(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_rodex_send_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_rodex_send_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_rodex_send_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
void HP_mapif_rodex_send(int fd, int sender_id, int receiver_id, int receiver_accountid, bool result) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_rodex_send_pre > 0) {
		void (*preHookFunc) (int *fd, int *sender_id, int *receiver_id, int *receiver_accountid, bool *result);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_rodex_send_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_rodex_send_pre[hIndex].func;
			preHookFunc(&fd, &sender_id, &receiver_id, &receiver_accountid, &result);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.rodex_send(fd, sender_id, receiver_id, receiver_accountid, result);
	}
	if (HPMHooks.count.HP_mapif_rodex_send_post > 0) {
		void (*postHookFunc) (int fd, int sender_id, int receiver_id, int receiver_accountid, bool result);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_rodex_send_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_rodex_send_post[hIndex].func;
			postHookFunc(fd, sender_id, receiver_id, receiver_accountid, result);
		}
	}
	return;
}
int HP_mapif_parse_rodex_checkname(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_rodex_checkname_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_rodex_checkname_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_rodex_checkname_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_rodex_checkname(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_rodex_checkname_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_rodex_checkname_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_rodex_checkname_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
void HP_mapif_rodex_checkname(int fd, int reqchar_id, int target_char_id, short target_class, int target_level, char *name) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_rodex_checkname_pre > 0) {
		void (*preHookFunc) (int *fd, int *reqchar_id, int *target_char_id, short *target_class, int *target_level, char **name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_rodex_checkname_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_rodex_checkname_pre[hIndex].func;
			preHookFunc(&fd, &reqchar_id, &target_char_id, &target_class, &target_level, &name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.rodex_checkname(fd, reqchar_id, target_char_id, target_class, target_level, name);
	}
	if (HPMHooks.count.HP_mapif_rodex_checkname_post > 0) {
		void (*postHookFunc) (int fd, int reqchar_id, int target_char_id, short target_class, int target_level, char *name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_rodex_checkname_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_rodex_checkname_post[hIndex].func;
			postHookFunc(fd, reqchar_id, target_char_id, target_class, target_level, name);
		}
	}
	return;
}
int HP_mapif_load_guild_storage(int fd, int account_id, int guild_id, char flag) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_load_guild_storage_pre > 0) {
		int (*preHookFunc) (int *fd, int *account_id, int *guild_id, char *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_load_guild_storage_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_load_guild_storage_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, &guild_id, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.load_guild_storage(fd, account_id, guild_id, flag);
	}
	if (HPMHooks.count.HP_mapif_load_guild_storage_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int account_id, int guild_id, char flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_load_guild_storage_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_load_guild_storage_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, account_id, guild_id, flag);
		}
	}
	return retVal___;
}
int HP_mapif_save_guild_storage_ack(int fd, int account_id, int guild_id, int fail) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_save_guild_storage_ack_pre > 0) {
		int (*preHookFunc) (int *fd, int *account_id, int *guild_id, int *fail);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_save_guild_storage_ack_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_save_guild_storage_ack_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, &guild_id, &fail);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.save_guild_storage_ack(fd, account_id, guild_id, fail);
	}
	if (HPMHooks.count.HP_mapif_save_guild_storage_ack_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int account_id, int guild_id, int fail);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_save_guild_storage_ack_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_save_guild_storage_ack_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, account_id, guild_id, fail);
		}
	}
	return retVal___;
}
int HP_mapif_parse_LoadGuildStorage(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_LoadGuildStorage_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_LoadGuildStorage_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_LoadGuildStorage_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_LoadGuildStorage(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_LoadGuildStorage_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_LoadGuildStorage_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_LoadGuildStorage_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_mapif_parse_SaveGuildStorage(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_SaveGuildStorage_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_SaveGuildStorage_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_SaveGuildStorage_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_SaveGuildStorage(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_SaveGuildStorage_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_SaveGuildStorage_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_SaveGuildStorage_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_mapif_account_storage_load(int fd, int account_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_account_storage_load_pre > 0) {
		int (*preHookFunc) (int *fd, int *account_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_account_storage_load_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_account_storage_load_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.account_storage_load(fd, account_id);
	}
	if (HPMHooks.count.HP_mapif_account_storage_load_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int account_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_account_storage_load_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_account_storage_load_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, account_id);
		}
	}
	return retVal___;
}
int HP_mapif_pAccountStorageLoad(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_pAccountStorageLoad_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_pAccountStorageLoad_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_pAccountStorageLoad_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.pAccountStorageLoad(fd);
	}
	if (HPMHooks.count.HP_mapif_pAccountStorageLoad_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_pAccountStorageLoad_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_pAccountStorageLoad_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_mapif_pAccountStorageSave(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_pAccountStorageSave_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_pAccountStorageSave_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_pAccountStorageSave_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.pAccountStorageSave(fd);
	}
	if (HPMHooks.count.HP_mapif_pAccountStorageSave_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_pAccountStorageSave_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_pAccountStorageSave_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
void HP_mapif_sAccountStorageSaveAck(int fd, int account_id, bool save) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_sAccountStorageSaveAck_pre > 0) {
		void (*preHookFunc) (int *fd, int *account_id, bool *save);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_sAccountStorageSaveAck_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_sAccountStorageSaveAck_pre[hIndex].func;
			preHookFunc(&fd, &account_id, &save);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.sAccountStorageSaveAck(fd, account_id, save);
	}
	if (HPMHooks.count.HP_mapif_sAccountStorageSaveAck_post > 0) {
		void (*postHookFunc) (int fd, int account_id, bool save);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_sAccountStorageSaveAck_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_sAccountStorageSaveAck_post[hIndex].func;
			postHookFunc(fd, account_id, save);
		}
	}
	return;
}
int HP_mapif_itembound_ack(int fd, int aid, int guild_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_itembound_ack_pre > 0) {
		int (*preHookFunc) (int *fd, int *aid, int *guild_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_itembound_ack_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_itembound_ack_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &aid, &guild_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.itembound_ack(fd, aid, guild_id);
	}
	if (HPMHooks.count.HP_mapif_itembound_ack_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int aid, int guild_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_itembound_ack_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_itembound_ack_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, aid, guild_id);
		}
	}
	return retVal___;
}
int HP_mapif_parse_ItemBoundRetrieve_sub(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_ItemBoundRetrieve_sub_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_ItemBoundRetrieve_sub_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_ItemBoundRetrieve_sub_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_ItemBoundRetrieve_sub(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_ItemBoundRetrieve_sub_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_ItemBoundRetrieve_sub_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_ItemBoundRetrieve_sub_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
void HP_mapif_parse_ItemBoundRetrieve(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_ItemBoundRetrieve_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_ItemBoundRetrieve_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_ItemBoundRetrieve_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_ItemBoundRetrieve(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_ItemBoundRetrieve_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_ItemBoundRetrieve_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_ItemBoundRetrieve_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_mapif_parse_accinfo(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_accinfo_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_accinfo_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_accinfo_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_accinfo(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_accinfo_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_accinfo_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_accinfo_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_mapif_parse_accinfo2(bool success, int map_fd, int u_fd, int u_aid, int account_id, const char *userid, const char *user_pass, const char *email, const char *last_ip, const char *lastlogin, const char *pin_code, const char *birthdate, int group_id, int logincount, int state) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_parse_accinfo2_pre > 0) {
		void (*preHookFunc) (bool *success, int *map_fd, int *u_fd, int *u_aid, int *account_id, const char **userid, const char **user_pass, const char **email, const char **last_ip, const char **lastlogin, const char **pin_code, const char **birthdate, int *group_id, int *logincount, int *state);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_accinfo2_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_accinfo2_pre[hIndex].func;
			preHookFunc(&success, &map_fd, &u_fd, &u_aid, &account_id, &userid, &user_pass, &email, &last_ip, &lastlogin, &pin_code, &birthdate, &group_id, &logincount, &state);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_accinfo2(success, map_fd, u_fd, u_aid, account_id, userid, user_pass, email, last_ip, lastlogin, pin_code, birthdate, group_id, logincount, state);
	}
	if (HPMHooks.count.HP_mapif_parse_accinfo2_post > 0) {
		void (*postHookFunc) (bool success, int map_fd, int u_fd, int u_aid, int account_id, const char *userid, const char *user_pass, const char *email, const char *last_ip, const char *lastlogin, const char *pin_code, const char *birthdate, int group_id, int logincount, int state);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_accinfo2_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_accinfo2_post[hIndex].func;
			postHookFunc(success, map_fd, u_fd, u_aid, account_id, userid, user_pass, email, last_ip, lastlogin, pin_code, birthdate, group_id, logincount, state);
		}
	}
	return;
}
int HP_mapif_broadcast(const unsigned char *mes, int len, unsigned int fontColor, short fontType, short fontSize, short fontAlign, short fontY, int sfd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_broadcast_pre > 0) {
		int (*preHookFunc) (const unsigned char **mes, int *len, unsigned int *fontColor, short *fontType, short *fontSize, short *fontAlign, short *fontY, int *sfd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_broadcast_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_broadcast_pre[hIndex].func;
			retVal___ = preHookFunc(&mes, &len, &fontColor, &fontType, &fontSize, &fontAlign, &fontY, &sfd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.broadcast(mes, len, fontColor, fontType, fontSize, fontAlign, fontY, sfd);
	}
	if (HPMHooks.count.HP_mapif_broadcast_post > 0) {
		int (*postHookFunc) (int retVal___, const unsigned char *mes, int len, unsigned int fontColor, short fontType, short fontSize, short fontAlign, short fontY, int sfd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_broadcast_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_broadcast_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, mes, len, fontColor, fontType, fontSize, fontAlign, fontY, sfd);
		}
	}
	return retVal___;
}
int HP_mapif_wis_message(struct WisData *wd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_wis_message_pre > 0) {
		int (*preHookFunc) (struct WisData **wd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_wis_message_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_wis_message_pre[hIndex].func;
			retVal___ = preHookFunc(&wd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.wis_message(wd);
	}
	if (HPMHooks.count.HP_mapif_wis_message_post > 0) {
		int (*postHookFunc) (int retVal___, struct WisData *wd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_wis_message_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_wis_message_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, wd);
		}
	}
	return retVal___;
}
void HP_mapif_wis_response(int fd, const unsigned char *src, int flag) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_wis_response_pre > 0) {
		void (*preHookFunc) (int *fd, const unsigned char **src, int *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_wis_response_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_wis_response_pre[hIndex].func;
			preHookFunc(&fd, &src, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.wis_response(fd, src, flag);
	}
	if (HPMHooks.count.HP_mapif_wis_response_post > 0) {
		void (*postHookFunc) (int fd, const unsigned char *src, int flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_wis_response_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_wis_response_post[hIndex].func;
			postHookFunc(fd, src, flag);
		}
	}
	return;
}
int HP_mapif_wis_end(struct WisData *wd, int flag) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_wis_end_pre > 0) {
		int (*preHookFunc) (struct WisData **wd, int *flag);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_wis_end_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_wis_end_pre[hIndex].func;
			retVal___ = preHookFunc(&wd, &flag);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.wis_end(wd, flag);
	}
	if (HPMHooks.count.HP_mapif_wis_end_post > 0) {
		int (*postHookFunc) (int retVal___, struct WisData *wd, int flag);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_wis_end_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_wis_end_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, wd, flag);
		}
	}
	return retVal___;
}
int HP_mapif_account_reg_reply(int fd, int account_id, int char_id, int type) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_account_reg_reply_pre > 0) {
		int (*preHookFunc) (int *fd, int *account_id, int *char_id, int *type);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_account_reg_reply_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_account_reg_reply_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, &char_id, &type);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.account_reg_reply(fd, account_id, char_id, type);
	}
	if (HPMHooks.count.HP_mapif_account_reg_reply_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int account_id, int char_id, int type);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_account_reg_reply_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_account_reg_reply_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, account_id, char_id, type);
		}
	}
	return retVal___;
}
int HP_mapif_disconnectplayer(int fd, int account_id, int char_id, int reason) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_disconnectplayer_pre > 0) {
		int (*preHookFunc) (int *fd, int *account_id, int *char_id, int *reason);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_disconnectplayer_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_disconnectplayer_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, &char_id, &reason);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.disconnectplayer(fd, account_id, char_id, reason);
	}
	if (HPMHooks.count.HP_mapif_disconnectplayer_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, int account_id, int char_id, int reason);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_disconnectplayer_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_disconnectplayer_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, account_id, char_id, reason);
		}
	}
	return retVal___;
}
int HP_mapif_parse_broadcast(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_broadcast_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_broadcast_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_broadcast_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_broadcast(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_broadcast_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_broadcast_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_broadcast_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_mapif_parse_WisRequest(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_WisRequest_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_WisRequest_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_WisRequest_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_WisRequest(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_WisRequest_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_WisRequest_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_WisRequest_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_mapif_parse_WisReply(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_WisReply_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_WisReply_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_WisReply_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_WisReply(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_WisReply_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_WisReply_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_WisReply_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_mapif_parse_WisToGM(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_WisToGM_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_WisToGM_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_WisToGM_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_WisToGM(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_WisToGM_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_WisToGM_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_WisToGM_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_mapif_parse_Registry(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_Registry_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_Registry_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_Registry_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_Registry(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_Registry_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_Registry_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_Registry_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_mapif_parse_RegistryRequest(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_RegistryRequest_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_RegistryRequest_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_RegistryRequest_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_RegistryRequest(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_RegistryRequest_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_RegistryRequest_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_RegistryRequest_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
void HP_mapif_namechange_ack(int fd, int account_id, int char_id, int type, int flag, const char *name) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapif_namechange_ack_pre > 0) {
		void (*preHookFunc) (int *fd, int *account_id, int *char_id, int *type, int *flag, const char **name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_namechange_ack_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_namechange_ack_pre[hIndex].func;
			preHookFunc(&fd, &account_id, &char_id, &type, &flag, &name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.namechange_ack(fd, account_id, char_id, type, flag, name);
	}
	if (HPMHooks.count.HP_mapif_namechange_ack_post > 0) {
		void (*postHookFunc) (int fd, int account_id, int char_id, int type, int flag, const char *name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_namechange_ack_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_namechange_ack_post[hIndex].func;
			postHookFunc(fd, account_id, char_id, type, flag, name);
		}
	}
	return;
}
int HP_mapif_parse_NameChangeRequest(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapif_parse_NameChangeRequest_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_NameChangeRequest_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_NameChangeRequest_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_NameChangeRequest(fd);
	}
	if (HPMHooks.count.HP_mapif_parse_NameChangeRequest_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_NameChangeRequest_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_NameChangeRequest_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
/* mapindex_interface */
int HP_mapindex_init(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapindex_init_pre > 0) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapindex_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapindex_init_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapindex.init();
	}
	if (HPMHooks.count.HP_mapindex_init_post > 0) {
		int (*postHookFunc) (int retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapindex_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapindex_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_mapindex_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapindex_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapindex_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapindex_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapindex.final();
	}
	if (HPMHooks.count.HP_mapindex_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapindex_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapindex_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_mapindex_addmap(int index, const char *name) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_mapindex_addmap_pre > 0) {
		int (*preHookFunc) (int *index, const char **name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapindex_addmap_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapindex_addmap_pre[hIndex].func;
			retVal___ = preHookFunc(&index, &name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapindex.addmap(index, name);
	}
	if (HPMHooks.count.HP_mapindex_addmap_post > 0) {
		int (*postHookFunc) (int retVal___, int index, const char *name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapindex_addmap_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapindex_addmap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, index, name);
		}
	}
	return retVal___;
}
void HP_mapindex_removemap(int index) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mapindex_removemap_pre > 0) {
		void (*preHookFunc) (int *index);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapindex_removemap_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapindex_removemap_pre[hIndex].func;
			preHookFunc(&index);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapindex.removemap(index);
	}
	if (HPMHooks.count.HP_mapindex_removemap_post > 0) {
		void (*postHookFunc) (int index);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapindex_removemap_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapindex_removemap_post[hIndex].func;
			postHookFunc(index);
		}
	}
	return;
}
const char* HP_mapindex_getmapname(const char *string, char *output) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if (HPMHooks.count.HP_mapindex_getmapname_pre > 0) {
		const char* (*preHookFunc) (const char **string, char **output);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapindex_getmapname_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapindex_getmapname_pre[hIndex].func;
			retVal___ = preHookFunc(&string, &output);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapindex.getmapname(string, output);
	}
	if (HPMHooks.count.HP_mapindex_getmapname_post > 0) {
		const char* (*postHookFunc) (const char* retVal___, const char *string, char *output);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapindex_getmapname_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapindex_getmapname_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, string, output);
		}
	}
	return retVal___;
}
const char* HP_mapindex_getmapname_ext(const char *string, char *output) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if (HPMHooks.count.HP_mapindex_getmapname_ext_pre > 0) {
		const char* (*preHookFunc) (const char **string, char **output);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapindex_getmapname_ext_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapindex_getmapname_ext_pre[hIndex].func;
			retVal___ = preHookFunc(&string, &output);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapindex.getmapname_ext(string, output);
	}
	if (HPMHooks.count.HP_mapindex_getmapname_ext_post > 0) {
		const char* (*postHookFunc) (const char* retVal___, const char *string, char *output);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapindex_getmapname_ext_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapindex_getmapname_ext_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, string, output);
		}
	}
	return retVal___;
}
unsigned short HP_mapindex_name2id(const char *p1) {
	int hIndex = 0;
	unsigned short retVal___ = 0;
	if (HPMHooks.count.HP_mapindex_name2id_pre > 0) {
		unsigned short (*preHookFunc) (const char **p1);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapindex_name2id_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapindex_name2id_pre[hIndex].func;
			retVal___ = preHookFunc(&p1);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapindex.name2id(p1);
	}
	if (HPMHooks.count.HP_mapindex_name2id_post > 0) {
		unsigned short (*postHookFunc) (unsigned short retVal___, const char *p1);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapindex_name2id_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapindex_name2id_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, p1);
		}
	}
	return retVal___;
}
const char* HP_mapindex_id2name(uint16 id, const char *file, int line, const char *func) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if (HPMHooks.count.HP_mapindex_id2name_pre > 0) {
		const char* (*preHookFunc) (uint16 *id, const char **file, int *line, const char **func);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapindex_id2name_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapindex_id2name_pre[hIndex].func;
			retVal___ = preHookFunc(&id, &file, &line, &func);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapindex.id2name(id, file, line, func);
	}
	if (HPMHooks.count.HP_mapindex_id2name_post > 0) {
		const char* (*postHookFunc) (const char* retVal___, uint16 id, const char *file, int line, const char *func);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapindex_id2name_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapindex_id2name_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, id, file, line, func);
		}
	}
	return retVal___;
}
bool HP_mapindex_check_default(void) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_mapindex_check_default_pre > 0) {
		bool (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapindex_check_default_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mapindex_check_default_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapindex.check_default();
	}
	if (HPMHooks.count.HP_mapindex_check_default_post > 0) {
		bool (*postHookFunc) (bool retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mapindex_check_default_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mapindex_check_default_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
/* md5_interface */
void HP_md5_string(const char *string, char *output) {
	int hIndex = 0;
	if (HPMHooks.count.HP_md5_string_pre > 0) {
		void (*preHookFunc) (const char **string, char **output);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_md5_string_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_md5_string_pre[hIndex].func;
			preHookFunc(&string, &output);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.md5.string(string, output);
	}
	if (HPMHooks.count.HP_md5_string_post > 0) {
		void (*postHookFunc) (const char *string, char *output);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_md5_string_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_md5_string_post[hIndex].func;
			postHookFunc(string, output);
		}
	}
	return;
}
void HP_md5_binary(const uint8 *buf, const int buf_size, uint8 *output) {
	int hIndex = 0;
	if (HPMHooks.count.HP_md5_binary_pre > 0) {
		void (*preHookFunc) (const uint8 **buf, const int *buf_size, uint8 **output);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_md5_binary_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_md5_binary_pre[hIndex].func;
			preHookFunc(&buf, &buf_size, &output);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.md5.binary(buf, buf_size, output);
	}
	if (HPMHooks.count.HP_md5_binary_post > 0) {
		void (*postHookFunc) (const uint8 *buf, const int buf_size, uint8 *output);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_md5_binary_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_md5_binary_post[hIndex].func;
			postHookFunc(buf, buf_size, output);
		}
	}
	return;
}
void HP_md5_salt(int len, char *output) {
	int hIndex = 0;
	if (HPMHooks.count.HP_md5_salt_pre > 0) {
		void (*preHookFunc) (int *len, char **output);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_md5_salt_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_md5_salt_pre[hIndex].func;
			preHookFunc(&len, &output);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.md5.salt(len, output);
	}
	if (HPMHooks.count.HP_md5_salt_post > 0) {
		void (*postHookFunc) (int len, char *output);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_md5_salt_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_md5_salt_post[hIndex].func;
			postHookFunc(len, output);
		}
	}
	return;
}
/* mutex_interface */
struct mutex_data* HP_mutex_create(void) {
	int hIndex = 0;
	struct mutex_data* retVal___ = NULL;
	if (HPMHooks.count.HP_mutex_create_pre > 0) {
		struct mutex_data* (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mutex_create_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mutex_create_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mutex.create();
	}
	if (HPMHooks.count.HP_mutex_create_post > 0) {
		struct mutex_data* (*postHookFunc) (struct mutex_data* retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mutex_create_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mutex_create_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_mutex_destroy(struct mutex_data *m) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mutex_destroy_pre > 0) {
		void (*preHookFunc) (struct mutex_data **m);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mutex_destroy_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mutex_destroy_pre[hIndex].func;
			preHookFunc(&m);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mutex.destroy(m);
	}
	if (HPMHooks.count.HP_mutex_destroy_post > 0) {
		void (*postHookFunc) (struct mutex_data *m);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mutex_destroy_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mutex_destroy_post[hIndex].func;
			postHookFunc(m);
		}
	}
	return;
}
void HP_mutex_lock(struct mutex_data *m) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mutex_lock_pre > 0) {
		void (*preHookFunc) (struct mutex_data **m);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mutex_lock_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mutex_lock_pre[hIndex].func;
			preHookFunc(&m);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mutex.lock(m);
	}
	if (HPMHooks.count.HP_mutex_lock_post > 0) {
		void (*postHookFunc) (struct mutex_data *m);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mutex_lock_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mutex_lock_post[hIndex].func;
			postHookFunc(m);
		}
	}
	return;
}
bool HP_mutex_trylock(struct mutex_data *m) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_mutex_trylock_pre > 0) {
		bool (*preHookFunc) (struct mutex_data **m);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mutex_trylock_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mutex_trylock_pre[hIndex].func;
			retVal___ = preHookFunc(&m);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mutex.trylock(m);
	}
	if (HPMHooks.count.HP_mutex_trylock_post > 0) {
		bool (*postHookFunc) (bool retVal___, struct mutex_data *m);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mutex_trylock_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mutex_trylock_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, m);
		}
	}
	return retVal___;
}
void HP_mutex_unlock(struct mutex_data *m) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mutex_unlock_pre > 0) {
		void (*preHookFunc) (struct mutex_data **m);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mutex_unlock_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mutex_unlock_pre[hIndex].func;
			preHookFunc(&m);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mutex.unlock(m);
	}
	if (HPMHooks.count.HP_mutex_unlock_post > 0) {
		void (*postHookFunc) (struct mutex_data *m);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mutex_unlock_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mutex_unlock_post[hIndex].func;
			postHookFunc(m);
		}
	}
	return;
}
struct cond_data* HP_mutex_cond_create(void) {
	int hIndex = 0;
	struct cond_data* retVal___ = NULL;
	if (HPMHooks.count.HP_mutex_cond_create_pre > 0) {
		struct cond_data* (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mutex_cond_create_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mutex_cond_create_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mutex.cond_create();
	}
	if (HPMHooks.count.HP_mutex_cond_create_post > 0) {
		struct cond_data* (*postHookFunc) (struct cond_data* retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mutex_cond_create_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mutex_cond_create_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_mutex_cond_destroy(struct cond_data *c) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mutex_cond_destroy_pre > 0) {
		void (*preHookFunc) (struct cond_data **c);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mutex_cond_destroy_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mutex_cond_destroy_pre[hIndex].func;
			preHookFunc(&c);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mutex.cond_destroy(c);
	}
	if (HPMHooks.count.HP_mutex_cond_destroy_post > 0) {
		void (*postHookFunc) (struct cond_data *c);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mutex_cond_destroy_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mutex_cond_destroy_post[hIndex].func;
			postHookFunc(c);
		}
	}
	return;
}
void HP_mutex_cond_wait(struct cond_data *c, struct mutex_data *m, sysint timeout_ticks) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mutex_cond_wait_pre > 0) {
		void (*preHookFunc) (struct cond_data **c, struct mutex_data **m, sysint *timeout_ticks);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mutex_cond_wait_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mutex_cond_wait_pre[hIndex].func;
			preHookFunc(&c, &m, &timeout_ticks);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mutex.cond_wait(c, m, timeout_ticks);
	}
	if (HPMHooks.count.HP_mutex_cond_wait_post > 0) {
		void (*postHookFunc) (struct cond_data *c, struct mutex_data *m, sysint timeout_ticks);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mutex_cond_wait_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mutex_cond_wait_post[hIndex].func;
			postHookFunc(c, m, timeout_ticks);
		}
	}
	return;
}
void HP_mutex_cond_signal(struct cond_data *c) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mutex_cond_signal_pre > 0) {
		void (*preHookFunc) (struct cond_data **c);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mutex_cond_signal_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mutex_cond_signal_pre[hIndex].func;
			preHookFunc(&c);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mutex.cond_signal(c);
	}
	if (HPMHooks.count.HP_mutex_cond_signal_post > 0) {
		void (*postHookFunc) (struct cond_data *c);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mutex_cond_signal_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mutex_cond_signal_post[hIndex].func;
			postHookFunc(c);
		}
	}
	return;
}
void HP_mutex_cond_broadcast(struct cond_data *c) {
	int hIndex = 0;
	if (HPMHooks.count.HP_mutex_cond_broadcast_pre > 0) {
		void (*preHookFunc) (struct cond_data **c);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mutex_cond_broadcast_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_mutex_cond_broadcast_pre[hIndex].func;
			preHookFunc(&c);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mutex.cond_broadcast(c);
	}
	if (HPMHooks.count.HP_mutex_cond_broadcast_post > 0) {
		void (*postHookFunc) (struct cond_data *c);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_mutex_cond_broadcast_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_mutex_cond_broadcast_post[hIndex].func;
			postHookFunc(c);
		}
	}
	return;
}
/* nullpo_interface */
void HP_nullpo_assert_report(const char *file, int line, const char *func, const char *targetname, const char *title) {
	int hIndex = 0;
	if (HPMHooks.count.HP_nullpo_assert_report_pre > 0) {
		void (*preHookFunc) (const char **file, int *line, const char **func, const char **targetname, const char **title);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_nullpo_assert_report_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_nullpo_assert_report_pre[hIndex].func;
			preHookFunc(&file, &line, &func, &targetname, &title);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.nullpo.assert_report(file, line, func, targetname, title);
	}
	if (HPMHooks.count.HP_nullpo_assert_report_post > 0) {
		void (*postHookFunc) (const char *file, int line, const char *func, const char *targetname, const char *title);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_nullpo_assert_report_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_nullpo_assert_report_post[hIndex].func;
			postHookFunc(file, line, func, targetname, title);
		}
	}
	return;
}
/* pincode_interface */
void HP_pincode_handle(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_pincode_handle_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_pincode_handle_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_pincode_handle_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.pincode.handle(fd, sd);
	}
	if (HPMHooks.count.HP_pincode_handle_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_pincode_handle_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_pincode_handle_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_pincode_decrypt(unsigned int userSeed, char *pin) {
	int hIndex = 0;
	if (HPMHooks.count.HP_pincode_decrypt_pre > 0) {
		void (*preHookFunc) (unsigned int *userSeed, char **pin);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_pincode_decrypt_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_pincode_decrypt_pre[hIndex].func;
			preHookFunc(&userSeed, &pin);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.pincode.decrypt(userSeed, pin);
	}
	if (HPMHooks.count.HP_pincode_decrypt_post > 0) {
		void (*postHookFunc) (unsigned int userSeed, char *pin);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_pincode_decrypt_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_pincode_decrypt_post[hIndex].func;
			postHookFunc(userSeed, pin);
		}
	}
	return;
}
void HP_pincode_error(int account_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_pincode_error_pre > 0) {
		void (*preHookFunc) (int *account_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_pincode_error_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_pincode_error_pre[hIndex].func;
			preHookFunc(&account_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.pincode.error(account_id);
	}
	if (HPMHooks.count.HP_pincode_error_post > 0) {
		void (*postHookFunc) (int account_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_pincode_error_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_pincode_error_post[hIndex].func;
			postHookFunc(account_id);
		}
	}
	return;
}
void HP_pincode_update(int account_id, char *pin) {
	int hIndex = 0;
	if (HPMHooks.count.HP_pincode_update_pre > 0) {
		void (*preHookFunc) (int *account_id, char **pin);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_pincode_update_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_pincode_update_pre[hIndex].func;
			preHookFunc(&account_id, &pin);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.pincode.update(account_id, pin);
	}
	if (HPMHooks.count.HP_pincode_update_post > 0) {
		void (*postHookFunc) (int account_id, char *pin);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_pincode_update_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_pincode_update_post[hIndex].func;
			postHookFunc(account_id, pin);
		}
	}
	return;
}
void HP_pincode_sendstate(int fd, struct char_session_data *sd, uint16 state) {
	int hIndex = 0;
	if (HPMHooks.count.HP_pincode_sendstate_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd, uint16 *state);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_pincode_sendstate_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_pincode_sendstate_pre[hIndex].func;
			preHookFunc(&fd, &sd, &state);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.pincode.sendstate(fd, sd, state);
	}
	if (HPMHooks.count.HP_pincode_sendstate_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd, uint16 state);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_pincode_sendstate_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_pincode_sendstate_post[hIndex].func;
			postHookFunc(fd, sd, state);
		}
	}
	return;
}
void HP_pincode_setnew(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_pincode_setnew_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_pincode_setnew_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_pincode_setnew_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.pincode.setnew(fd, sd);
	}
	if (HPMHooks.count.HP_pincode_setnew_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_pincode_setnew_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_pincode_setnew_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_pincode_change(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_pincode_change_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_pincode_change_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_pincode_change_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.pincode.change(fd, sd);
	}
	if (HPMHooks.count.HP_pincode_change_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_pincode_change_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_pincode_change_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
int HP_pincode_compare(int fd, struct char_session_data *sd, char *pin) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_pincode_compare_pre > 0) {
		int (*preHookFunc) (int *fd, struct char_session_data **sd, char **pin);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_pincode_compare_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_pincode_compare_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &sd, &pin);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.pincode.compare(fd, sd, pin);
	}
	if (HPMHooks.count.HP_pincode_compare_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, struct char_session_data *sd, char *pin);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_pincode_compare_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_pincode_compare_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, sd, pin);
		}
	}
	return retVal___;
}
void HP_pincode_check(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_pincode_check_pre > 0) {
		void (*preHookFunc) (int *fd, struct char_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_pincode_check_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_pincode_check_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.pincode.check(fd, sd);
	}
	if (HPMHooks.count.HP_pincode_check_post > 0) {
		void (*postHookFunc) (int fd, struct char_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_pincode_check_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_pincode_check_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
bool HP_pincode_config_read(const char *filename, const struct config_t *config, bool imported) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_pincode_config_read_pre > 0) {
		bool (*preHookFunc) (const char **filename, const struct config_t **config, bool *imported);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_pincode_config_read_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_pincode_config_read_pre[hIndex].func;
			retVal___ = preHookFunc(&filename, &config, &imported);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.pincode.config_read(filename, config, imported);
	}
	if (HPMHooks.count.HP_pincode_config_read_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *filename, const struct config_t *config, bool imported);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_pincode_config_read_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_pincode_config_read_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, filename, config, imported);
		}
	}
	return retVal___;
}
/* rnd_interface */
void HP_rnd_init(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_rnd_init_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_rnd_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_rnd_init_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.rnd.init();
	}
	if (HPMHooks.count.HP_rnd_init_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_rnd_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_rnd_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_rnd_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_rnd_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_rnd_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_rnd_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.rnd.final();
	}
	if (HPMHooks.count.HP_rnd_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_rnd_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_rnd_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_rnd_seed(uint32 seed) {
	int hIndex = 0;
	if (HPMHooks.count.HP_rnd_seed_pre > 0) {
		void (*preHookFunc) (uint32 *seed);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_rnd_seed_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_rnd_seed_pre[hIndex].func;
			preHookFunc(&seed);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.rnd.seed(seed);
	}
	if (HPMHooks.count.HP_rnd_seed_post > 0) {
		void (*postHookFunc) (uint32 seed);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_rnd_seed_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_rnd_seed_post[hIndex].func;
			postHookFunc(seed);
		}
	}
	return;
}
int32 HP_rnd_random(void) {
	int hIndex = 0;
	int32 retVal___ = 0;
	if (HPMHooks.count.HP_rnd_random_pre > 0) {
		int32 (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_rnd_random_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_rnd_random_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.rnd.random();
	}
	if (HPMHooks.count.HP_rnd_random_post > 0) {
		int32 (*postHookFunc) (int32 retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_rnd_random_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_rnd_random_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
uint32 HP_rnd_roll(uint32 dice_faces) {
	int hIndex = 0;
	uint32 retVal___ = 0;
	if (HPMHooks.count.HP_rnd_roll_pre > 0) {
		uint32 (*preHookFunc) (uint32 *dice_faces);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_rnd_roll_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_rnd_roll_pre[hIndex].func;
			retVal___ = preHookFunc(&dice_faces);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.rnd.roll(dice_faces);
	}
	if (HPMHooks.count.HP_rnd_roll_post > 0) {
		uint32 (*postHookFunc) (uint32 retVal___, uint32 dice_faces);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_rnd_roll_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_rnd_roll_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, dice_faces);
		}
	}
	return retVal___;
}
int32 HP_rnd_value(int32 min, int32 max) {
	int hIndex = 0;
	int32 retVal___ = 0;
	if (HPMHooks.count.HP_rnd_value_pre > 0) {
		int32 (*preHookFunc) (int32 *min, int32 *max);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_rnd_value_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_rnd_value_pre[hIndex].func;
			retVal___ = preHookFunc(&min, &max);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.rnd.value(min, max);
	}
	if (HPMHooks.count.HP_rnd_value_post > 0) {
		int32 (*postHookFunc) (int32 retVal___, int32 min, int32 max);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_rnd_value_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_rnd_value_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, min, max);
		}
	}
	return retVal___;
}
double HP_rnd_uniform(void) {
	int hIndex = 0;
	double retVal___ = 0.;
	if (HPMHooks.count.HP_rnd_uniform_pre > 0) {
		double (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_rnd_uniform_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_rnd_uniform_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.rnd.uniform();
	}
	if (HPMHooks.count.HP_rnd_uniform_post > 0) {
		double (*postHookFunc) (double retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_rnd_uniform_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_rnd_uniform_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
double HP_rnd_uniform53(void) {
	int hIndex = 0;
	double retVal___ = 0.;
	if (HPMHooks.count.HP_rnd_uniform53_pre > 0) {
		double (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_rnd_uniform53_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_rnd_uniform53_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.rnd.uniform53();
	}
	if (HPMHooks.count.HP_rnd_uniform53_post > 0) {
		double (*postHookFunc) (double retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_rnd_uniform53_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_rnd_uniform53_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
/* showmsg_interface */
void HP_showmsg_init(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_showmsg_init_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_showmsg_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_showmsg_init_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.showmsg.init();
	}
	if (HPMHooks.count.HP_showmsg_init_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_showmsg_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_showmsg_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_showmsg_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_showmsg_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_showmsg_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_showmsg_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.showmsg.final();
	}
	if (HPMHooks.count.HP_showmsg_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_showmsg_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_showmsg_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_showmsg_clearScreen(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_showmsg_clearScreen_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_showmsg_clearScreen_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_showmsg_clearScreen_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.showmsg.clearScreen();
	}
	if (HPMHooks.count.HP_showmsg_clearScreen_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_showmsg_clearScreen_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_showmsg_clearScreen_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_showmsg_showMessageV(const char *string, va_list ap) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_showmsg_showMessageV_pre > 0) {
		int (*preHookFunc) (const char **string, va_list ap);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_showmsg_showMessageV_pre; hIndex++) {
			va_list ap___copy; va_copy(ap___copy, ap);
			preHookFunc = HPMHooks.list.HP_showmsg_showMessageV_pre[hIndex].func;
			retVal___ = preHookFunc(&string, ap___copy);
			va_end(ap___copy);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		va_list ap___copy; va_copy(ap___copy, ap);
		retVal___ = HPMHooks.source.showmsg.showMessageV(string, ap___copy);
		va_end(ap___copy);
	}
	if (HPMHooks.count.HP_showmsg_showMessageV_post > 0) {
		int (*postHookFunc) (int retVal___, const char *string, va_list ap);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_showmsg_showMessageV_post; hIndex++) {
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
	if (HPMHooks.count.HP_sockt_init_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_init_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.init();
	}
	if (HPMHooks.count.HP_sockt_init_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_sockt_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_sockt_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.final();
	}
	if (HPMHooks.count.HP_sockt_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_sockt_perform(int next) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_sockt_perform_pre > 0) {
		int (*preHookFunc) (int *next);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_perform_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_perform_pre[hIndex].func;
			retVal___ = preHookFunc(&next);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.perform(next);
	}
	if (HPMHooks.count.HP_sockt_perform_post > 0) {
		int (*postHookFunc) (int retVal___, int next);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_perform_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_perform_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, next);
		}
	}
	return retVal___;
}
void HP_sockt_datasync(int fd, bool send) {
	int hIndex = 0;
	if (HPMHooks.count.HP_sockt_datasync_pre > 0) {
		void (*preHookFunc) (int *fd, bool *send);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_datasync_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_datasync_pre[hIndex].func;
			preHookFunc(&fd, &send);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.datasync(fd, send);
	}
	if (HPMHooks.count.HP_sockt_datasync_post > 0) {
		void (*postHookFunc) (int fd, bool send);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_datasync_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_datasync_post[hIndex].func;
			postHookFunc(fd, send);
		}
	}
	return;
}
int HP_sockt_make_listen_bind(uint32 ip, uint16 port) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_sockt_make_listen_bind_pre > 0) {
		int (*preHookFunc) (uint32 *ip, uint16 *port);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_make_listen_bind_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_make_listen_bind_pre[hIndex].func;
			retVal___ = preHookFunc(&ip, &port);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.make_listen_bind(ip, port);
	}
	if (HPMHooks.count.HP_sockt_make_listen_bind_post > 0) {
		int (*postHookFunc) (int retVal___, uint32 ip, uint16 port);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_make_listen_bind_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_make_listen_bind_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, ip, port);
		}
	}
	return retVal___;
}
int HP_sockt_make_connection(uint32 ip, uint16 port, struct hSockOpt *opt) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_sockt_make_connection_pre > 0) {
		int (*preHookFunc) (uint32 *ip, uint16 *port, struct hSockOpt **opt);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_make_connection_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_make_connection_pre[hIndex].func;
			retVal___ = preHookFunc(&ip, &port, &opt);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.make_connection(ip, port, opt);
	}
	if (HPMHooks.count.HP_sockt_make_connection_post > 0) {
		int (*postHookFunc) (int retVal___, uint32 ip, uint16 port, struct hSockOpt *opt);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_make_connection_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_make_connection_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, ip, port, opt);
		}
	}
	return retVal___;
}
int HP_sockt_realloc_fifo(int fd, unsigned int rfifo_size, unsigned int wfifo_size) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_sockt_realloc_fifo_pre > 0) {
		int (*preHookFunc) (int *fd, unsigned int *rfifo_size, unsigned int *wfifo_size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_realloc_fifo_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_realloc_fifo_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &rfifo_size, &wfifo_size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.realloc_fifo(fd, rfifo_size, wfifo_size);
	}
	if (HPMHooks.count.HP_sockt_realloc_fifo_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, unsigned int rfifo_size, unsigned int wfifo_size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_realloc_fifo_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_realloc_fifo_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, rfifo_size, wfifo_size);
		}
	}
	return retVal___;
}
int HP_sockt_realloc_writefifo(int fd, size_t addition) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_sockt_realloc_writefifo_pre > 0) {
		int (*preHookFunc) (int *fd, size_t *addition);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_realloc_writefifo_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_realloc_writefifo_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &addition);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.realloc_writefifo(fd, addition);
	}
	if (HPMHooks.count.HP_sockt_realloc_writefifo_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, size_t addition);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_realloc_writefifo_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_realloc_writefifo_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, addition);
		}
	}
	return retVal___;
}
int HP_sockt_wfifoset(int fd, size_t len) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_sockt_wfifoset_pre > 0) {
		int (*preHookFunc) (int *fd, size_t *len);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_wfifoset_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_wfifoset_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &len);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.wfifoset(fd, len);
	}
	if (HPMHooks.count.HP_sockt_wfifoset_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, size_t len);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_wfifoset_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_wfifoset_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, len);
		}
	}
	return retVal___;
}
int HP_sockt_rfifoskip(int fd, size_t len) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_sockt_rfifoskip_pre > 0) {
		int (*preHookFunc) (int *fd, size_t *len);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_rfifoskip_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_rfifoskip_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &len);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.rfifoskip(fd, len);
	}
	if (HPMHooks.count.HP_sockt_rfifoskip_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, size_t len);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_rfifoskip_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_rfifoskip_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, len);
		}
	}
	return retVal___;
}
void HP_sockt_close(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_sockt_close_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_close_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_close_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.close(fd);
	}
	if (HPMHooks.count.HP_sockt_close_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_close_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_close_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
bool HP_sockt_session_is_valid(int fd) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_sockt_session_is_valid_pre > 0) {
		bool (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_session_is_valid_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_session_is_valid_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.session_is_valid(fd);
	}
	if (HPMHooks.count.HP_sockt_session_is_valid_post > 0) {
		bool (*postHookFunc) (bool retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_session_is_valid_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_session_is_valid_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
bool HP_sockt_session_is_active(int fd) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_sockt_session_is_active_pre > 0) {
		bool (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_session_is_active_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_session_is_active_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.session_is_active(fd);
	}
	if (HPMHooks.count.HP_sockt_session_is_active_post > 0) {
		bool (*postHookFunc) (bool retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_session_is_active_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_session_is_active_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
void HP_sockt_flush(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_sockt_flush_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_flush_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_flush_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.flush(fd);
	}
	if (HPMHooks.count.HP_sockt_flush_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_flush_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_flush_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_sockt_flush_fifos(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_sockt_flush_fifos_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_flush_fifos_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_flush_fifos_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.flush_fifos();
	}
	if (HPMHooks.count.HP_sockt_flush_fifos_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_flush_fifos_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_flush_fifos_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_sockt_set_nonblocking(int fd, unsigned long yes) {
	int hIndex = 0;
	if (HPMHooks.count.HP_sockt_set_nonblocking_pre > 0) {
		void (*preHookFunc) (int *fd, unsigned long *yes);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_set_nonblocking_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_set_nonblocking_pre[hIndex].func;
			preHookFunc(&fd, &yes);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.set_nonblocking(fd, yes);
	}
	if (HPMHooks.count.HP_sockt_set_nonblocking_post > 0) {
		void (*postHookFunc) (int fd, unsigned long yes);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_set_nonblocking_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_set_nonblocking_post[hIndex].func;
			postHookFunc(fd, yes);
		}
	}
	return;
}
void HP_sockt_set_defaultparse(ParseFunc defaultparse) {
	int hIndex = 0;
	if (HPMHooks.count.HP_sockt_set_defaultparse_pre > 0) {
		void (*preHookFunc) (ParseFunc *defaultparse);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_set_defaultparse_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_set_defaultparse_pre[hIndex].func;
			preHookFunc(&defaultparse);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.set_defaultparse(defaultparse);
	}
	if (HPMHooks.count.HP_sockt_set_defaultparse_post > 0) {
		void (*postHookFunc) (ParseFunc defaultparse);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_set_defaultparse_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_set_defaultparse_post[hIndex].func;
			postHookFunc(defaultparse);
		}
	}
	return;
}
uint32 HP_sockt_host2ip(const char *hostname) {
	int hIndex = 0;
	uint32 retVal___ = 0;
	if (HPMHooks.count.HP_sockt_host2ip_pre > 0) {
		uint32 (*preHookFunc) (const char **hostname);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_host2ip_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_host2ip_pre[hIndex].func;
			retVal___ = preHookFunc(&hostname);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.host2ip(hostname);
	}
	if (HPMHooks.count.HP_sockt_host2ip_post > 0) {
		uint32 (*postHookFunc) (uint32 retVal___, const char *hostname);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_host2ip_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_host2ip_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, hostname);
		}
	}
	return retVal___;
}
const char* HP_sockt_ip2str(uint32 ip, char *ip_str) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if (HPMHooks.count.HP_sockt_ip2str_pre > 0) {
		const char* (*preHookFunc) (uint32 *ip, char **ip_str);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_ip2str_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_ip2str_pre[hIndex].func;
			retVal___ = preHookFunc(&ip, &ip_str);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.ip2str(ip, ip_str);
	}
	if (HPMHooks.count.HP_sockt_ip2str_post > 0) {
		const char* (*postHookFunc) (const char* retVal___, uint32 ip, char *ip_str);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_ip2str_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_ip2str_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, ip, ip_str);
		}
	}
	return retVal___;
}
uint32 HP_sockt_str2ip(const char *ip_str) {
	int hIndex = 0;
	uint32 retVal___ = 0;
	if (HPMHooks.count.HP_sockt_str2ip_pre > 0) {
		uint32 (*preHookFunc) (const char **ip_str);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_str2ip_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_str2ip_pre[hIndex].func;
			retVal___ = preHookFunc(&ip_str);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.str2ip(ip_str);
	}
	if (HPMHooks.count.HP_sockt_str2ip_post > 0) {
		uint32 (*postHookFunc) (uint32 retVal___, const char *ip_str);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_str2ip_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_str2ip_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, ip_str);
		}
	}
	return retVal___;
}
uint16 HP_sockt_ntows(uint16 netshort) {
	int hIndex = 0;
	uint16 retVal___ = 0;
	if (HPMHooks.count.HP_sockt_ntows_pre > 0) {
		uint16 (*preHookFunc) (uint16 *netshort);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_ntows_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_ntows_pre[hIndex].func;
			retVal___ = preHookFunc(&netshort);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.ntows(netshort);
	}
	if (HPMHooks.count.HP_sockt_ntows_post > 0) {
		uint16 (*postHookFunc) (uint16 retVal___, uint16 netshort);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_ntows_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_ntows_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, netshort);
		}
	}
	return retVal___;
}
int HP_sockt_getips(uint32 *ips, int max) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_sockt_getips_pre > 0) {
		int (*preHookFunc) (uint32 **ips, int *max);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_getips_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_getips_pre[hIndex].func;
			retVal___ = preHookFunc(&ips, &max);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.getips(ips, max);
	}
	if (HPMHooks.count.HP_sockt_getips_post > 0) {
		int (*postHookFunc) (int retVal___, uint32 *ips, int max);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_getips_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_getips_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, ips, max);
		}
	}
	return retVal___;
}
void HP_sockt_eof(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_sockt_eof_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_eof_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_eof_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.eof(fd);
	}
	if (HPMHooks.count.HP_sockt_eof_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_eof_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_eof_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
uint32 HP_sockt_lan_subnet_check(uint32 ip, struct s_subnet *info) {
	int hIndex = 0;
	uint32 retVal___ = 0;
	if (HPMHooks.count.HP_sockt_lan_subnet_check_pre > 0) {
		uint32 (*preHookFunc) (uint32 *ip, struct s_subnet **info);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_lan_subnet_check_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_lan_subnet_check_pre[hIndex].func;
			retVal___ = preHookFunc(&ip, &info);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.lan_subnet_check(ip, info);
	}
	if (HPMHooks.count.HP_sockt_lan_subnet_check_post > 0) {
		uint32 (*postHookFunc) (uint32 retVal___, uint32 ip, struct s_subnet *info);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_lan_subnet_check_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_lan_subnet_check_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, ip, info);
		}
	}
	return retVal___;
}
bool HP_sockt_allowed_ip_check(uint32 ip) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_sockt_allowed_ip_check_pre > 0) {
		bool (*preHookFunc) (uint32 *ip);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_allowed_ip_check_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_allowed_ip_check_pre[hIndex].func;
			retVal___ = preHookFunc(&ip);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.allowed_ip_check(ip);
	}
	if (HPMHooks.count.HP_sockt_allowed_ip_check_post > 0) {
		bool (*postHookFunc) (bool retVal___, uint32 ip);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_allowed_ip_check_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_allowed_ip_check_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, ip);
		}
	}
	return retVal___;
}
bool HP_sockt_trusted_ip_check(uint32 ip) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_sockt_trusted_ip_check_pre > 0) {
		bool (*preHookFunc) (uint32 *ip);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_trusted_ip_check_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_trusted_ip_check_pre[hIndex].func;
			retVal___ = preHookFunc(&ip);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.trusted_ip_check(ip);
	}
	if (HPMHooks.count.HP_sockt_trusted_ip_check_post > 0) {
		bool (*postHookFunc) (bool retVal___, uint32 ip);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_trusted_ip_check_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_trusted_ip_check_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, ip);
		}
	}
	return retVal___;
}
int HP_sockt_net_config_read_sub(struct config_setting_t *t, struct s_subnet_vector *list, const char *filename, const char *groupname) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_sockt_net_config_read_sub_pre > 0) {
		int (*preHookFunc) (struct config_setting_t **t, struct s_subnet_vector **list, const char **filename, const char **groupname);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_net_config_read_sub_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_net_config_read_sub_pre[hIndex].func;
			retVal___ = preHookFunc(&t, &list, &filename, &groupname);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.net_config_read_sub(t, list, filename, groupname);
	}
	if (HPMHooks.count.HP_sockt_net_config_read_sub_post > 0) {
		int (*postHookFunc) (int retVal___, struct config_setting_t *t, struct s_subnet_vector *list, const char *filename, const char *groupname);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_net_config_read_sub_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_net_config_read_sub_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, t, list, filename, groupname);
		}
	}
	return retVal___;
}
void HP_sockt_net_config_read(const char *filename) {
	int hIndex = 0;
	if (HPMHooks.count.HP_sockt_net_config_read_pre > 0) {
		void (*preHookFunc) (const char **filename);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_net_config_read_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_net_config_read_pre[hIndex].func;
			preHookFunc(&filename);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.net_config_read(filename);
	}
	if (HPMHooks.count.HP_sockt_net_config_read_post > 0) {
		void (*postHookFunc) (const char *filename);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_net_config_read_post; hIndex++) {
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
	if (HPMHooks.count.HP_SQL_Connect_pre > 0) {
		int (*preHookFunc) (struct Sql **self, const char **user, const char **passwd, const char **host, uint16 *port, const char **db);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_Connect_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_Connect_pre[hIndex].func;
			retVal___ = preHookFunc(&self, &user, &passwd, &host, &port, &db);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.Connect(self, user, passwd, host, port, db);
	}
	if (HPMHooks.count.HP_SQL_Connect_post > 0) {
		int (*postHookFunc) (int retVal___, struct Sql *self, const char *user, const char *passwd, const char *host, uint16 port, const char *db);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_Connect_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_Connect_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, user, passwd, host, port, db);
		}
	}
	return retVal___;
}
int HP_SQL_GetTimeout(struct Sql *self, uint32 *out_timeout) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_SQL_GetTimeout_pre > 0) {
		int (*preHookFunc) (struct Sql **self, uint32 **out_timeout);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_GetTimeout_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_GetTimeout_pre[hIndex].func;
			retVal___ = preHookFunc(&self, &out_timeout);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.GetTimeout(self, out_timeout);
	}
	if (HPMHooks.count.HP_SQL_GetTimeout_post > 0) {
		int (*postHookFunc) (int retVal___, struct Sql *self, uint32 *out_timeout);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_GetTimeout_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_GetTimeout_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, out_timeout);
		}
	}
	return retVal___;
}
int HP_SQL_GetColumnNames(struct Sql *self, const char *table, char *out_buf, size_t buf_len, char sep) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_SQL_GetColumnNames_pre > 0) {
		int (*preHookFunc) (struct Sql **self, const char **table, char **out_buf, size_t *buf_len, char *sep);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_GetColumnNames_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_GetColumnNames_pre[hIndex].func;
			retVal___ = preHookFunc(&self, &table, &out_buf, &buf_len, &sep);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.GetColumnNames(self, table, out_buf, buf_len, sep);
	}
	if (HPMHooks.count.HP_SQL_GetColumnNames_post > 0) {
		int (*postHookFunc) (int retVal___, struct Sql *self, const char *table, char *out_buf, size_t buf_len, char sep);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_GetColumnNames_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_GetColumnNames_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, table, out_buf, buf_len, sep);
		}
	}
	return retVal___;
}
int HP_SQL_SetEncoding(struct Sql *self, const char *encoding) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_SQL_SetEncoding_pre > 0) {
		int (*preHookFunc) (struct Sql **self, const char **encoding);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_SetEncoding_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_SetEncoding_pre[hIndex].func;
			retVal___ = preHookFunc(&self, &encoding);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.SetEncoding(self, encoding);
	}
	if (HPMHooks.count.HP_SQL_SetEncoding_post > 0) {
		int (*postHookFunc) (int retVal___, struct Sql *self, const char *encoding);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_SetEncoding_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_SetEncoding_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, encoding);
		}
	}
	return retVal___;
}
int HP_SQL_Ping(struct Sql *self) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_SQL_Ping_pre > 0) {
		int (*preHookFunc) (struct Sql **self);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_Ping_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_Ping_pre[hIndex].func;
			retVal___ = preHookFunc(&self);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.Ping(self);
	}
	if (HPMHooks.count.HP_SQL_Ping_post > 0) {
		int (*postHookFunc) (int retVal___, struct Sql *self);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_Ping_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_Ping_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
size_t HP_SQL_EscapeString(struct Sql *self, char *out_to, const char *from) {
	int hIndex = 0;
	size_t retVal___ = 0;
	if (HPMHooks.count.HP_SQL_EscapeString_pre > 0) {
		size_t (*preHookFunc) (struct Sql **self, char **out_to, const char **from);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_EscapeString_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_EscapeString_pre[hIndex].func;
			retVal___ = preHookFunc(&self, &out_to, &from);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.EscapeString(self, out_to, from);
	}
	if (HPMHooks.count.HP_SQL_EscapeString_post > 0) {
		size_t (*postHookFunc) (size_t retVal___, struct Sql *self, char *out_to, const char *from);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_EscapeString_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_EscapeString_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, out_to, from);
		}
	}
	return retVal___;
}
size_t HP_SQL_EscapeStringLen(struct Sql *self, char *out_to, const char *from, size_t from_len) {
	int hIndex = 0;
	size_t retVal___ = 0;
	if (HPMHooks.count.HP_SQL_EscapeStringLen_pre > 0) {
		size_t (*preHookFunc) (struct Sql **self, char **out_to, const char **from, size_t *from_len);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_EscapeStringLen_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_EscapeStringLen_pre[hIndex].func;
			retVal___ = preHookFunc(&self, &out_to, &from, &from_len);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.EscapeStringLen(self, out_to, from, from_len);
	}
	if (HPMHooks.count.HP_SQL_EscapeStringLen_post > 0) {
		size_t (*postHookFunc) (size_t retVal___, struct Sql *self, char *out_to, const char *from, size_t from_len);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_EscapeStringLen_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_EscapeStringLen_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, out_to, from, from_len);
		}
	}
	return retVal___;
}
int HP_SQL_QueryV(struct Sql *self, const char *query, va_list args) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_SQL_QueryV_pre > 0) {
		int (*preHookFunc) (struct Sql **self, const char **query, va_list args);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_QueryV_pre; hIndex++) {
			va_list args___copy; va_copy(args___copy, args);
			preHookFunc = HPMHooks.list.HP_SQL_QueryV_pre[hIndex].func;
			retVal___ = preHookFunc(&self, &query, args___copy);
			va_end(args___copy);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		va_list args___copy; va_copy(args___copy, args);
		retVal___ = HPMHooks.source.SQL.QueryV(self, query, args___copy);
		va_end(args___copy);
	}
	if (HPMHooks.count.HP_SQL_QueryV_post > 0) {
		int (*postHookFunc) (int retVal___, struct Sql *self, const char *query, va_list args);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_QueryV_post; hIndex++) {
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
	if (HPMHooks.count.HP_SQL_QueryStr_pre > 0) {
		int (*preHookFunc) (struct Sql **self, const char **query);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_QueryStr_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_QueryStr_pre[hIndex].func;
			retVal___ = preHookFunc(&self, &query);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.QueryStr(self, query);
	}
	if (HPMHooks.count.HP_SQL_QueryStr_post > 0) {
		int (*postHookFunc) (int retVal___, struct Sql *self, const char *query);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_QueryStr_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_QueryStr_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, query);
		}
	}
	return retVal___;
}
uint64 HP_SQL_LastInsertId(struct Sql *self) {
	int hIndex = 0;
	uint64 retVal___ = 0;
	if (HPMHooks.count.HP_SQL_LastInsertId_pre > 0) {
		uint64 (*preHookFunc) (struct Sql **self);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_LastInsertId_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_LastInsertId_pre[hIndex].func;
			retVal___ = preHookFunc(&self);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.LastInsertId(self);
	}
	if (HPMHooks.count.HP_SQL_LastInsertId_post > 0) {
		uint64 (*postHookFunc) (uint64 retVal___, struct Sql *self);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_LastInsertId_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_LastInsertId_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
uint32 HP_SQL_NumColumns(struct Sql *self) {
	int hIndex = 0;
	uint32 retVal___ = 0;
	if (HPMHooks.count.HP_SQL_NumColumns_pre > 0) {
		uint32 (*preHookFunc) (struct Sql **self);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_NumColumns_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_NumColumns_pre[hIndex].func;
			retVal___ = preHookFunc(&self);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.NumColumns(self);
	}
	if (HPMHooks.count.HP_SQL_NumColumns_post > 0) {
		uint32 (*postHookFunc) (uint32 retVal___, struct Sql *self);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_NumColumns_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_NumColumns_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
uint64 HP_SQL_NumRows(struct Sql *self) {
	int hIndex = 0;
	uint64 retVal___ = 0;
	if (HPMHooks.count.HP_SQL_NumRows_pre > 0) {
		uint64 (*preHookFunc) (struct Sql **self);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_NumRows_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_NumRows_pre[hIndex].func;
			retVal___ = preHookFunc(&self);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.NumRows(self);
	}
	if (HPMHooks.count.HP_SQL_NumRows_post > 0) {
		uint64 (*postHookFunc) (uint64 retVal___, struct Sql *self);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_NumRows_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_NumRows_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
int HP_SQL_NextRow(struct Sql *self) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_SQL_NextRow_pre > 0) {
		int (*preHookFunc) (struct Sql **self);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_NextRow_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_NextRow_pre[hIndex].func;
			retVal___ = preHookFunc(&self);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.NextRow(self);
	}
	if (HPMHooks.count.HP_SQL_NextRow_post > 0) {
		int (*postHookFunc) (int retVal___, struct Sql *self);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_NextRow_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_NextRow_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
int HP_SQL_GetData(struct Sql *self, size_t col, char **out_buf, size_t *out_len) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_SQL_GetData_pre > 0) {
		int (*preHookFunc) (struct Sql **self, size_t *col, char ***out_buf, size_t **out_len);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_GetData_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_GetData_pre[hIndex].func;
			retVal___ = preHookFunc(&self, &col, &out_buf, &out_len);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.GetData(self, col, out_buf, out_len);
	}
	if (HPMHooks.count.HP_SQL_GetData_post > 0) {
		int (*postHookFunc) (int retVal___, struct Sql *self, size_t col, char **out_buf, size_t *out_len);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_GetData_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_GetData_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, col, out_buf, out_len);
		}
	}
	return retVal___;
}
void HP_SQL_FreeResult(struct Sql *self) {
	int hIndex = 0;
	if (HPMHooks.count.HP_SQL_FreeResult_pre > 0) {
		void (*preHookFunc) (struct Sql **self);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_FreeResult_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_FreeResult_pre[hIndex].func;
			preHookFunc(&self);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.SQL.FreeResult(self);
	}
	if (HPMHooks.count.HP_SQL_FreeResult_post > 0) {
		void (*postHookFunc) (struct Sql *self);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_FreeResult_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_FreeResult_post[hIndex].func;
			postHookFunc(self);
		}
	}
	return;
}
void HP_SQL_ShowDebug_(struct Sql *self, const char *debug_file, const unsigned long debug_line) {
	int hIndex = 0;
	if (HPMHooks.count.HP_SQL_ShowDebug__pre > 0) {
		void (*preHookFunc) (struct Sql **self, const char **debug_file, const unsigned long *debug_line);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_ShowDebug__pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_ShowDebug__pre[hIndex].func;
			preHookFunc(&self, &debug_file, &debug_line);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.SQL.ShowDebug_(self, debug_file, debug_line);
	}
	if (HPMHooks.count.HP_SQL_ShowDebug__post > 0) {
		void (*postHookFunc) (struct Sql *self, const char *debug_file, const unsigned long debug_line);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_ShowDebug__post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_ShowDebug__post[hIndex].func;
			postHookFunc(self, debug_file, debug_line);
		}
	}
	return;
}
void HP_SQL_Free(struct Sql *self) {
	int hIndex = 0;
	if (HPMHooks.count.HP_SQL_Free_pre > 0) {
		void (*preHookFunc) (struct Sql **self);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_Free_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_Free_pre[hIndex].func;
			preHookFunc(&self);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.SQL.Free(self);
	}
	if (HPMHooks.count.HP_SQL_Free_post > 0) {
		void (*postHookFunc) (struct Sql *self);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_Free_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_Free_post[hIndex].func;
			postHookFunc(self);
		}
	}
	return;
}
struct Sql* HP_SQL_Malloc(void) {
	int hIndex = 0;
	struct Sql* retVal___ = NULL;
	if (HPMHooks.count.HP_SQL_Malloc_pre > 0) {
		struct Sql* (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_Malloc_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_Malloc_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.Malloc();
	}
	if (HPMHooks.count.HP_SQL_Malloc_post > 0) {
		struct Sql* (*postHookFunc) (struct Sql* retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_Malloc_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_Malloc_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
struct SqlStmt* HP_SQL_StmtMalloc(struct Sql *sql) {
	int hIndex = 0;
	struct SqlStmt* retVal___ = NULL;
	if (HPMHooks.count.HP_SQL_StmtMalloc_pre > 0) {
		struct SqlStmt* (*preHookFunc) (struct Sql **sql);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtMalloc_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtMalloc_pre[hIndex].func;
			retVal___ = preHookFunc(&sql);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.StmtMalloc(sql);
	}
	if (HPMHooks.count.HP_SQL_StmtMalloc_post > 0) {
		struct SqlStmt* (*postHookFunc) (struct SqlStmt* retVal___, struct Sql *sql);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtMalloc_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtMalloc_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, sql);
		}
	}
	return retVal___;
}
int HP_SQL_StmtPrepareV(struct SqlStmt *self, const char *query, va_list args) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_SQL_StmtPrepareV_pre > 0) {
		int (*preHookFunc) (struct SqlStmt **self, const char **query, va_list args);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtPrepareV_pre; hIndex++) {
			va_list args___copy; va_copy(args___copy, args);
			preHookFunc = HPMHooks.list.HP_SQL_StmtPrepareV_pre[hIndex].func;
			retVal___ = preHookFunc(&self, &query, args___copy);
			va_end(args___copy);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		va_list args___copy; va_copy(args___copy, args);
		retVal___ = HPMHooks.source.SQL.StmtPrepareV(self, query, args___copy);
		va_end(args___copy);
	}
	if (HPMHooks.count.HP_SQL_StmtPrepareV_post > 0) {
		int (*postHookFunc) (int retVal___, struct SqlStmt *self, const char *query, va_list args);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtPrepareV_post; hIndex++) {
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
	if (HPMHooks.count.HP_SQL_StmtPrepareStr_pre > 0) {
		int (*preHookFunc) (struct SqlStmt **self, const char **query);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtPrepareStr_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtPrepareStr_pre[hIndex].func;
			retVal___ = preHookFunc(&self, &query);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.StmtPrepareStr(self, query);
	}
	if (HPMHooks.count.HP_SQL_StmtPrepareStr_post > 0) {
		int (*postHookFunc) (int retVal___, struct SqlStmt *self, const char *query);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtPrepareStr_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtPrepareStr_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, query);
		}
	}
	return retVal___;
}
size_t HP_SQL_StmtNumParams(struct SqlStmt *self) {
	int hIndex = 0;
	size_t retVal___ = 0;
	if (HPMHooks.count.HP_SQL_StmtNumParams_pre > 0) {
		size_t (*preHookFunc) (struct SqlStmt **self);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtNumParams_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtNumParams_pre[hIndex].func;
			retVal___ = preHookFunc(&self);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.StmtNumParams(self);
	}
	if (HPMHooks.count.HP_SQL_StmtNumParams_post > 0) {
		size_t (*postHookFunc) (size_t retVal___, struct SqlStmt *self);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtNumParams_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtNumParams_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
int HP_SQL_StmtBindParam(struct SqlStmt *self, size_t idx, enum SqlDataType buffer_type, const void *buffer, size_t buffer_len) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_SQL_StmtBindParam_pre > 0) {
		int (*preHookFunc) (struct SqlStmt **self, size_t *idx, enum SqlDataType *buffer_type, const void **buffer, size_t *buffer_len);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtBindParam_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtBindParam_pre[hIndex].func;
			retVal___ = preHookFunc(&self, &idx, &buffer_type, &buffer, &buffer_len);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.StmtBindParam(self, idx, buffer_type, buffer, buffer_len);
	}
	if (HPMHooks.count.HP_SQL_StmtBindParam_post > 0) {
		int (*postHookFunc) (int retVal___, struct SqlStmt *self, size_t idx, enum SqlDataType buffer_type, const void *buffer, size_t buffer_len);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtBindParam_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtBindParam_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, idx, buffer_type, buffer, buffer_len);
		}
	}
	return retVal___;
}
int HP_SQL_StmtExecute(struct SqlStmt *self) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_SQL_StmtExecute_pre > 0) {
		int (*preHookFunc) (struct SqlStmt **self);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtExecute_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtExecute_pre[hIndex].func;
			retVal___ = preHookFunc(&self);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.StmtExecute(self);
	}
	if (HPMHooks.count.HP_SQL_StmtExecute_post > 0) {
		int (*postHookFunc) (int retVal___, struct SqlStmt *self);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtExecute_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtExecute_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
uint64 HP_SQL_StmtLastInsertId(struct SqlStmt *self) {
	int hIndex = 0;
	uint64 retVal___ = 0;
	if (HPMHooks.count.HP_SQL_StmtLastInsertId_pre > 0) {
		uint64 (*preHookFunc) (struct SqlStmt **self);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtLastInsertId_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtLastInsertId_pre[hIndex].func;
			retVal___ = preHookFunc(&self);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.StmtLastInsertId(self);
	}
	if (HPMHooks.count.HP_SQL_StmtLastInsertId_post > 0) {
		uint64 (*postHookFunc) (uint64 retVal___, struct SqlStmt *self);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtLastInsertId_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtLastInsertId_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
size_t HP_SQL_StmtNumColumns(struct SqlStmt *self) {
	int hIndex = 0;
	size_t retVal___ = 0;
	if (HPMHooks.count.HP_SQL_StmtNumColumns_pre > 0) {
		size_t (*preHookFunc) (struct SqlStmt **self);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtNumColumns_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtNumColumns_pre[hIndex].func;
			retVal___ = preHookFunc(&self);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.StmtNumColumns(self);
	}
	if (HPMHooks.count.HP_SQL_StmtNumColumns_post > 0) {
		size_t (*postHookFunc) (size_t retVal___, struct SqlStmt *self);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtNumColumns_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtNumColumns_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
int HP_SQL_StmtBindColumn(struct SqlStmt *self, size_t idx, enum SqlDataType buffer_type, void *buffer, size_t buffer_len, uint32 *out_length, int8 *out_is_null) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_SQL_StmtBindColumn_pre > 0) {
		int (*preHookFunc) (struct SqlStmt **self, size_t *idx, enum SqlDataType *buffer_type, void **buffer, size_t *buffer_len, uint32 **out_length, int8 **out_is_null);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtBindColumn_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtBindColumn_pre[hIndex].func;
			retVal___ = preHookFunc(&self, &idx, &buffer_type, &buffer, &buffer_len, &out_length, &out_is_null);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.StmtBindColumn(self, idx, buffer_type, buffer, buffer_len, out_length, out_is_null);
	}
	if (HPMHooks.count.HP_SQL_StmtBindColumn_post > 0) {
		int (*postHookFunc) (int retVal___, struct SqlStmt *self, size_t idx, enum SqlDataType buffer_type, void *buffer, size_t buffer_len, uint32 *out_length, int8 *out_is_null);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtBindColumn_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtBindColumn_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, idx, buffer_type, buffer, buffer_len, out_length, out_is_null);
		}
	}
	return retVal___;
}
uint64 HP_SQL_StmtNumRows(struct SqlStmt *self) {
	int hIndex = 0;
	uint64 retVal___ = 0;
	if (HPMHooks.count.HP_SQL_StmtNumRows_pre > 0) {
		uint64 (*preHookFunc) (struct SqlStmt **self);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtNumRows_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtNumRows_pre[hIndex].func;
			retVal___ = preHookFunc(&self);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.StmtNumRows(self);
	}
	if (HPMHooks.count.HP_SQL_StmtNumRows_post > 0) {
		uint64 (*postHookFunc) (uint64 retVal___, struct SqlStmt *self);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtNumRows_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtNumRows_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
int HP_SQL_StmtNextRow(struct SqlStmt *self) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_SQL_StmtNextRow_pre > 0) {
		int (*preHookFunc) (struct SqlStmt **self);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtNextRow_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtNextRow_pre[hIndex].func;
			retVal___ = preHookFunc(&self);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.StmtNextRow(self);
	}
	if (HPMHooks.count.HP_SQL_StmtNextRow_post > 0) {
		int (*postHookFunc) (int retVal___, struct SqlStmt *self);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtNextRow_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtNextRow_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
void HP_SQL_StmtFreeResult(struct SqlStmt *self) {
	int hIndex = 0;
	if (HPMHooks.count.HP_SQL_StmtFreeResult_pre > 0) {
		void (*preHookFunc) (struct SqlStmt **self);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtFreeResult_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtFreeResult_pre[hIndex].func;
			preHookFunc(&self);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.SQL.StmtFreeResult(self);
	}
	if (HPMHooks.count.HP_SQL_StmtFreeResult_post > 0) {
		void (*postHookFunc) (struct SqlStmt *self);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtFreeResult_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtFreeResult_post[hIndex].func;
			postHookFunc(self);
		}
	}
	return;
}
void HP_SQL_StmtFree(struct SqlStmt *self) {
	int hIndex = 0;
	if (HPMHooks.count.HP_SQL_StmtFree_pre > 0) {
		void (*preHookFunc) (struct SqlStmt **self);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtFree_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtFree_pre[hIndex].func;
			preHookFunc(&self);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.SQL.StmtFree(self);
	}
	if (HPMHooks.count.HP_SQL_StmtFree_post > 0) {
		void (*postHookFunc) (struct SqlStmt *self);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtFree_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtFree_post[hIndex].func;
			postHookFunc(self);
		}
	}
	return;
}
void HP_SQL_StmtShowDebug_(struct SqlStmt *self, const char *debug_file, const unsigned long debug_line) {
	int hIndex = 0;
	if (HPMHooks.count.HP_SQL_StmtShowDebug__pre > 0) {
		void (*preHookFunc) (struct SqlStmt **self, const char **debug_file, const unsigned long *debug_line);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtShowDebug__pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_StmtShowDebug__pre[hIndex].func;
			preHookFunc(&self, &debug_file, &debug_line);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.SQL.StmtShowDebug_(self, debug_file, debug_line);
	}
	if (HPMHooks.count.HP_SQL_StmtShowDebug__post > 0) {
		void (*postHookFunc) (struct SqlStmt *self, const char *debug_file, const unsigned long debug_line);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_StmtShowDebug__post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_StmtShowDebug__post[hIndex].func;
			postHookFunc(self, debug_file, debug_line);
		}
	}
	return;
}
/* stringbuf_interface */
StringBuf* HP_StrBuf_Malloc(void) {
	int hIndex = 0;
	StringBuf* retVal___ = NULL;
	if (HPMHooks.count.HP_StrBuf_Malloc_pre > 0) {
		StringBuf* (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Malloc_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_StrBuf_Malloc_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.StrBuf.Malloc();
	}
	if (HPMHooks.count.HP_StrBuf_Malloc_post > 0) {
		StringBuf* (*postHookFunc) (StringBuf* retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Malloc_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_StrBuf_Malloc_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_StrBuf_Init(StringBuf *self) {
	int hIndex = 0;
	if (HPMHooks.count.HP_StrBuf_Init_pre > 0) {
		void (*preHookFunc) (StringBuf **self);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_StrBuf_Init_pre[hIndex].func;
			preHookFunc(&self);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.StrBuf.Init(self);
	}
	if (HPMHooks.count.HP_StrBuf_Init_post > 0) {
		void (*postHookFunc) (StringBuf *self);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_StrBuf_Init_post[hIndex].func;
			postHookFunc(self);
		}
	}
	return;
}
int HP_StrBuf_Vprintf(StringBuf *self, const char *fmt, va_list args) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_StrBuf_Vprintf_pre > 0) {
		int (*preHookFunc) (StringBuf **self, const char **fmt, va_list args);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Vprintf_pre; hIndex++) {
			va_list args___copy; va_copy(args___copy, args);
			preHookFunc = HPMHooks.list.HP_StrBuf_Vprintf_pre[hIndex].func;
			retVal___ = preHookFunc(&self, &fmt, args___copy);
			va_end(args___copy);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		va_list args___copy; va_copy(args___copy, args);
		retVal___ = HPMHooks.source.StrBuf.Vprintf(self, fmt, args___copy);
		va_end(args___copy);
	}
	if (HPMHooks.count.HP_StrBuf_Vprintf_post > 0) {
		int (*postHookFunc) (int retVal___, StringBuf *self, const char *fmt, va_list args);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Vprintf_post; hIndex++) {
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
	if (HPMHooks.count.HP_StrBuf_Append_pre > 0) {
		int (*preHookFunc) (StringBuf **self, const StringBuf **sbuf);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Append_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_StrBuf_Append_pre[hIndex].func;
			retVal___ = preHookFunc(&self, &sbuf);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.StrBuf.Append(self, sbuf);
	}
	if (HPMHooks.count.HP_StrBuf_Append_post > 0) {
		int (*postHookFunc) (int retVal___, StringBuf *self, const StringBuf *sbuf);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Append_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_StrBuf_Append_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, sbuf);
		}
	}
	return retVal___;
}
int HP_StrBuf_AppendStr(StringBuf *self, const char *str) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_StrBuf_AppendStr_pre > 0) {
		int (*preHookFunc) (StringBuf **self, const char **str);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_AppendStr_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_StrBuf_AppendStr_pre[hIndex].func;
			retVal___ = preHookFunc(&self, &str);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.StrBuf.AppendStr(self, str);
	}
	if (HPMHooks.count.HP_StrBuf_AppendStr_post > 0) {
		int (*postHookFunc) (int retVal___, StringBuf *self, const char *str);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_AppendStr_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_StrBuf_AppendStr_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self, str);
		}
	}
	return retVal___;
}
int HP_StrBuf_Length(StringBuf *self) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_StrBuf_Length_pre > 0) {
		int (*preHookFunc) (StringBuf **self);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Length_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_StrBuf_Length_pre[hIndex].func;
			retVal___ = preHookFunc(&self);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.StrBuf.Length(self);
	}
	if (HPMHooks.count.HP_StrBuf_Length_post > 0) {
		int (*postHookFunc) (int retVal___, StringBuf *self);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Length_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_StrBuf_Length_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
char* HP_StrBuf_Value(StringBuf *self) {
	int hIndex = 0;
	char* retVal___ = NULL;
	if (HPMHooks.count.HP_StrBuf_Value_pre > 0) {
		char* (*preHookFunc) (StringBuf **self);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Value_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_StrBuf_Value_pre[hIndex].func;
			retVal___ = preHookFunc(&self);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.StrBuf.Value(self);
	}
	if (HPMHooks.count.HP_StrBuf_Value_post > 0) {
		char* (*postHookFunc) (char* retVal___, StringBuf *self);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Value_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_StrBuf_Value_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, self);
		}
	}
	return retVal___;
}
void HP_StrBuf_Clear(StringBuf *self) {
	int hIndex = 0;
	if (HPMHooks.count.HP_StrBuf_Clear_pre > 0) {
		void (*preHookFunc) (StringBuf **self);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Clear_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_StrBuf_Clear_pre[hIndex].func;
			preHookFunc(&self);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.StrBuf.Clear(self);
	}
	if (HPMHooks.count.HP_StrBuf_Clear_post > 0) {
		void (*postHookFunc) (StringBuf *self);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Clear_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_StrBuf_Clear_post[hIndex].func;
			postHookFunc(self);
		}
	}
	return;
}
void HP_StrBuf_Destroy(StringBuf *self) {
	int hIndex = 0;
	if (HPMHooks.count.HP_StrBuf_Destroy_pre > 0) {
		void (*preHookFunc) (StringBuf **self);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Destroy_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_StrBuf_Destroy_pre[hIndex].func;
			preHookFunc(&self);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.StrBuf.Destroy(self);
	}
	if (HPMHooks.count.HP_StrBuf_Destroy_post > 0) {
		void (*postHookFunc) (StringBuf *self);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Destroy_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_StrBuf_Destroy_post[hIndex].func;
			postHookFunc(self);
		}
	}
	return;
}
void HP_StrBuf_Free(StringBuf *self) {
	int hIndex = 0;
	if (HPMHooks.count.HP_StrBuf_Free_pre > 0) {
		void (*preHookFunc) (StringBuf **self);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Free_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_StrBuf_Free_pre[hIndex].func;
			preHookFunc(&self);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.StrBuf.Free(self);
	}
	if (HPMHooks.count.HP_StrBuf_Free_post > 0) {
		void (*postHookFunc) (StringBuf *self);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_StrBuf_Free_post; hIndex++) {
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
	if (HPMHooks.count.HP_strlib_jstrescape_pre > 0) {
		char* (*preHookFunc) (char **pt);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_jstrescape_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_strlib_jstrescape_pre[hIndex].func;
			retVal___ = preHookFunc(&pt);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.jstrescape(pt);
	}
	if (HPMHooks.count.HP_strlib_jstrescape_post > 0) {
		char* (*postHookFunc) (char* retVal___, char *pt);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_jstrescape_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_strlib_jstrescape_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, pt);
		}
	}
	return retVal___;
}
char* HP_strlib_jstrescapecpy(char *pt, const char *spt) {
	int hIndex = 0;
	char* retVal___ = NULL;
	if (HPMHooks.count.HP_strlib_jstrescapecpy_pre > 0) {
		char* (*preHookFunc) (char **pt, const char **spt);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_jstrescapecpy_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_strlib_jstrescapecpy_pre[hIndex].func;
			retVal___ = preHookFunc(&pt, &spt);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.jstrescapecpy(pt, spt);
	}
	if (HPMHooks.count.HP_strlib_jstrescapecpy_post > 0) {
		char* (*postHookFunc) (char* retVal___, char *pt, const char *spt);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_jstrescapecpy_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_strlib_jstrescapecpy_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, pt, spt);
		}
	}
	return retVal___;
}
int HP_strlib_jmemescapecpy(char *pt, const char *spt, int size) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_strlib_jmemescapecpy_pre > 0) {
		int (*preHookFunc) (char **pt, const char **spt, int *size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_jmemescapecpy_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_strlib_jmemescapecpy_pre[hIndex].func;
			retVal___ = preHookFunc(&pt, &spt, &size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.jmemescapecpy(pt, spt, size);
	}
	if (HPMHooks.count.HP_strlib_jmemescapecpy_post > 0) {
		int (*postHookFunc) (int retVal___, char *pt, const char *spt, int size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_jmemescapecpy_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_strlib_jmemescapecpy_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, pt, spt, size);
		}
	}
	return retVal___;
}
int HP_strlib_remove_control_chars_(char *str) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_strlib_remove_control_chars__pre > 0) {
		int (*preHookFunc) (char **str);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_remove_control_chars__pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_strlib_remove_control_chars__pre[hIndex].func;
			retVal___ = preHookFunc(&str);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.remove_control_chars_(str);
	}
	if (HPMHooks.count.HP_strlib_remove_control_chars__post > 0) {
		int (*postHookFunc) (int retVal___, char *str);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_remove_control_chars__post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_strlib_remove_control_chars__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, str);
		}
	}
	return retVal___;
}
char* HP_strlib_trim_(char *str) {
	int hIndex = 0;
	char* retVal___ = NULL;
	if (HPMHooks.count.HP_strlib_trim__pre > 0) {
		char* (*preHookFunc) (char **str);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_trim__pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_strlib_trim__pre[hIndex].func;
			retVal___ = preHookFunc(&str);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.trim_(str);
	}
	if (HPMHooks.count.HP_strlib_trim__post > 0) {
		char* (*postHookFunc) (char* retVal___, char *str);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_trim__post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_strlib_trim__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, str);
		}
	}
	return retVal___;
}
char* HP_strlib_normalize_name_(char *str, const char *delims) {
	int hIndex = 0;
	char* retVal___ = NULL;
	if (HPMHooks.count.HP_strlib_normalize_name__pre > 0) {
		char* (*preHookFunc) (char **str, const char **delims);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_normalize_name__pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_strlib_normalize_name__pre[hIndex].func;
			retVal___ = preHookFunc(&str, &delims);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.normalize_name_(str, delims);
	}
	if (HPMHooks.count.HP_strlib_normalize_name__post > 0) {
		char* (*postHookFunc) (char* retVal___, char *str, const char *delims);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_normalize_name__post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_strlib_normalize_name__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, str, delims);
		}
	}
	return retVal___;
}
const char* HP_strlib_stristr_(const char *haystack, const char *needle) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if (HPMHooks.count.HP_strlib_stristr__pre > 0) {
		const char* (*preHookFunc) (const char **haystack, const char **needle);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_stristr__pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_strlib_stristr__pre[hIndex].func;
			retVal___ = preHookFunc(&haystack, &needle);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.stristr_(haystack, needle);
	}
	if (HPMHooks.count.HP_strlib_stristr__post > 0) {
		const char* (*postHookFunc) (const char* retVal___, const char *haystack, const char *needle);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_stristr__post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_strlib_stristr__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, haystack, needle);
		}
	}
	return retVal___;
}
size_t HP_strlib_strnlen_(const char *string, size_t maxlen) {
	int hIndex = 0;
	size_t retVal___ = 0;
	if (HPMHooks.count.HP_strlib_strnlen__pre > 0) {
		size_t (*preHookFunc) (const char **string, size_t *maxlen);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_strnlen__pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_strlib_strnlen__pre[hIndex].func;
			retVal___ = preHookFunc(&string, &maxlen);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.strnlen_(string, maxlen);
	}
	if (HPMHooks.count.HP_strlib_strnlen__post > 0) {
		size_t (*postHookFunc) (size_t retVal___, const char *string, size_t maxlen);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_strnlen__post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_strlib_strnlen__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, string, maxlen);
		}
	}
	return retVal___;
}
char* HP_strlib_strtok_r_(char *s1, const char *s2, char **lasts) {
	int hIndex = 0;
	char* retVal___ = NULL;
	if (HPMHooks.count.HP_strlib_strtok_r__pre > 0) {
		char* (*preHookFunc) (char **s1, const char **s2, char ***lasts);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_strtok_r__pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_strlib_strtok_r__pre[hIndex].func;
			retVal___ = preHookFunc(&s1, &s2, &lasts);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.strtok_r_(s1, s2, lasts);
	}
	if (HPMHooks.count.HP_strlib_strtok_r__post > 0) {
		char* (*postHookFunc) (char* retVal___, char *s1, const char *s2, char **lasts);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_strtok_r__post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_strlib_strtok_r__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, s1, s2, lasts);
		}
	}
	return retVal___;
}
int HP_strlib_e_mail_check_(char *email) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_strlib_e_mail_check__pre > 0) {
		int (*preHookFunc) (char **email);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_e_mail_check__pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_strlib_e_mail_check__pre[hIndex].func;
			retVal___ = preHookFunc(&email);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.e_mail_check_(email);
	}
	if (HPMHooks.count.HP_strlib_e_mail_check__post > 0) {
		int (*postHookFunc) (int retVal___, char *email);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_e_mail_check__post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_strlib_e_mail_check__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, email);
		}
	}
	return retVal___;
}
int HP_strlib_config_switch_(const char *str) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_strlib_config_switch__pre > 0) {
		int (*preHookFunc) (const char **str);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_config_switch__pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_strlib_config_switch__pre[hIndex].func;
			retVal___ = preHookFunc(&str);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.config_switch_(str);
	}
	if (HPMHooks.count.HP_strlib_config_switch__post > 0) {
		int (*postHookFunc) (int retVal___, const char *str);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_config_switch__post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_strlib_config_switch__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, str);
		}
	}
	return retVal___;
}
char* HP_strlib_safestrncpy_(char *dst, const char *src, size_t n) {
	int hIndex = 0;
	char* retVal___ = NULL;
	if (HPMHooks.count.HP_strlib_safestrncpy__pre > 0) {
		char* (*preHookFunc) (char **dst, const char **src, size_t *n);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_safestrncpy__pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_strlib_safestrncpy__pre[hIndex].func;
			retVal___ = preHookFunc(&dst, &src, &n);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.safestrncpy_(dst, src, n);
	}
	if (HPMHooks.count.HP_strlib_safestrncpy__post > 0) {
		char* (*postHookFunc) (char* retVal___, char *dst, const char *src, size_t n);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_safestrncpy__post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_strlib_safestrncpy__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, dst, src, n);
		}
	}
	return retVal___;
}
size_t HP_strlib_safestrnlen_(const char *string, size_t maxlen) {
	int hIndex = 0;
	size_t retVal___ = 0;
	if (HPMHooks.count.HP_strlib_safestrnlen__pre > 0) {
		size_t (*preHookFunc) (const char **string, size_t *maxlen);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_safestrnlen__pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_strlib_safestrnlen__pre[hIndex].func;
			retVal___ = preHookFunc(&string, &maxlen);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.safestrnlen_(string, maxlen);
	}
	if (HPMHooks.count.HP_strlib_safestrnlen__post > 0) {
		size_t (*postHookFunc) (size_t retVal___, const char *string, size_t maxlen);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_safestrnlen__post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_strlib_safestrnlen__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, string, maxlen);
		}
	}
	return retVal___;
}
int HP_strlib_strline_(const char *str, size_t pos) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_strlib_strline__pre > 0) {
		int (*preHookFunc) (const char **str, size_t *pos);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_strline__pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_strlib_strline__pre[hIndex].func;
			retVal___ = preHookFunc(&str, &pos);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.strline_(str, pos);
	}
	if (HPMHooks.count.HP_strlib_strline__post > 0) {
		int (*postHookFunc) (int retVal___, const char *str, size_t pos);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_strline__post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_strlib_strline__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, str, pos);
		}
	}
	return retVal___;
}
bool HP_strlib_bin2hex_(char *output, const unsigned char *input, size_t count) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_strlib_bin2hex__pre > 0) {
		bool (*preHookFunc) (char **output, const unsigned char **input, size_t *count);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_bin2hex__pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_strlib_bin2hex__pre[hIndex].func;
			retVal___ = preHookFunc(&output, &input, &count);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.strlib.bin2hex_(output, input, count);
	}
	if (HPMHooks.count.HP_strlib_bin2hex__post > 0) {
		bool (*postHookFunc) (bool retVal___, char *output, const unsigned char *input, size_t count);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_strlib_bin2hex__post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_strlib_bin2hex__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, output, input, count);
		}
	}
	return retVal___;
}
/* sv_interface */
int HP_sv_parse_next(struct s_svstate *svstate) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_sv_parse_next_pre > 0) {
		int (*preHookFunc) (struct s_svstate **svstate);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sv_parse_next_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sv_parse_next_pre[hIndex].func;
			retVal___ = preHookFunc(&svstate);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sv.parse_next(svstate);
	}
	if (HPMHooks.count.HP_sv_parse_next_post > 0) {
		int (*postHookFunc) (int retVal___, struct s_svstate *svstate);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sv_parse_next_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sv_parse_next_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, svstate);
		}
	}
	return retVal___;
}
int HP_sv_parse(const char *str, int len, int startoff, char delim, int *out_pos, int npos, enum e_svopt opt) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_sv_parse_pre > 0) {
		int (*preHookFunc) (const char **str, int *len, int *startoff, char *delim, int **out_pos, int *npos, enum e_svopt *opt);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sv_parse_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sv_parse_pre[hIndex].func;
			retVal___ = preHookFunc(&str, &len, &startoff, &delim, &out_pos, &npos, &opt);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sv.parse(str, len, startoff, delim, out_pos, npos, opt);
	}
	if (HPMHooks.count.HP_sv_parse_post > 0) {
		int (*postHookFunc) (int retVal___, const char *str, int len, int startoff, char delim, int *out_pos, int npos, enum e_svopt opt);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sv_parse_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sv_parse_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, str, len, startoff, delim, out_pos, npos, opt);
		}
	}
	return retVal___;
}
int HP_sv_split(char *str, int len, int startoff, char delim, char **out_fields, int nfields, enum e_svopt opt) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_sv_split_pre > 0) {
		int (*preHookFunc) (char **str, int *len, int *startoff, char *delim, char ***out_fields, int *nfields, enum e_svopt *opt);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sv_split_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sv_split_pre[hIndex].func;
			retVal___ = preHookFunc(&str, &len, &startoff, &delim, &out_fields, &nfields, &opt);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sv.split(str, len, startoff, delim, out_fields, nfields, opt);
	}
	if (HPMHooks.count.HP_sv_split_post > 0) {
		int (*postHookFunc) (int retVal___, char *str, int len, int startoff, char delim, char **out_fields, int nfields, enum e_svopt opt);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sv_split_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sv_split_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, str, len, startoff, delim, out_fields, nfields, opt);
		}
	}
	return retVal___;
}
size_t HP_sv_escape_c(char *out_dest, const char *src, size_t len, const char *escapes) {
	int hIndex = 0;
	size_t retVal___ = 0;
	if (HPMHooks.count.HP_sv_escape_c_pre > 0) {
		size_t (*preHookFunc) (char **out_dest, const char **src, size_t *len, const char **escapes);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sv_escape_c_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sv_escape_c_pre[hIndex].func;
			retVal___ = preHookFunc(&out_dest, &src, &len, &escapes);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sv.escape_c(out_dest, src, len, escapes);
	}
	if (HPMHooks.count.HP_sv_escape_c_post > 0) {
		size_t (*postHookFunc) (size_t retVal___, char *out_dest, const char *src, size_t len, const char *escapes);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sv_escape_c_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sv_escape_c_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, out_dest, src, len, escapes);
		}
	}
	return retVal___;
}
size_t HP_sv_unescape_c(char *out_dest, const char *src, size_t len) {
	int hIndex = 0;
	size_t retVal___ = 0;
	if (HPMHooks.count.HP_sv_unescape_c_pre > 0) {
		size_t (*preHookFunc) (char **out_dest, const char **src, size_t *len);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sv_unescape_c_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sv_unescape_c_pre[hIndex].func;
			retVal___ = preHookFunc(&out_dest, &src, &len);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sv.unescape_c(out_dest, src, len);
	}
	if (HPMHooks.count.HP_sv_unescape_c_post > 0) {
		size_t (*postHookFunc) (size_t retVal___, char *out_dest, const char *src, size_t len);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sv_unescape_c_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sv_unescape_c_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, out_dest, src, len);
		}
	}
	return retVal___;
}
const char* HP_sv_skip_escaped_c(const char *p) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if (HPMHooks.count.HP_sv_skip_escaped_c_pre > 0) {
		const char* (*preHookFunc) (const char **p);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sv_skip_escaped_c_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sv_skip_escaped_c_pre[hIndex].func;
			retVal___ = preHookFunc(&p);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sv.skip_escaped_c(p);
	}
	if (HPMHooks.count.HP_sv_skip_escaped_c_post > 0) {
		const char* (*postHookFunc) (const char* retVal___, const char *p);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sv_skip_escaped_c_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sv_skip_escaped_c_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, p);
		}
	}
	return retVal___;
}
bool HP_sv_readdb(const char *directory, const char *filename, char delim, int mincols, int maxcols, int maxrows, bool ( *parseproc ) (char *fields[], int columns, int current)) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_sv_readdb_pre > 0) {
		bool (*preHookFunc) (const char **directory, const char **filename, char *delim, int *mincols, int *maxcols, int *maxrows, bool ( **parseproc ) (char *fields[], int columns, int current));
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sv_readdb_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sv_readdb_pre[hIndex].func;
			retVal___ = preHookFunc(&directory, &filename, &delim, &mincols, &maxcols, &maxrows, &parseproc);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sv.readdb(directory, filename, delim, mincols, maxcols, maxrows, parseproc);
	}
	if (HPMHooks.count.HP_sv_readdb_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *directory, const char *filename, char delim, int mincols, int maxcols, int maxrows, bool ( *parseproc ) (char *fields[], int columns, int current));
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sv_readdb_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sv_readdb_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, directory, filename, delim, mincols, maxcols, maxrows, parseproc);
		}
	}
	return retVal___;
}
/* sysinfo_interface */
int HP_sysinfo_getpagesize(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_sysinfo_getpagesize_pre > 0) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_getpagesize_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sysinfo_getpagesize_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.getpagesize();
	}
	if (HPMHooks.count.HP_sysinfo_getpagesize_post > 0) {
		int (*postHookFunc) (int retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_getpagesize_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sysinfo_getpagesize_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
const char* HP_sysinfo_platform(void) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if (HPMHooks.count.HP_sysinfo_platform_pre > 0) {
		const char* (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_platform_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sysinfo_platform_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.platform();
	}
	if (HPMHooks.count.HP_sysinfo_platform_post > 0) {
		const char* (*postHookFunc) (const char* retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_platform_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sysinfo_platform_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
const char* HP_sysinfo_osversion(void) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if (HPMHooks.count.HP_sysinfo_osversion_pre > 0) {
		const char* (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_osversion_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sysinfo_osversion_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.osversion();
	}
	if (HPMHooks.count.HP_sysinfo_osversion_post > 0) {
		const char* (*postHookFunc) (const char* retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_osversion_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sysinfo_osversion_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
const char* HP_sysinfo_cpu(void) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if (HPMHooks.count.HP_sysinfo_cpu_pre > 0) {
		const char* (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_cpu_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sysinfo_cpu_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.cpu();
	}
	if (HPMHooks.count.HP_sysinfo_cpu_post > 0) {
		const char* (*postHookFunc) (const char* retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_cpu_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sysinfo_cpu_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
int HP_sysinfo_cpucores(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_sysinfo_cpucores_pre > 0) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_cpucores_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sysinfo_cpucores_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.cpucores();
	}
	if (HPMHooks.count.HP_sysinfo_cpucores_post > 0) {
		int (*postHookFunc) (int retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_cpucores_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sysinfo_cpucores_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
const char* HP_sysinfo_arch(void) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if (HPMHooks.count.HP_sysinfo_arch_pre > 0) {
		const char* (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_arch_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sysinfo_arch_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.arch();
	}
	if (HPMHooks.count.HP_sysinfo_arch_post > 0) {
		const char* (*postHookFunc) (const char* retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_arch_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sysinfo_arch_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
bool HP_sysinfo_is64bit(void) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_sysinfo_is64bit_pre > 0) {
		bool (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_is64bit_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sysinfo_is64bit_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.is64bit();
	}
	if (HPMHooks.count.HP_sysinfo_is64bit_post > 0) {
		bool (*postHookFunc) (bool retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_is64bit_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sysinfo_is64bit_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
const char* HP_sysinfo_compiler(void) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if (HPMHooks.count.HP_sysinfo_compiler_pre > 0) {
		const char* (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_compiler_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sysinfo_compiler_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.compiler();
	}
	if (HPMHooks.count.HP_sysinfo_compiler_post > 0) {
		const char* (*postHookFunc) (const char* retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_compiler_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sysinfo_compiler_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
const char* HP_sysinfo_cflags(void) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if (HPMHooks.count.HP_sysinfo_cflags_pre > 0) {
		const char* (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_cflags_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sysinfo_cflags_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.cflags();
	}
	if (HPMHooks.count.HP_sysinfo_cflags_post > 0) {
		const char* (*postHookFunc) (const char* retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_cflags_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sysinfo_cflags_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
const char* HP_sysinfo_time(void) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if (HPMHooks.count.HP_sysinfo_time_pre > 0) {
		const char* (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_time_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sysinfo_time_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.time();
	}
	if (HPMHooks.count.HP_sysinfo_time_post > 0) {
		const char* (*postHookFunc) (const char* retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_time_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sysinfo_time_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
const char* HP_sysinfo_vcstype(void) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if (HPMHooks.count.HP_sysinfo_vcstype_pre > 0) {
		const char* (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_vcstype_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sysinfo_vcstype_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.vcstype();
	}
	if (HPMHooks.count.HP_sysinfo_vcstype_post > 0) {
		const char* (*postHookFunc) (const char* retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_vcstype_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sysinfo_vcstype_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
int HP_sysinfo_vcstypeid(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_sysinfo_vcstypeid_pre > 0) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_vcstypeid_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sysinfo_vcstypeid_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.vcstypeid();
	}
	if (HPMHooks.count.HP_sysinfo_vcstypeid_post > 0) {
		int (*postHookFunc) (int retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_vcstypeid_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sysinfo_vcstypeid_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
const char* HP_sysinfo_vcsrevision_src(void) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if (HPMHooks.count.HP_sysinfo_vcsrevision_src_pre > 0) {
		const char* (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_vcsrevision_src_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sysinfo_vcsrevision_src_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.vcsrevision_src();
	}
	if (HPMHooks.count.HP_sysinfo_vcsrevision_src_post > 0) {
		const char* (*postHookFunc) (const char* retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_vcsrevision_src_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sysinfo_vcsrevision_src_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
const char* HP_sysinfo_vcsrevision_scripts(void) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if (HPMHooks.count.HP_sysinfo_vcsrevision_scripts_pre > 0) {
		const char* (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_vcsrevision_scripts_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sysinfo_vcsrevision_scripts_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.vcsrevision_scripts();
	}
	if (HPMHooks.count.HP_sysinfo_vcsrevision_scripts_post > 0) {
		const char* (*postHookFunc) (const char* retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_vcsrevision_scripts_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sysinfo_vcsrevision_scripts_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_sysinfo_vcsrevision_reload(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_sysinfo_vcsrevision_reload_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_vcsrevision_reload_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sysinfo_vcsrevision_reload_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sysinfo.vcsrevision_reload();
	}
	if (HPMHooks.count.HP_sysinfo_vcsrevision_reload_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_vcsrevision_reload_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sysinfo_vcsrevision_reload_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
bool HP_sysinfo_is_superuser(void) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_sysinfo_is_superuser_pre > 0) {
		bool (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_is_superuser_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sysinfo_is_superuser_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.is_superuser();
	}
	if (HPMHooks.count.HP_sysinfo_is_superuser_post > 0) {
		bool (*postHookFunc) (bool retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_is_superuser_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sysinfo_is_superuser_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_sysinfo_init(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_sysinfo_init_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sysinfo_init_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sysinfo.init();
	}
	if (HPMHooks.count.HP_sysinfo_init_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sysinfo_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_sysinfo_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_sysinfo_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sysinfo_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sysinfo.final();
	}
	if (HPMHooks.count.HP_sysinfo_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sysinfo_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
/* thread_interface */
void HP_thread_init(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_thread_init_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_thread_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_thread_init_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.thread.init();
	}
	if (HPMHooks.count.HP_thread_init_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_thread_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_thread_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_thread_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_thread_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_thread_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_thread_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.thread.final();
	}
	if (HPMHooks.count.HP_thread_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_thread_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_thread_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
struct thread_handle* HP_thread_create(threadFunc entry_point, void *param) {
	int hIndex = 0;
	struct thread_handle* retVal___ = NULL;
	if (HPMHooks.count.HP_thread_create_pre > 0) {
		struct thread_handle* (*preHookFunc) (threadFunc *entry_point, void **param);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_thread_create_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_thread_create_pre[hIndex].func;
			retVal___ = preHookFunc(&entry_point, &param);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.thread.create(entry_point, param);
	}
	if (HPMHooks.count.HP_thread_create_post > 0) {
		struct thread_handle* (*postHookFunc) (struct thread_handle* retVal___, threadFunc entry_point, void *param);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_thread_create_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_thread_create_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, entry_point, param);
		}
	}
	return retVal___;
}
struct thread_handle* HP_thread_create_opt(threadFunc entry_point, void *param, size_t stack_size, enum thread_priority prio) {
	int hIndex = 0;
	struct thread_handle* retVal___ = NULL;
	if (HPMHooks.count.HP_thread_create_opt_pre > 0) {
		struct thread_handle* (*preHookFunc) (threadFunc *entry_point, void **param, size_t *stack_size, enum thread_priority *prio);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_thread_create_opt_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_thread_create_opt_pre[hIndex].func;
			retVal___ = preHookFunc(&entry_point, &param, &stack_size, &prio);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.thread.create_opt(entry_point, param, stack_size, prio);
	}
	if (HPMHooks.count.HP_thread_create_opt_post > 0) {
		struct thread_handle* (*postHookFunc) (struct thread_handle* retVal___, threadFunc entry_point, void *param, size_t stack_size, enum thread_priority prio);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_thread_create_opt_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_thread_create_opt_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, entry_point, param, stack_size, prio);
		}
	}
	return retVal___;
}
void HP_thread_destroy(struct thread_handle *handle) {
	int hIndex = 0;
	if (HPMHooks.count.HP_thread_destroy_pre > 0) {
		void (*preHookFunc) (struct thread_handle **handle);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_thread_destroy_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_thread_destroy_pre[hIndex].func;
			preHookFunc(&handle);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.thread.destroy(handle);
	}
	if (HPMHooks.count.HP_thread_destroy_post > 0) {
		void (*postHookFunc) (struct thread_handle *handle);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_thread_destroy_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_thread_destroy_post[hIndex].func;
			postHookFunc(handle);
		}
	}
	return;
}
struct thread_handle* HP_thread_self(void) {
	int hIndex = 0;
	struct thread_handle* retVal___ = NULL;
	if (HPMHooks.count.HP_thread_self_pre > 0) {
		struct thread_handle* (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_thread_self_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_thread_self_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.thread.self();
	}
	if (HPMHooks.count.HP_thread_self_post > 0) {
		struct thread_handle* (*postHookFunc) (struct thread_handle* retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_thread_self_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_thread_self_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
int HP_thread_get_tid(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_thread_get_tid_pre > 0) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_thread_get_tid_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_thread_get_tid_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.thread.get_tid();
	}
	if (HPMHooks.count.HP_thread_get_tid_post > 0) {
		int (*postHookFunc) (int retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_thread_get_tid_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_thread_get_tid_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
bool HP_thread_wait(struct thread_handle *handle, void **out_exit_code) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_thread_wait_pre > 0) {
		bool (*preHookFunc) (struct thread_handle **handle, void ***out_exit_code);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_thread_wait_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_thread_wait_pre[hIndex].func;
			retVal___ = preHookFunc(&handle, &out_exit_code);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.thread.wait(handle, out_exit_code);
	}
	if (HPMHooks.count.HP_thread_wait_post > 0) {
		bool (*postHookFunc) (bool retVal___, struct thread_handle *handle, void **out_exit_code);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_thread_wait_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_thread_wait_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, handle, out_exit_code);
		}
	}
	return retVal___;
}
void HP_thread_prio_set(struct thread_handle *handle, enum thread_priority prio) {
	int hIndex = 0;
	if (HPMHooks.count.HP_thread_prio_set_pre > 0) {
		void (*preHookFunc) (struct thread_handle **handle, enum thread_priority *prio);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_thread_prio_set_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_thread_prio_set_pre[hIndex].func;
			preHookFunc(&handle, &prio);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.thread.prio_set(handle, prio);
	}
	if (HPMHooks.count.HP_thread_prio_set_post > 0) {
		void (*postHookFunc) (struct thread_handle *handle, enum thread_priority prio);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_thread_prio_set_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_thread_prio_set_post[hIndex].func;
			postHookFunc(handle, prio);
		}
	}
	return;
}
enum thread_priority HP_thread_prio_get(struct thread_handle *handle) {
	int hIndex = 0;
	enum thread_priority retVal___ = THREADPRIO_NORMAL;
	if (HPMHooks.count.HP_thread_prio_get_pre > 0) {
		enum thread_priority (*preHookFunc) (struct thread_handle **handle);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_thread_prio_get_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_thread_prio_get_pre[hIndex].func;
			retVal___ = preHookFunc(&handle);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.thread.prio_get(handle);
	}
	if (HPMHooks.count.HP_thread_prio_get_post > 0) {
		enum thread_priority (*postHookFunc) (enum thread_priority retVal___, struct thread_handle *handle);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_thread_prio_get_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_thread_prio_get_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, handle);
		}
	}
	return retVal___;
}
void HP_thread_yield(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_thread_yield_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_thread_yield_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_thread_yield_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.thread.yield();
	}
	if (HPMHooks.count.HP_thread_yield_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_thread_yield_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_thread_yield_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
/* timer_interface */
int64 HP_timer_gettick(void) {
	int hIndex = 0;
	int64 retVal___ = 0;
	if (HPMHooks.count.HP_timer_gettick_pre > 0) {
		int64 (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_gettick_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_timer_gettick_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.timer.gettick();
	}
	if (HPMHooks.count.HP_timer_gettick_post > 0) {
		int64 (*postHookFunc) (int64 retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_gettick_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_timer_gettick_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
int64 HP_timer_gettick_nocache(void) {
	int hIndex = 0;
	int64 retVal___ = 0;
	if (HPMHooks.count.HP_timer_gettick_nocache_pre > 0) {
		int64 (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_gettick_nocache_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_timer_gettick_nocache_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.timer.gettick_nocache();
	}
	if (HPMHooks.count.HP_timer_gettick_nocache_post > 0) {
		int64 (*postHookFunc) (int64 retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_gettick_nocache_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_timer_gettick_nocache_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
int HP_timer_add(int64 tick, TimerFunc func, int id, intptr_t data) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_timer_add_pre > 0) {
		int (*preHookFunc) (int64 *tick, TimerFunc *func, int *id, intptr_t *data);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_add_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_timer_add_pre[hIndex].func;
			retVal___ = preHookFunc(&tick, &func, &id, &data);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.timer.add(tick, func, id, data);
	}
	if (HPMHooks.count.HP_timer_add_post > 0) {
		int (*postHookFunc) (int retVal___, int64 tick, TimerFunc func, int id, intptr_t data);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_add_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_timer_add_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, tick, func, id, data);
		}
	}
	return retVal___;
}
int HP_timer_add_interval(int64 tick, TimerFunc func, int id, intptr_t data, int interval) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_timer_add_interval_pre > 0) {
		int (*preHookFunc) (int64 *tick, TimerFunc *func, int *id, intptr_t *data, int *interval);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_add_interval_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_timer_add_interval_pre[hIndex].func;
			retVal___ = preHookFunc(&tick, &func, &id, &data, &interval);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.timer.add_interval(tick, func, id, data, interval);
	}
	if (HPMHooks.count.HP_timer_add_interval_post > 0) {
		int (*postHookFunc) (int retVal___, int64 tick, TimerFunc func, int id, intptr_t data, int interval);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_add_interval_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_timer_add_interval_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, tick, func, id, data, interval);
		}
	}
	return retVal___;
}
const struct TimerData* HP_timer_get(int tid) {
	int hIndex = 0;
	const struct TimerData* retVal___ = NULL;
	if (HPMHooks.count.HP_timer_get_pre > 0) {
		const struct TimerData* (*preHookFunc) (int *tid);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_get_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_timer_get_pre[hIndex].func;
			retVal___ = preHookFunc(&tid);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.timer.get(tid);
	}
	if (HPMHooks.count.HP_timer_get_post > 0) {
		const struct TimerData* (*postHookFunc) (const struct TimerData* retVal___, int tid);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_get_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_timer_get_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, tid);
		}
	}
	return retVal___;
}
int HP_timer_delete(int tid, TimerFunc func) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_timer_delete_pre > 0) {
		int (*preHookFunc) (int *tid, TimerFunc *func);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_delete_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_timer_delete_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &func);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.timer.delete(tid, func);
	}
	if (HPMHooks.count.HP_timer_delete_post > 0) {
		int (*postHookFunc) (int retVal___, int tid, TimerFunc func);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_delete_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_timer_delete_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, tid, func);
		}
	}
	return retVal___;
}
int64 HP_timer_addtick(int tid, int64 tick) {
	int hIndex = 0;
	int64 retVal___ = 0;
	if (HPMHooks.count.HP_timer_addtick_pre > 0) {
		int64 (*preHookFunc) (int *tid, int64 *tick);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_addtick_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_timer_addtick_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &tick);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.timer.addtick(tid, tick);
	}
	if (HPMHooks.count.HP_timer_addtick_post > 0) {
		int64 (*postHookFunc) (int64 retVal___, int tid, int64 tick);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_addtick_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_timer_addtick_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, tid, tick);
		}
	}
	return retVal___;
}
int64 HP_timer_settick(int tid, int64 tick) {
	int hIndex = 0;
	int64 retVal___ = 0;
	if (HPMHooks.count.HP_timer_settick_pre > 0) {
		int64 (*preHookFunc) (int *tid, int64 *tick);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_settick_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_timer_settick_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &tick);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.timer.settick(tid, tick);
	}
	if (HPMHooks.count.HP_timer_settick_post > 0) {
		int64 (*postHookFunc) (int64 retVal___, int tid, int64 tick);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_settick_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_timer_settick_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, tid, tick);
		}
	}
	return retVal___;
}
int HP_timer_add_func_list(TimerFunc func, char *name) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_timer_add_func_list_pre > 0) {
		int (*preHookFunc) (TimerFunc *func, char **name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_add_func_list_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_timer_add_func_list_pre[hIndex].func;
			retVal___ = preHookFunc(&func, &name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.timer.add_func_list(func, name);
	}
	if (HPMHooks.count.HP_timer_add_func_list_post > 0) {
		int (*postHookFunc) (int retVal___, TimerFunc func, char *name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_add_func_list_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_timer_add_func_list_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, func, name);
		}
	}
	return retVal___;
}
unsigned long HP_timer_get_uptime(void) {
	int hIndex = 0;
	unsigned long retVal___ = 0;
	if (HPMHooks.count.HP_timer_get_uptime_pre > 0) {
		unsigned long (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_get_uptime_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_timer_get_uptime_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.timer.get_uptime();
	}
	if (HPMHooks.count.HP_timer_get_uptime_post > 0) {
		unsigned long (*postHookFunc) (unsigned long retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_get_uptime_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_timer_get_uptime_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
int HP_timer_perform(int64 tick) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_timer_perform_pre > 0) {
		int (*preHookFunc) (int64 *tick);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_perform_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_timer_perform_pre[hIndex].func;
			retVal___ = preHookFunc(&tick);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.timer.perform(tick);
	}
	if (HPMHooks.count.HP_timer_perform_post > 0) {
		int (*postHookFunc) (int retVal___, int64 tick);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_perform_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_timer_perform_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, tick);
		}
	}
	return retVal___;
}
void HP_timer_init(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_timer_init_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_timer_init_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.timer.init();
	}
	if (HPMHooks.count.HP_timer_init_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_timer_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_timer_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_timer_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_timer_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.timer.final();
	}
	if (HPMHooks.count.HP_timer_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_timer_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_timer_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
