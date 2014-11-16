// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
//
// NOTE: This file was auto-generated and should never be manually edited,
//       as it will get overwritten.

/* chr */
int HP_chr_waiting_disconnect(int tid, int64 tick, int id, intptr_t data) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_waiting_disconnect_pre ) {
		int (*preHookFunc) (int *tid, int64 *tick, int *id, intptr_t *data);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_waiting_disconnect_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_waiting_disconnect_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &tick, &id, &data);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.waiting_disconnect(tid, tick, id, data);
	}
	if( HPMHooks.count.HP_chr_waiting_disconnect_post ) {
		int (*postHookFunc) (int retVal___, int *tid, int64 *tick, int *id, intptr_t *data);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_waiting_disconnect_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_waiting_disconnect_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &tid, &tick, &id, &data);
		}
	}
	return retVal___;
}
int HP_chr_delete_char_sql(int char_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_delete_char_sql_pre ) {
		int (*preHookFunc) (int *char_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete_char_sql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_delete_char_sql_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.delete_char_sql(char_id);
	}
	if( HPMHooks.count.HP_chr_delete_char_sql_post ) {
		int (*postHookFunc) (int retVal___, int *char_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete_char_sql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_delete_char_sql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &char_id);
		}
	}
	return retVal___;
}
DBData HP_chr_create_online_char_data(DBKey key, va_list args) {
	int hIndex = 0;
	DBData retVal___;
	memset(&retVal___, '\0', sizeof(DBData));
	if( HPMHooks.count.HP_chr_create_online_char_data_pre ) {
		DBData (*preHookFunc) (DBKey *key, va_list args);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_create_online_char_data_pre; hIndex++ ) {
			va_list args___copy; va_copy(args___copy, args);
			preHookFunc = HPMHooks.list.HP_chr_create_online_char_data_pre[hIndex].func;
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
		retVal___ = HPMHooks.source.chr.create_online_char_data(key, args___copy);
		va_end(args___copy);
	}
	if( HPMHooks.count.HP_chr_create_online_char_data_post ) {
		DBData (*postHookFunc) (DBData retVal___, DBKey *key, va_list args);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_create_online_char_data_post; hIndex++ ) {
			va_list args___copy; va_copy(args___copy, args);
			postHookFunc = HPMHooks.list.HP_chr_create_online_char_data_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &key, args___copy);
			va_end(args___copy);
		}
	}
	return retVal___;
}
void HP_chr_set_account_online(int account_id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_set_account_online_pre ) {
		void (*preHookFunc) (int *account_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_account_online_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_set_account_online_pre[hIndex].func;
			preHookFunc(&account_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.set_account_online(account_id);
	}
	if( HPMHooks.count.HP_chr_set_account_online_post ) {
		void (*postHookFunc) (int *account_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_account_online_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_set_account_online_post[hIndex].func;
			postHookFunc(&account_id);
		}
	}
	return;
}
void HP_chr_set_account_offline(int account_id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_set_account_offline_pre ) {
		void (*preHookFunc) (int *account_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_account_offline_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_set_account_offline_pre[hIndex].func;
			preHookFunc(&account_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.set_account_offline(account_id);
	}
	if( HPMHooks.count.HP_chr_set_account_offline_post ) {
		void (*postHookFunc) (int *account_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_account_offline_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_set_account_offline_post[hIndex].func;
			postHookFunc(&account_id);
		}
	}
	return;
}
void HP_chr_set_char_charselect(int account_id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_set_char_charselect_pre ) {
		void (*preHookFunc) (int *account_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_char_charselect_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_set_char_charselect_pre[hIndex].func;
			preHookFunc(&account_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.set_char_charselect(account_id);
	}
	if( HPMHooks.count.HP_chr_set_char_charselect_post ) {
		void (*postHookFunc) (int *account_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_char_charselect_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_set_char_charselect_post[hIndex].func;
			postHookFunc(&account_id);
		}
	}
	return;
}
void HP_chr_set_char_online(int map_id, int char_id, int account_id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_set_char_online_pre ) {
		void (*preHookFunc) (int *map_id, int *char_id, int *account_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_char_online_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_set_char_online_pre[hIndex].func;
			preHookFunc(&map_id, &char_id, &account_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.set_char_online(map_id, char_id, account_id);
	}
	if( HPMHooks.count.HP_chr_set_char_online_post ) {
		void (*postHookFunc) (int *map_id, int *char_id, int *account_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_char_online_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_set_char_online_post[hIndex].func;
			postHookFunc(&map_id, &char_id, &account_id);
		}
	}
	return;
}
void HP_chr_set_char_offline(int char_id, int account_id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_set_char_offline_pre ) {
		void (*preHookFunc) (int *char_id, int *account_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_char_offline_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_set_char_offline_pre[hIndex].func;
			preHookFunc(&char_id, &account_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.set_char_offline(char_id, account_id);
	}
	if( HPMHooks.count.HP_chr_set_char_offline_post ) {
		void (*postHookFunc) (int *char_id, int *account_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_char_offline_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_set_char_offline_post[hIndex].func;
			postHookFunc(&char_id, &account_id);
		}
	}
	return;
}
int HP_chr_db_setoffline(DBKey key, DBData *data, va_list ap) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_db_setoffline_pre ) {
		int (*preHookFunc) (DBKey *key, DBData *data, va_list ap);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_db_setoffline_pre; hIndex++ ) {
			va_list ap___copy; va_copy(ap___copy, ap);
			preHookFunc = HPMHooks.list.HP_chr_db_setoffline_pre[hIndex].func;
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
		retVal___ = HPMHooks.source.chr.db_setoffline(key, data, ap___copy);
		va_end(ap___copy);
	}
	if( HPMHooks.count.HP_chr_db_setoffline_post ) {
		int (*postHookFunc) (int retVal___, DBKey *key, DBData *data, va_list ap);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_db_setoffline_post; hIndex++ ) {
			va_list ap___copy; va_copy(ap___copy, ap);
			postHookFunc = HPMHooks.list.HP_chr_db_setoffline_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &key, data, ap___copy);
			va_end(ap___copy);
		}
	}
	return retVal___;
}
int HP_chr_db_kickoffline(DBKey key, DBData *data, va_list ap) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_db_kickoffline_pre ) {
		int (*preHookFunc) (DBKey *key, DBData *data, va_list ap);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_db_kickoffline_pre; hIndex++ ) {
			va_list ap___copy; va_copy(ap___copy, ap);
			preHookFunc = HPMHooks.list.HP_chr_db_kickoffline_pre[hIndex].func;
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
		retVal___ = HPMHooks.source.chr.db_kickoffline(key, data, ap___copy);
		va_end(ap___copy);
	}
	if( HPMHooks.count.HP_chr_db_kickoffline_post ) {
		int (*postHookFunc) (int retVal___, DBKey *key, DBData *data, va_list ap);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_db_kickoffline_post; hIndex++ ) {
			va_list ap___copy; va_copy(ap___copy, ap);
			postHookFunc = HPMHooks.list.HP_chr_db_kickoffline_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &key, data, ap___copy);
			va_end(ap___copy);
		}
	}
	return retVal___;
}
void HP_chr_set_login_all_offline(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_set_login_all_offline_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_login_all_offline_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_set_login_all_offline_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.set_login_all_offline();
	}
	if( HPMHooks.count.HP_chr_set_login_all_offline_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_login_all_offline_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_set_login_all_offline_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_chr_set_all_offline(int id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_set_all_offline_pre ) {
		void (*preHookFunc) (int *id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_all_offline_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_set_all_offline_pre[hIndex].func;
			preHookFunc(&id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.set_all_offline(id);
	}
	if( HPMHooks.count.HP_chr_set_all_offline_post ) {
		void (*postHookFunc) (int *id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_all_offline_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_set_all_offline_post[hIndex].func;
			postHookFunc(&id);
		}
	}
	return;
}
void HP_chr_set_all_offline_sql(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_set_all_offline_sql_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_all_offline_sql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_set_all_offline_sql_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.set_all_offline_sql();
	}
	if( HPMHooks.count.HP_chr_set_all_offline_sql_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_set_all_offline_sql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_set_all_offline_sql_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
DBData HP_chr_create_charstatus(DBKey key, va_list args) {
	int hIndex = 0;
	DBData retVal___;
	memset(&retVal___, '\0', sizeof(DBData));
	if( HPMHooks.count.HP_chr_create_charstatus_pre ) {
		DBData (*preHookFunc) (DBKey *key, va_list args);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_create_charstatus_pre; hIndex++ ) {
			va_list args___copy; va_copy(args___copy, args);
			preHookFunc = HPMHooks.list.HP_chr_create_charstatus_pre[hIndex].func;
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
		retVal___ = HPMHooks.source.chr.create_charstatus(key, args___copy);
		va_end(args___copy);
	}
	if( HPMHooks.count.HP_chr_create_charstatus_post ) {
		DBData (*postHookFunc) (DBData retVal___, DBKey *key, va_list args);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_create_charstatus_post; hIndex++ ) {
			va_list args___copy; va_copy(args___copy, args);
			postHookFunc = HPMHooks.list.HP_chr_create_charstatus_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &key, args___copy);
			va_end(args___copy);
		}
	}
	return retVal___;
}
int HP_chr_mmo_char_tosql(int char_id, struct mmo_charstatus *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_mmo_char_tosql_pre ) {
		int (*preHookFunc) (int *char_id, struct mmo_charstatus *p);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_tosql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_mmo_char_tosql_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, p);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.mmo_char_tosql(char_id, p);
	}
	if( HPMHooks.count.HP_chr_mmo_char_tosql_post ) {
		int (*postHookFunc) (int retVal___, int *char_id, struct mmo_charstatus *p);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_tosql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_mmo_char_tosql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &char_id, p);
		}
	}
	return retVal___;
}
int HP_chr_memitemdata_to_sql(const struct item items[], int max, int id, int tableswitch) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_memitemdata_to_sql_pre ) {
		int (*preHookFunc) (const struct item *items[], int *max, int *id, int *tableswitch);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_memitemdata_to_sql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_memitemdata_to_sql_pre[hIndex].func;
			retVal___ = preHookFunc(&items, &max, &id, &tableswitch);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.memitemdata_to_sql(items, max, id, tableswitch);
	}
	if( HPMHooks.count.HP_chr_memitemdata_to_sql_post ) {
		int (*postHookFunc) (int retVal___, const struct item *items[], int *max, int *id, int *tableswitch);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_memitemdata_to_sql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_memitemdata_to_sql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &items, &max, &id, &tableswitch);
		}
	}
	return retVal___;
}
int HP_chr_inventory_to_sql(const struct item items[], int max, int id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_inventory_to_sql_pre ) {
		int (*preHookFunc) (const struct item *items[], int *max, int *id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_inventory_to_sql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_inventory_to_sql_pre[hIndex].func;
			retVal___ = preHookFunc(&items, &max, &id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.inventory_to_sql(items, max, id);
	}
	if( HPMHooks.count.HP_chr_inventory_to_sql_post ) {
		int (*postHookFunc) (int retVal___, const struct item *items[], int *max, int *id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_inventory_to_sql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_inventory_to_sql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &items, &max, &id);
		}
	}
	return retVal___;
}
int HP_chr_mmo_chars_fromsql(struct char_session_data *sd, uint8 *buf) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_mmo_chars_fromsql_pre ) {
		int (*preHookFunc) (struct char_session_data *sd, uint8 *buf);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_chars_fromsql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_mmo_chars_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(sd, buf);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.mmo_chars_fromsql(sd, buf);
	}
	if( HPMHooks.count.HP_chr_mmo_chars_fromsql_post ) {
		int (*postHookFunc) (int retVal___, struct char_session_data *sd, uint8 *buf);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_chars_fromsql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_mmo_chars_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, sd, buf);
		}
	}
	return retVal___;
}
int HP_chr_mmo_char_fromsql(int char_id, struct mmo_charstatus *p, bool load_everything) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_mmo_char_fromsql_pre ) {
		int (*preHookFunc) (int *char_id, struct mmo_charstatus *p, bool *load_everything);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_fromsql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_mmo_char_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, p, &load_everything);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.mmo_char_fromsql(char_id, p, load_everything);
	}
	if( HPMHooks.count.HP_chr_mmo_char_fromsql_post ) {
		int (*postHookFunc) (int retVal___, int *char_id, struct mmo_charstatus *p, bool *load_everything);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_fromsql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_mmo_char_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &char_id, p, &load_everything);
		}
	}
	return retVal___;
}
int HP_chr_mmo_char_sql_init(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_mmo_char_sql_init_pre ) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_sql_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_mmo_char_sql_init_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.mmo_char_sql_init();
	}
	if( HPMHooks.count.HP_chr_mmo_char_sql_init_post ) {
		int (*postHookFunc) (int retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_sql_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_mmo_char_sql_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
bool HP_chr_char_slotchange(struct char_session_data *sd, int fd, unsigned short from, unsigned short to) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_chr_char_slotchange_pre ) {
		bool (*preHookFunc) (struct char_session_data *sd, int *fd, unsigned short *from, unsigned short *to);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_char_slotchange_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_char_slotchange_pre[hIndex].func;
			retVal___ = preHookFunc(sd, &fd, &from, &to);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.char_slotchange(sd, fd, from, to);
	}
	if( HPMHooks.count.HP_chr_char_slotchange_post ) {
		bool (*postHookFunc) (bool retVal___, struct char_session_data *sd, int *fd, unsigned short *from, unsigned short *to);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_char_slotchange_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_char_slotchange_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, sd, &fd, &from, &to);
		}
	}
	return retVal___;
}
int HP_chr_rename_char_sql(struct char_session_data *sd, int char_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_rename_char_sql_pre ) {
		int (*preHookFunc) (struct char_session_data *sd, int *char_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_rename_char_sql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_rename_char_sql_pre[hIndex].func;
			retVal___ = preHookFunc(sd, &char_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.rename_char_sql(sd, char_id);
	}
	if( HPMHooks.count.HP_chr_rename_char_sql_post ) {
		int (*postHookFunc) (int retVal___, struct char_session_data *sd, int *char_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_rename_char_sql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_rename_char_sql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, sd, &char_id);
		}
	}
	return retVal___;
}
int HP_chr_check_char_name(char *name, char *esc_name) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_check_char_name_pre ) {
		int (*preHookFunc) (char *name, char *esc_name);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_check_char_name_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_check_char_name_pre[hIndex].func;
			retVal___ = preHookFunc(name, esc_name);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.check_char_name(name, esc_name);
	}
	if( HPMHooks.count.HP_chr_check_char_name_post ) {
		int (*postHookFunc) (int retVal___, char *name, char *esc_name);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_check_char_name_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_check_char_name_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, name, esc_name);
		}
	}
	return retVal___;
}
int HP_chr_make_new_char_sql(struct char_session_data *sd, char *name_, int str, int agi, int vit, int int_, int dex, int luk, int slot, int hair_color, int hair_style) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_make_new_char_sql_pre ) {
		int (*preHookFunc) (struct char_session_data *sd, char *name_, int *str, int *agi, int *vit, int *int_, int *dex, int *luk, int *slot, int *hair_color, int *hair_style);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_make_new_char_sql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_make_new_char_sql_pre[hIndex].func;
			retVal___ = preHookFunc(sd, name_, &str, &agi, &vit, &int_, &dex, &luk, &slot, &hair_color, &hair_style);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.make_new_char_sql(sd, name_, str, agi, vit, int_, dex, luk, slot, hair_color, hair_style);
	}
	if( HPMHooks.count.HP_chr_make_new_char_sql_post ) {
		int (*postHookFunc) (int retVal___, struct char_session_data *sd, char *name_, int *str, int *agi, int *vit, int *int_, int *dex, int *luk, int *slot, int *hair_color, int *hair_style);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_make_new_char_sql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_make_new_char_sql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, sd, name_, &str, &agi, &vit, &int_, &dex, &luk, &slot, &hair_color, &hair_style);
		}
	}
	return retVal___;
}
int HP_chr_divorce_char_sql(int partner_id1, int partner_id2) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_divorce_char_sql_pre ) {
		int (*preHookFunc) (int *partner_id1, int *partner_id2);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_divorce_char_sql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_divorce_char_sql_pre[hIndex].func;
			retVal___ = preHookFunc(&partner_id1, &partner_id2);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.divorce_char_sql(partner_id1, partner_id2);
	}
	if( HPMHooks.count.HP_chr_divorce_char_sql_post ) {
		int (*postHookFunc) (int retVal___, int *partner_id1, int *partner_id2);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_divorce_char_sql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_divorce_char_sql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &partner_id1, &partner_id2);
		}
	}
	return retVal___;
}
int HP_chr_count_users(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_count_users_pre ) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_count_users_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_count_users_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.count_users();
	}
	if( HPMHooks.count.HP_chr_count_users_post ) {
		int (*postHookFunc) (int retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_count_users_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_count_users_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
int HP_chr_mmo_char_tobuf(uint8 *buffer, struct mmo_charstatus *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_mmo_char_tobuf_pre ) {
		int (*preHookFunc) (uint8 *buffer, struct mmo_charstatus *p);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_tobuf_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_mmo_char_tobuf_pre[hIndex].func;
			retVal___ = preHookFunc(buffer, p);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.mmo_char_tobuf(buffer, p);
	}
	if( HPMHooks.count.HP_chr_mmo_char_tobuf_post ) {
		int (*postHookFunc) (int retVal___, uint8 *buffer, struct mmo_charstatus *p);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_tobuf_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_mmo_char_tobuf_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, buffer, p);
		}
	}
	return retVal___;
}
void HP_chr_mmo_char_send099d(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_mmo_char_send099d_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_send099d_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_mmo_char_send099d_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.mmo_char_send099d(fd, sd);
	}
	if( HPMHooks.count.HP_chr_mmo_char_send099d_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_send099d_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_mmo_char_send099d_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
void HP_chr_mmo_char_send_ban_list(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_mmo_char_send_ban_list_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_send_ban_list_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_mmo_char_send_ban_list_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.mmo_char_send_ban_list(fd, sd);
	}
	if( HPMHooks.count.HP_chr_mmo_char_send_ban_list_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_send_ban_list_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_mmo_char_send_ban_list_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
void HP_chr_mmo_char_send_slots_info(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_mmo_char_send_slots_info_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_send_slots_info_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_mmo_char_send_slots_info_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.mmo_char_send_slots_info(fd, sd);
	}
	if( HPMHooks.count.HP_chr_mmo_char_send_slots_info_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_send_slots_info_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_mmo_char_send_slots_info_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
int HP_chr_mmo_char_send_characters(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_mmo_char_send_characters_pre ) {
		int (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_send_characters_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_mmo_char_send_characters_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.mmo_char_send_characters(fd, sd);
	}
	if( HPMHooks.count.HP_chr_mmo_char_send_characters_post ) {
		int (*postHookFunc) (int retVal___, int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_mmo_char_send_characters_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_mmo_char_send_characters_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, sd);
		}
	}
	return retVal___;
}
int HP_chr_char_married(int pl1, int pl2) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_char_married_pre ) {
		int (*preHookFunc) (int *pl1, int *pl2);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_char_married_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_char_married_pre[hIndex].func;
			retVal___ = preHookFunc(&pl1, &pl2);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.char_married(pl1, pl2);
	}
	if( HPMHooks.count.HP_chr_char_married_post ) {
		int (*postHookFunc) (int retVal___, int *pl1, int *pl2);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_char_married_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_char_married_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &pl1, &pl2);
		}
	}
	return retVal___;
}
int HP_chr_char_child(int parent_id, int child_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_char_child_pre ) {
		int (*preHookFunc) (int *parent_id, int *child_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_char_child_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_char_child_pre[hIndex].func;
			retVal___ = preHookFunc(&parent_id, &child_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.char_child(parent_id, child_id);
	}
	if( HPMHooks.count.HP_chr_char_child_post ) {
		int (*postHookFunc) (int retVal___, int *parent_id, int *child_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_char_child_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_char_child_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &parent_id, &child_id);
		}
	}
	return retVal___;
}
int HP_chr_char_family(int cid1, int cid2, int cid3) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_char_family_pre ) {
		int (*preHookFunc) (int *cid1, int *cid2, int *cid3);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_char_family_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_char_family_pre[hIndex].func;
			retVal___ = preHookFunc(&cid1, &cid2, &cid3);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.char_family(cid1, cid2, cid3);
	}
	if( HPMHooks.count.HP_chr_char_family_post ) {
		int (*postHookFunc) (int retVal___, int *cid1, int *cid2, int *cid3);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_char_family_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_char_family_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &cid1, &cid2, &cid3);
		}
	}
	return retVal___;
}
void HP_chr_disconnect_player(int account_id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_disconnect_player_pre ) {
		void (*preHookFunc) (int *account_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_disconnect_player_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_disconnect_player_pre[hIndex].func;
			preHookFunc(&account_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.disconnect_player(account_id);
	}
	if( HPMHooks.count.HP_chr_disconnect_player_post ) {
		void (*postHookFunc) (int *account_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_disconnect_player_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_disconnect_player_post[hIndex].func;
			postHookFunc(&account_id);
		}
	}
	return;
}
void HP_chr_authfail_fd(int fd, int type) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_authfail_fd_pre ) {
		void (*preHookFunc) (int *fd, int *type);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_authfail_fd_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_authfail_fd_pre[hIndex].func;
			preHookFunc(&fd, &type);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.authfail_fd(fd, type);
	}
	if( HPMHooks.count.HP_chr_authfail_fd_post ) {
		void (*postHookFunc) (int *fd, int *type);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_authfail_fd_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_authfail_fd_post[hIndex].func;
			postHookFunc(&fd, &type);
		}
	}
	return;
}
void HP_chr_request_account_data(int account_id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_request_account_data_pre ) {
		void (*preHookFunc) (int *account_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_request_account_data_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_request_account_data_pre[hIndex].func;
			preHookFunc(&account_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.request_account_data(account_id);
	}
	if( HPMHooks.count.HP_chr_request_account_data_post ) {
		void (*postHookFunc) (int *account_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_request_account_data_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_request_account_data_post[hIndex].func;
			postHookFunc(&account_id);
		}
	}
	return;
}
void HP_chr_auth_ok(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_auth_ok_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_auth_ok_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_auth_ok_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.auth_ok(fd, sd);
	}
	if( HPMHooks.count.HP_chr_auth_ok_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_auth_ok_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_auth_ok_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
void HP_chr_ping_login_server(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_ping_login_server_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_ping_login_server_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_ping_login_server_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.ping_login_server(fd);
	}
	if( HPMHooks.count.HP_chr_ping_login_server_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_ping_login_server_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_ping_login_server_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
int HP_chr_parse_fromlogin_connection_state(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_parse_fromlogin_connection_state_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_connection_state_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_connection_state_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.parse_fromlogin_connection_state(fd);
	}
	if( HPMHooks.count.HP_chr_parse_fromlogin_connection_state_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_connection_state_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_connection_state_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
void HP_chr_auth_error(int fd, unsigned char flag) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_auth_error_pre ) {
		void (*preHookFunc) (int *fd, unsigned char *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_auth_error_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_auth_error_pre[hIndex].func;
			preHookFunc(&fd, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.auth_error(fd, flag);
	}
	if( HPMHooks.count.HP_chr_auth_error_post ) {
		void (*postHookFunc) (int *fd, unsigned char *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_auth_error_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_auth_error_post[hIndex].func;
			postHookFunc(&fd, &flag);
		}
	}
	return;
}
void HP_chr_parse_fromlogin_auth_state(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_fromlogin_auth_state_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_auth_state_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_auth_state_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_fromlogin_auth_state(fd);
	}
	if( HPMHooks.count.HP_chr_parse_fromlogin_auth_state_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_auth_state_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_auth_state_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_fromlogin_account_data(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_fromlogin_account_data_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_account_data_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_account_data_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_fromlogin_account_data(fd);
	}
	if( HPMHooks.count.HP_chr_parse_fromlogin_account_data_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_account_data_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_account_data_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_fromlogin_login_pong(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_fromlogin_login_pong_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_login_pong_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_login_pong_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_fromlogin_login_pong(fd);
	}
	if( HPMHooks.count.HP_chr_parse_fromlogin_login_pong_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_login_pong_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_login_pong_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_changesex(int account_id, int sex) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_changesex_pre ) {
		void (*preHookFunc) (int *account_id, int *sex);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_changesex_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_changesex_pre[hIndex].func;
			preHookFunc(&account_id, &sex);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.changesex(account_id, sex);
	}
	if( HPMHooks.count.HP_chr_changesex_post ) {
		void (*postHookFunc) (int *account_id, int *sex);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_changesex_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_changesex_post[hIndex].func;
			postHookFunc(&account_id, &sex);
		}
	}
	return;
}
int HP_chr_parse_fromlogin_changesex_reply(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_parse_fromlogin_changesex_reply_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_changesex_reply_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_changesex_reply_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.parse_fromlogin_changesex_reply(fd);
	}
	if( HPMHooks.count.HP_chr_parse_fromlogin_changesex_reply_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_changesex_reply_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_changesex_reply_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
void HP_chr_parse_fromlogin_account_reg2(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_fromlogin_account_reg2_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_account_reg2_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_account_reg2_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_fromlogin_account_reg2(fd);
	}
	if( HPMHooks.count.HP_chr_parse_fromlogin_account_reg2_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_account_reg2_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_account_reg2_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_fromlogin_ban(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_fromlogin_ban_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_ban_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_ban_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_fromlogin_ban(fd);
	}
	if( HPMHooks.count.HP_chr_parse_fromlogin_ban_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_ban_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_ban_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_fromlogin_kick(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_fromlogin_kick_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_kick_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_kick_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_fromlogin_kick(fd);
	}
	if( HPMHooks.count.HP_chr_parse_fromlogin_kick_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_kick_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_kick_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_update_ip(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_update_ip_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_update_ip_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_update_ip_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.update_ip(fd);
	}
	if( HPMHooks.count.HP_chr_update_ip_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_update_ip_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_update_ip_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_fromlogin_update_ip(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_fromlogin_update_ip_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_update_ip_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_update_ip_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_fromlogin_update_ip(fd);
	}
	if( HPMHooks.count.HP_chr_parse_fromlogin_update_ip_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_update_ip_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_update_ip_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_fromlogin_accinfo2_failed(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_fromlogin_accinfo2_failed_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_accinfo2_failed_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_accinfo2_failed_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_fromlogin_accinfo2_failed(fd);
	}
	if( HPMHooks.count.HP_chr_parse_fromlogin_accinfo2_failed_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_accinfo2_failed_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_accinfo2_failed_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_fromlogin_accinfo2_ok(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_fromlogin_accinfo2_ok_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_accinfo2_ok_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_accinfo2_ok_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_fromlogin_accinfo2_ok(fd);
	}
	if( HPMHooks.count.HP_chr_parse_fromlogin_accinfo2_ok_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_accinfo2_ok_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_accinfo2_ok_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
int HP_chr_parse_fromlogin(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_parse_fromlogin_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.parse_fromlogin(fd);
	}
	if( HPMHooks.count.HP_chr_parse_fromlogin_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_fromlogin_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_fromlogin_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
int HP_chr_request_accreg2(int account_id, int char_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_request_accreg2_pre ) {
		int (*preHookFunc) (int *account_id, int *char_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_request_accreg2_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_request_accreg2_pre[hIndex].func;
			retVal___ = preHookFunc(&account_id, &char_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.request_accreg2(account_id, char_id);
	}
	if( HPMHooks.count.HP_chr_request_accreg2_post ) {
		int (*postHookFunc) (int retVal___, int *account_id, int *char_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_request_accreg2_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_request_accreg2_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &account_id, &char_id);
		}
	}
	return retVal___;
}
void HP_chr_global_accreg_to_login_start(int account_id, int char_id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_global_accreg_to_login_start_pre ) {
		void (*preHookFunc) (int *account_id, int *char_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_global_accreg_to_login_start_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_global_accreg_to_login_start_pre[hIndex].func;
			preHookFunc(&account_id, &char_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.global_accreg_to_login_start(account_id, char_id);
	}
	if( HPMHooks.count.HP_chr_global_accreg_to_login_start_post ) {
		void (*postHookFunc) (int *account_id, int *char_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_global_accreg_to_login_start_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_global_accreg_to_login_start_post[hIndex].func;
			postHookFunc(&account_id, &char_id);
		}
	}
	return;
}
void HP_chr_global_accreg_to_login_send(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_global_accreg_to_login_send_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_global_accreg_to_login_send_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_global_accreg_to_login_send_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.global_accreg_to_login_send();
	}
	if( HPMHooks.count.HP_chr_global_accreg_to_login_send_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_global_accreg_to_login_send_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_global_accreg_to_login_send_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_chr_global_accreg_to_login_add(const char *key, unsigned int index, intptr_t val, bool is_string) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_global_accreg_to_login_add_pre ) {
		void (*preHookFunc) (const char *key, unsigned int *index, intptr_t *val, bool *is_string);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_global_accreg_to_login_add_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_global_accreg_to_login_add_pre[hIndex].func;
			preHookFunc(key, &index, &val, &is_string);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.global_accreg_to_login_add(key, index, val, is_string);
	}
	if( HPMHooks.count.HP_chr_global_accreg_to_login_add_post ) {
		void (*postHookFunc) (const char *key, unsigned int *index, intptr_t *val, bool *is_string);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_global_accreg_to_login_add_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_global_accreg_to_login_add_post[hIndex].func;
			postHookFunc(key, &index, &val, &is_string);
		}
	}
	return;
}
void HP_chr_read_fame_list(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_read_fame_list_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_read_fame_list_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_read_fame_list_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.read_fame_list();
	}
	if( HPMHooks.count.HP_chr_read_fame_list_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_read_fame_list_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_read_fame_list_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_chr_send_fame_list(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_send_fame_list_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_fame_list_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_send_fame_list_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.send_fame_list(fd);
	}
	if( HPMHooks.count.HP_chr_send_fame_list_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_fame_list_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_send_fame_list_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
void HP_chr_update_fame_list(int type, int index, int fame) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_update_fame_list_pre ) {
		void (*preHookFunc) (int *type, int *index, int *fame);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_update_fame_list_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_update_fame_list_pre[hIndex].func;
			preHookFunc(&type, &index, &fame);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.update_fame_list(type, index, fame);
	}
	if( HPMHooks.count.HP_chr_update_fame_list_post ) {
		void (*postHookFunc) (int *type, int *index, int *fame);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_update_fame_list_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_update_fame_list_post[hIndex].func;
			postHookFunc(&type, &index, &fame);
		}
	}
	return;
}
int HP_chr_loadName(int char_id, char *name) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_loadName_pre ) {
		int (*preHookFunc) (int *char_id, char *name);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_loadName_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_loadName_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, name);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.loadName(char_id, name);
	}
	if( HPMHooks.count.HP_chr_loadName_post ) {
		int (*postHookFunc) (int retVal___, int *char_id, char *name);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_loadName_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_loadName_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &char_id, name);
		}
	}
	return retVal___;
}
void HP_chr_parse_frommap_datasync(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_datasync_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_datasync_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_datasync_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_datasync(fd);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_datasync_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_datasync_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_datasync_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_skillid2idx(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_skillid2idx_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_skillid2idx_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_skillid2idx_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_skillid2idx(fd);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_skillid2idx_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_skillid2idx_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_skillid2idx_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_map_received_ok(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_map_received_ok_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_map_received_ok_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_map_received_ok_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.map_received_ok(fd);
	}
	if( HPMHooks.count.HP_chr_map_received_ok_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_map_received_ok_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_map_received_ok_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_send_maps(int fd, int id, int j) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_send_maps_pre ) {
		void (*preHookFunc) (int *fd, int *id, int *j);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_maps_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_send_maps_pre[hIndex].func;
			preHookFunc(&fd, &id, &j);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.send_maps(fd, id, j);
	}
	if( HPMHooks.count.HP_chr_send_maps_post ) {
		void (*postHookFunc) (int *fd, int *id, int *j);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_maps_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_send_maps_post[hIndex].func;
			postHookFunc(&fd, &id, &j);
		}
	}
	return;
}
void HP_chr_parse_frommap_map_names(int fd, int id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_map_names_pre ) {
		void (*preHookFunc) (int *fd, int *id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_map_names_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_map_names_pre[hIndex].func;
			preHookFunc(&fd, &id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_map_names(fd, id);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_map_names_post ) {
		void (*postHookFunc) (int *fd, int *id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_map_names_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_map_names_post[hIndex].func;
			postHookFunc(&fd, &id);
		}
	}
	return;
}
void HP_chr_send_scdata(int fd, int aid, int cid) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_send_scdata_pre ) {
		void (*preHookFunc) (int *fd, int *aid, int *cid);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_scdata_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_send_scdata_pre[hIndex].func;
			preHookFunc(&fd, &aid, &cid);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.send_scdata(fd, aid, cid);
	}
	if( HPMHooks.count.HP_chr_send_scdata_post ) {
		void (*postHookFunc) (int *fd, int *aid, int *cid);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_scdata_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_send_scdata_post[hIndex].func;
			postHookFunc(&fd, &aid, &cid);
		}
	}
	return;
}
void HP_chr_parse_frommap_request_scdata(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_request_scdata_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_request_scdata_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_request_scdata_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_request_scdata(fd);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_request_scdata_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_request_scdata_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_request_scdata_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_set_users_count(int fd, int id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_set_users_count_pre ) {
		void (*preHookFunc) (int *fd, int *id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_set_users_count_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_set_users_count_pre[hIndex].func;
			preHookFunc(&fd, &id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_set_users_count(fd, id);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_set_users_count_post ) {
		void (*postHookFunc) (int *fd, int *id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_set_users_count_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_set_users_count_post[hIndex].func;
			postHookFunc(&fd, &id);
		}
	}
	return;
}
void HP_chr_parse_frommap_set_users(int fd, int id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_set_users_pre ) {
		void (*preHookFunc) (int *fd, int *id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_set_users_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_set_users_pre[hIndex].func;
			preHookFunc(&fd, &id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_set_users(fd, id);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_set_users_post ) {
		void (*postHookFunc) (int *fd, int *id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_set_users_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_set_users_post[hIndex].func;
			postHookFunc(&fd, &id);
		}
	}
	return;
}
void HP_chr_save_character_ack(int fd, int aid, int cid) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_save_character_ack_pre ) {
		void (*preHookFunc) (int *fd, int *aid, int *cid);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_save_character_ack_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_save_character_ack_pre[hIndex].func;
			preHookFunc(&fd, &aid, &cid);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.save_character_ack(fd, aid, cid);
	}
	if( HPMHooks.count.HP_chr_save_character_ack_post ) {
		void (*postHookFunc) (int *fd, int *aid, int *cid);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_save_character_ack_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_save_character_ack_post[hIndex].func;
			postHookFunc(&fd, &aid, &cid);
		}
	}
	return;
}
void HP_chr_parse_frommap_save_character(int fd, int id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_save_character_pre ) {
		void (*preHookFunc) (int *fd, int *id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_save_character_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_save_character_pre[hIndex].func;
			preHookFunc(&fd, &id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_save_character(fd, id);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_save_character_post ) {
		void (*postHookFunc) (int *fd, int *id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_save_character_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_save_character_post[hIndex].func;
			postHookFunc(&fd, &id);
		}
	}
	return;
}
void HP_chr_select_ack(int fd, int account_id, uint8 flag) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_select_ack_pre ) {
		void (*preHookFunc) (int *fd, int *account_id, uint8 *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_select_ack_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_select_ack_pre[hIndex].func;
			preHookFunc(&fd, &account_id, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.select_ack(fd, account_id, flag);
	}
	if( HPMHooks.count.HP_chr_select_ack_post ) {
		void (*postHookFunc) (int *fd, int *account_id, uint8 *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_select_ack_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_select_ack_post[hIndex].func;
			postHookFunc(&fd, &account_id, &flag);
		}
	}
	return;
}
void HP_chr_parse_frommap_char_select_req(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_char_select_req_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_char_select_req_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_char_select_req_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_char_select_req(fd);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_char_select_req_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_char_select_req_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_char_select_req_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_change_map_server_ack(int fd, uint8 *data, bool ok) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_change_map_server_ack_pre ) {
		void (*preHookFunc) (int *fd, uint8 *data, bool *ok);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_change_map_server_ack_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_change_map_server_ack_pre[hIndex].func;
			preHookFunc(&fd, data, &ok);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.change_map_server_ack(fd, data, ok);
	}
	if( HPMHooks.count.HP_chr_change_map_server_ack_post ) {
		void (*postHookFunc) (int *fd, uint8 *data, bool *ok);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_change_map_server_ack_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_change_map_server_ack_post[hIndex].func;
			postHookFunc(&fd, data, &ok);
		}
	}
	return;
}
void HP_chr_parse_frommap_change_map_server(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_change_map_server_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_change_map_server_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_change_map_server_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_change_map_server(fd);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_change_map_server_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_change_map_server_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_change_map_server_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_remove_friend(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_remove_friend_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_remove_friend_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_remove_friend_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_remove_friend(fd);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_remove_friend_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_remove_friend_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_remove_friend_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_char_name_ack(int fd, int char_id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_char_name_ack_pre ) {
		void (*preHookFunc) (int *fd, int *char_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_char_name_ack_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_char_name_ack_pre[hIndex].func;
			preHookFunc(&fd, &char_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.char_name_ack(fd, char_id);
	}
	if( HPMHooks.count.HP_chr_char_name_ack_post ) {
		void (*postHookFunc) (int *fd, int *char_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_char_name_ack_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_char_name_ack_post[hIndex].func;
			postHookFunc(&fd, &char_id);
		}
	}
	return;
}
void HP_chr_parse_frommap_char_name_request(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_char_name_request_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_char_name_request_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_char_name_request_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_char_name_request(fd);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_char_name_request_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_char_name_request_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_char_name_request_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_change_email(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_change_email_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_change_email_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_change_email_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_change_email(fd);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_change_email_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_change_email_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_change_email_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_ban(int account_id, int char_id, time_t *unban_time, short year, short month, short day, short hour, short minute, short second) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_ban_pre ) {
		void (*preHookFunc) (int *account_id, int *char_id, time_t *unban_time, short *year, short *month, short *day, short *hour, short *minute, short *second);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_ban_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_ban_pre[hIndex].func;
			preHookFunc(&account_id, &char_id, unban_time, &year, &month, &day, &hour, &minute, &second);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.ban(account_id, char_id, unban_time, year, month, day, hour, minute, second);
	}
	if( HPMHooks.count.HP_chr_ban_post ) {
		void (*postHookFunc) (int *account_id, int *char_id, time_t *unban_time, short *year, short *month, short *day, short *hour, short *minute, short *second);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_ban_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_ban_post[hIndex].func;
			postHookFunc(&account_id, &char_id, unban_time, &year, &month, &day, &hour, &minute, &second);
		}
	}
	return;
}
void HP_chr_unban(int char_id, int *result) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_unban_pre ) {
		void (*preHookFunc) (int *char_id, int *result);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_unban_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_unban_pre[hIndex].func;
			preHookFunc(&char_id, result);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.unban(char_id, result);
	}
	if( HPMHooks.count.HP_chr_unban_post ) {
		void (*postHookFunc) (int *char_id, int *result);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_unban_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_unban_post[hIndex].func;
			postHookFunc(&char_id, result);
		}
	}
	return;
}
void HP_chr_ask_name_ack(int fd, int acc, const char *name, int type, int result) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_ask_name_ack_pre ) {
		void (*preHookFunc) (int *fd, int *acc, const char *name, int *type, int *result);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_ask_name_ack_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_ask_name_ack_pre[hIndex].func;
			preHookFunc(&fd, &acc, name, &type, &result);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.ask_name_ack(fd, acc, name, type, result);
	}
	if( HPMHooks.count.HP_chr_ask_name_ack_post ) {
		void (*postHookFunc) (int *fd, int *acc, const char *name, int *type, int *result);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_ask_name_ack_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_ask_name_ack_post[hIndex].func;
			postHookFunc(&fd, &acc, name, &type, &result);
		}
	}
	return;
}
void HP_chr_parse_frommap_change_account(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_change_account_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_change_account_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_change_account_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_change_account(fd);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_change_account_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_change_account_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_change_account_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_fame_list(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_fame_list_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_fame_list_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_fame_list_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_fame_list(fd);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_fame_list_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_fame_list_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_fame_list_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_divorce_char(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_divorce_char_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_divorce_char_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_divorce_char_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_divorce_char(fd);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_divorce_char_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_divorce_char_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_divorce_char_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_ragsrvinfo(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_ragsrvinfo_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_ragsrvinfo_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_ragsrvinfo_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_ragsrvinfo(fd);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_ragsrvinfo_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_ragsrvinfo_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_ragsrvinfo_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_set_char_offline(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_set_char_offline_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_set_char_offline_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_set_char_offline_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_set_char_offline(fd);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_set_char_offline_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_set_char_offline_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_set_char_offline_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_set_all_offline(int fd, int id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_set_all_offline_pre ) {
		void (*preHookFunc) (int *fd, int *id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_set_all_offline_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_set_all_offline_pre[hIndex].func;
			preHookFunc(&fd, &id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_set_all_offline(fd, id);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_set_all_offline_post ) {
		void (*postHookFunc) (int *fd, int *id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_set_all_offline_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_set_all_offline_post[hIndex].func;
			postHookFunc(&fd, &id);
		}
	}
	return;
}
void HP_chr_parse_frommap_set_char_online(int fd, int id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_set_char_online_pre ) {
		void (*preHookFunc) (int *fd, int *id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_set_char_online_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_set_char_online_pre[hIndex].func;
			preHookFunc(&fd, &id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_set_char_online(fd, id);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_set_char_online_post ) {
		void (*postHookFunc) (int *fd, int *id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_set_char_online_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_set_char_online_post[hIndex].func;
			postHookFunc(&fd, &id);
		}
	}
	return;
}
void HP_chr_parse_frommap_build_fame_list(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_build_fame_list_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_build_fame_list_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_build_fame_list_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_build_fame_list(fd);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_build_fame_list_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_build_fame_list_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_build_fame_list_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_save_status_change_data(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_save_status_change_data_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_save_status_change_data_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_save_status_change_data_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_save_status_change_data(fd);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_save_status_change_data_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_save_status_change_data_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_save_status_change_data_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_send_pong(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_send_pong_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_pong_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_send_pong_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.send_pong(fd);
	}
	if( HPMHooks.count.HP_chr_send_pong_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_pong_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_send_pong_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_ping(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_ping_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_ping_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_ping_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_ping(fd);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_ping_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_ping_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_ping_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_map_auth_ok(int fd, int account_id, struct char_auth_node *node, struct mmo_charstatus *cd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_map_auth_ok_pre ) {
		void (*preHookFunc) (int *fd, int *account_id, struct char_auth_node *node, struct mmo_charstatus *cd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_map_auth_ok_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_map_auth_ok_pre[hIndex].func;
			preHookFunc(&fd, &account_id, node, cd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.map_auth_ok(fd, account_id, node, cd);
	}
	if( HPMHooks.count.HP_chr_map_auth_ok_post ) {
		void (*postHookFunc) (int *fd, int *account_id, struct char_auth_node *node, struct mmo_charstatus *cd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_map_auth_ok_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_map_auth_ok_post[hIndex].func;
			postHookFunc(&fd, &account_id, node, cd);
		}
	}
	return;
}
void HP_chr_map_auth_failed(int fd, int account_id, int char_id, int login_id1, char sex, uint32 ip) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_map_auth_failed_pre ) {
		void (*preHookFunc) (int *fd, int *account_id, int *char_id, int *login_id1, char *sex, uint32 *ip);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_map_auth_failed_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_map_auth_failed_pre[hIndex].func;
			preHookFunc(&fd, &account_id, &char_id, &login_id1, &sex, &ip);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.map_auth_failed(fd, account_id, char_id, login_id1, sex, ip);
	}
	if( HPMHooks.count.HP_chr_map_auth_failed_post ) {
		void (*postHookFunc) (int *fd, int *account_id, int *char_id, int *login_id1, char *sex, uint32 *ip);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_map_auth_failed_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_map_auth_failed_post[hIndex].func;
			postHookFunc(&fd, &account_id, &char_id, &login_id1, &sex, &ip);
		}
	}
	return;
}
void HP_chr_parse_frommap_auth_request(int fd, int id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_auth_request_pre ) {
		void (*preHookFunc) (int *fd, int *id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_auth_request_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_auth_request_pre[hIndex].func;
			preHookFunc(&fd, &id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_auth_request(fd, id);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_auth_request_post ) {
		void (*postHookFunc) (int *fd, int *id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_auth_request_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_auth_request_post[hIndex].func;
			postHookFunc(&fd, &id);
		}
	}
	return;
}
void HP_chr_parse_frommap_update_ip(int fd, int id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_update_ip_pre ) {
		void (*preHookFunc) (int *fd, int *id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_update_ip_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_update_ip_pre[hIndex].func;
			preHookFunc(&fd, &id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_update_ip(fd, id);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_update_ip_post ) {
		void (*postHookFunc) (int *fd, int *id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_update_ip_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_update_ip_post[hIndex].func;
			postHookFunc(&fd, &id);
		}
	}
	return;
}
void HP_chr_parse_frommap_request_stats_report(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_request_stats_report_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_request_stats_report_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_request_stats_report_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_request_stats_report(fd);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_request_stats_report_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_request_stats_report_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_request_stats_report_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_scdata_update(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_scdata_update_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_scdata_update_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_scdata_update_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_scdata_update(fd);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_scdata_update_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_scdata_update_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_scdata_update_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_frommap_scdata_delete(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_scdata_delete_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_scdata_delete_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_scdata_delete_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_frommap_scdata_delete(fd);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_scdata_delete_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_scdata_delete_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_scdata_delete_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
int HP_chr_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_parse_frommap_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.parse_frommap(fd);
	}
	if( HPMHooks.count.HP_chr_parse_frommap_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_frommap_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
int HP_chr_search_mapserver(unsigned short map, uint32 ip, uint16 port) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_search_mapserver_pre ) {
		int (*preHookFunc) (unsigned short *map, uint32 *ip, uint16 *port);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_search_mapserver_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_search_mapserver_pre[hIndex].func;
			retVal___ = preHookFunc(&map, &ip, &port);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.search_mapserver(map, ip, port);
	}
	if( HPMHooks.count.HP_chr_search_mapserver_post ) {
		int (*postHookFunc) (int retVal___, unsigned short *map, uint32 *ip, uint16 *port);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_search_mapserver_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_search_mapserver_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &map, &ip, &port);
		}
	}
	return retVal___;
}
int HP_chr_mapif_init(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_mapif_init_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_mapif_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_mapif_init_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.mapif_init(fd);
	}
	if( HPMHooks.count.HP_chr_mapif_init_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_mapif_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_mapif_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
int HP_chr_lan_subnetcheck(uint32 ip) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_lan_subnetcheck_pre ) {
		int (*preHookFunc) (uint32 *ip);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_lan_subnetcheck_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_lan_subnetcheck_pre[hIndex].func;
			retVal___ = preHookFunc(&ip);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.lan_subnetcheck(ip);
	}
	if( HPMHooks.count.HP_chr_lan_subnetcheck_post ) {
		int (*postHookFunc) (int retVal___, uint32 *ip);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_lan_subnetcheck_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_lan_subnetcheck_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &ip);
		}
	}
	return retVal___;
}
void HP_chr_delete2_ack(int fd, int char_id, uint32 result, time_t delete_date) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_delete2_ack_pre ) {
		void (*preHookFunc) (int *fd, int *char_id, uint32 *result, time_t *delete_date);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_ack_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_delete2_ack_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &result, &delete_date);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.delete2_ack(fd, char_id, result, delete_date);
	}
	if( HPMHooks.count.HP_chr_delete2_ack_post ) {
		void (*postHookFunc) (int *fd, int *char_id, uint32 *result, time_t *delete_date);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_ack_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_delete2_ack_post[hIndex].func;
			postHookFunc(&fd, &char_id, &result, &delete_date);
		}
	}
	return;
}
void HP_chr_delete2_accept_actual_ack(int fd, int char_id, uint32 result) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_delete2_accept_actual_ack_pre ) {
		void (*preHookFunc) (int *fd, int *char_id, uint32 *result);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_accept_actual_ack_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_delete2_accept_actual_ack_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &result);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.delete2_accept_actual_ack(fd, char_id, result);
	}
	if( HPMHooks.count.HP_chr_delete2_accept_actual_ack_post ) {
		void (*postHookFunc) (int *fd, int *char_id, uint32 *result);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_accept_actual_ack_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_delete2_accept_actual_ack_post[hIndex].func;
			postHookFunc(&fd, &char_id, &result);
		}
	}
	return;
}
void HP_chr_delete2_accept_ack(int fd, int char_id, uint32 result) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_delete2_accept_ack_pre ) {
		void (*preHookFunc) (int *fd, int *char_id, uint32 *result);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_accept_ack_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_delete2_accept_ack_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &result);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.delete2_accept_ack(fd, char_id, result);
	}
	if( HPMHooks.count.HP_chr_delete2_accept_ack_post ) {
		void (*postHookFunc) (int *fd, int *char_id, uint32 *result);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_accept_ack_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_delete2_accept_ack_post[hIndex].func;
			postHookFunc(&fd, &char_id, &result);
		}
	}
	return;
}
void HP_chr_delete2_cancel_ack(int fd, int char_id, uint32 result) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_delete2_cancel_ack_pre ) {
		void (*preHookFunc) (int *fd, int *char_id, uint32 *result);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_cancel_ack_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_delete2_cancel_ack_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &result);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.delete2_cancel_ack(fd, char_id, result);
	}
	if( HPMHooks.count.HP_chr_delete2_cancel_ack_post ) {
		void (*postHookFunc) (int *fd, int *char_id, uint32 *result);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_cancel_ack_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_delete2_cancel_ack_post[hIndex].func;
			postHookFunc(&fd, &char_id, &result);
		}
	}
	return;
}
void HP_chr_delete2_req(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_delete2_req_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_req_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_delete2_req_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.delete2_req(fd, sd);
	}
	if( HPMHooks.count.HP_chr_delete2_req_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_req_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_delete2_req_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
void HP_chr_delete2_accept(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_delete2_accept_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_accept_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_delete2_accept_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.delete2_accept(fd, sd);
	}
	if( HPMHooks.count.HP_chr_delete2_accept_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_accept_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_delete2_accept_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
void HP_chr_delete2_cancel(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_delete2_cancel_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_cancel_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_delete2_cancel_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.delete2_cancel(fd, sd);
	}
	if( HPMHooks.count.HP_chr_delete2_cancel_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete2_cancel_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_delete2_cancel_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
void HP_chr_send_account_id(int fd, int account_id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_send_account_id_pre ) {
		void (*preHookFunc) (int *fd, int *account_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_account_id_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_send_account_id_pre[hIndex].func;
			preHookFunc(&fd, &account_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.send_account_id(fd, account_id);
	}
	if( HPMHooks.count.HP_chr_send_account_id_post ) {
		void (*postHookFunc) (int *fd, int *account_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_account_id_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_send_account_id_post[hIndex].func;
			postHookFunc(&fd, &account_id);
		}
	}
	return;
}
void HP_chr_parse_char_connect(int fd, struct char_session_data *sd, uint32 ipl) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_char_connect_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd, uint32 *ipl);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_connect_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_connect_pre[hIndex].func;
			preHookFunc(&fd, sd, &ipl);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_connect(fd, sd, ipl);
	}
	if( HPMHooks.count.HP_chr_parse_char_connect_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd, uint32 *ipl);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_connect_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_connect_post[hIndex].func;
			postHookFunc(&fd, sd, &ipl);
		}
	}
	return;
}
void HP_chr_send_map_info(int fd, int i, uint32 subnet_map_ip, struct mmo_charstatus *cd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_send_map_info_pre ) {
		void (*preHookFunc) (int *fd, int *i, uint32 *subnet_map_ip, struct mmo_charstatus *cd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_map_info_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_send_map_info_pre[hIndex].func;
			preHookFunc(&fd, &i, &subnet_map_ip, cd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.send_map_info(fd, i, subnet_map_ip, cd);
	}
	if( HPMHooks.count.HP_chr_send_map_info_post ) {
		void (*postHookFunc) (int *fd, int *i, uint32 *subnet_map_ip, struct mmo_charstatus *cd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_map_info_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_send_map_info_post[hIndex].func;
			postHookFunc(&fd, &i, &subnet_map_ip, cd);
		}
	}
	return;
}
void HP_chr_send_wait_char_server(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_send_wait_char_server_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_wait_char_server_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_send_wait_char_server_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.send_wait_char_server(fd);
	}
	if( HPMHooks.count.HP_chr_send_wait_char_server_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_wait_char_server_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_send_wait_char_server_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
int HP_chr_search_default_maps_mapserver(struct mmo_charstatus *cd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_search_default_maps_mapserver_pre ) {
		int (*preHookFunc) (struct mmo_charstatus *cd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_search_default_maps_mapserver_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_search_default_maps_mapserver_pre[hIndex].func;
			retVal___ = preHookFunc(cd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.search_default_maps_mapserver(cd);
	}
	if( HPMHooks.count.HP_chr_search_default_maps_mapserver_post ) {
		int (*postHookFunc) (int retVal___, struct mmo_charstatus *cd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_search_default_maps_mapserver_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_search_default_maps_mapserver_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, cd);
		}
	}
	return retVal___;
}
void HP_chr_parse_char_select(int fd, struct char_session_data *sd, uint32 ipl) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_char_select_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd, uint32 *ipl);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_select_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_select_pre[hIndex].func;
			preHookFunc(&fd, sd, &ipl);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_select(fd, sd, ipl);
	}
	if( HPMHooks.count.HP_chr_parse_char_select_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd, uint32 *ipl);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_select_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_select_post[hIndex].func;
			postHookFunc(&fd, sd, &ipl);
		}
	}
	return;
}
void HP_chr_creation_failed(int fd, int result) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_creation_failed_pre ) {
		void (*preHookFunc) (int *fd, int *result);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_creation_failed_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_creation_failed_pre[hIndex].func;
			preHookFunc(&fd, &result);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.creation_failed(fd, result);
	}
	if( HPMHooks.count.HP_chr_creation_failed_post ) {
		void (*postHookFunc) (int *fd, int *result);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_creation_failed_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_creation_failed_post[hIndex].func;
			postHookFunc(&fd, &result);
		}
	}
	return;
}
void HP_chr_creation_ok(int fd, struct mmo_charstatus *char_dat) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_creation_ok_pre ) {
		void (*preHookFunc) (int *fd, struct mmo_charstatus *char_dat);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_creation_ok_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_creation_ok_pre[hIndex].func;
			preHookFunc(&fd, char_dat);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.creation_ok(fd, char_dat);
	}
	if( HPMHooks.count.HP_chr_creation_ok_post ) {
		void (*postHookFunc) (int *fd, struct mmo_charstatus *char_dat);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_creation_ok_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_creation_ok_post[hIndex].func;
			postHookFunc(&fd, char_dat);
		}
	}
	return;
}
void HP_chr_parse_char_create_new_char(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_char_create_new_char_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_create_new_char_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_create_new_char_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_create_new_char(fd, sd);
	}
	if( HPMHooks.count.HP_chr_parse_char_create_new_char_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_create_new_char_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_create_new_char_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
void HP_chr_delete_char_failed(int fd, int flag) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_delete_char_failed_pre ) {
		void (*preHookFunc) (int *fd, int *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete_char_failed_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_delete_char_failed_pre[hIndex].func;
			preHookFunc(&fd, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.delete_char_failed(fd, flag);
	}
	if( HPMHooks.count.HP_chr_delete_char_failed_post ) {
		void (*postHookFunc) (int *fd, int *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete_char_failed_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_delete_char_failed_post[hIndex].func;
			postHookFunc(&fd, &flag);
		}
	}
	return;
}
void HP_chr_delete_char_ok(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_delete_char_ok_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete_char_ok_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_delete_char_ok_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.delete_char_ok(fd);
	}
	if( HPMHooks.count.HP_chr_delete_char_ok_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_delete_char_ok_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_delete_char_ok_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_char_delete_char(int fd, struct char_session_data *sd, unsigned short cmd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_char_delete_char_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd, unsigned short *cmd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_delete_char_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_delete_char_pre[hIndex].func;
			preHookFunc(&fd, sd, &cmd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_delete_char(fd, sd, cmd);
	}
	if( HPMHooks.count.HP_chr_parse_char_delete_char_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd, unsigned short *cmd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_delete_char_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_delete_char_post[hIndex].func;
			postHookFunc(&fd, sd, &cmd);
		}
	}
	return;
}
void HP_chr_parse_char_ping(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_char_ping_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_ping_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_ping_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_ping(fd);
	}
	if( HPMHooks.count.HP_chr_parse_char_ping_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_ping_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_ping_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_allow_rename(int fd, int flag) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_allow_rename_pre ) {
		void (*preHookFunc) (int *fd, int *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_allow_rename_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_allow_rename_pre[hIndex].func;
			preHookFunc(&fd, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.allow_rename(fd, flag);
	}
	if( HPMHooks.count.HP_chr_allow_rename_post ) {
		void (*postHookFunc) (int *fd, int *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_allow_rename_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_allow_rename_post[hIndex].func;
			postHookFunc(&fd, &flag);
		}
	}
	return;
}
void HP_chr_parse_char_rename_char(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_char_rename_char_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_rename_char_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_rename_char_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_rename_char(fd, sd);
	}
	if( HPMHooks.count.HP_chr_parse_char_rename_char_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_rename_char_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_rename_char_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
void HP_chr_parse_char_rename_char2(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_char_rename_char2_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_rename_char2_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_rename_char2_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_rename_char2(fd, sd);
	}
	if( HPMHooks.count.HP_chr_parse_char_rename_char2_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_rename_char2_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_rename_char2_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
void HP_chr_rename_char_ack(int fd, int flag) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_rename_char_ack_pre ) {
		void (*preHookFunc) (int *fd, int *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_rename_char_ack_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_rename_char_ack_pre[hIndex].func;
			preHookFunc(&fd, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.rename_char_ack(fd, flag);
	}
	if( HPMHooks.count.HP_chr_rename_char_ack_post ) {
		void (*postHookFunc) (int *fd, int *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_rename_char_ack_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_rename_char_ack_post[hIndex].func;
			postHookFunc(&fd, &flag);
		}
	}
	return;
}
void HP_chr_parse_char_rename_char_confirm(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_char_rename_char_confirm_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_rename_char_confirm_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_rename_char_confirm_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_rename_char_confirm(fd, sd);
	}
	if( HPMHooks.count.HP_chr_parse_char_rename_char_confirm_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_rename_char_confirm_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_rename_char_confirm_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
void HP_chr_captcha_notsupported(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_captcha_notsupported_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_captcha_notsupported_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_captcha_notsupported_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.captcha_notsupported(fd);
	}
	if( HPMHooks.count.HP_chr_captcha_notsupported_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_captcha_notsupported_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_captcha_notsupported_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_char_request_captcha(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_char_request_captcha_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_request_captcha_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_request_captcha_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_request_captcha(fd);
	}
	if( HPMHooks.count.HP_chr_parse_char_request_captcha_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_request_captcha_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_request_captcha_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_char_check_captcha(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_char_check_captcha_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_check_captcha_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_check_captcha_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_check_captcha(fd);
	}
	if( HPMHooks.count.HP_chr_parse_char_check_captcha_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_check_captcha_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_check_captcha_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_char_delete2_req(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_char_delete2_req_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_delete2_req_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_delete2_req_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_delete2_req(fd, sd);
	}
	if( HPMHooks.count.HP_chr_parse_char_delete2_req_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_delete2_req_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_delete2_req_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
void HP_chr_parse_char_delete2_accept(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_char_delete2_accept_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_delete2_accept_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_delete2_accept_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_delete2_accept(fd, sd);
	}
	if( HPMHooks.count.HP_chr_parse_char_delete2_accept_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_delete2_accept_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_delete2_accept_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
void HP_chr_parse_char_delete2_cancel(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_char_delete2_cancel_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_delete2_cancel_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_delete2_cancel_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_delete2_cancel(fd, sd);
	}
	if( HPMHooks.count.HP_chr_parse_char_delete2_cancel_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_delete2_cancel_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_delete2_cancel_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
void HP_chr_login_map_server_ack(int fd, uint8 flag) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_login_map_server_ack_pre ) {
		void (*preHookFunc) (int *fd, uint8 *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_login_map_server_ack_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_login_map_server_ack_pre[hIndex].func;
			preHookFunc(&fd, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.login_map_server_ack(fd, flag);
	}
	if( HPMHooks.count.HP_chr_login_map_server_ack_post ) {
		void (*postHookFunc) (int *fd, uint8 *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_login_map_server_ack_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_login_map_server_ack_post[hIndex].func;
			postHookFunc(&fd, &flag);
		}
	}
	return;
}
void HP_chr_parse_char_login_map_server(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_char_login_map_server_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_login_map_server_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_login_map_server_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_login_map_server(fd);
	}
	if( HPMHooks.count.HP_chr_parse_char_login_map_server_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_login_map_server_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_login_map_server_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_chr_parse_char_pincode_check(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_char_pincode_check_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_pincode_check_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_pincode_check_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_pincode_check(fd, sd);
	}
	if( HPMHooks.count.HP_chr_parse_char_pincode_check_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_pincode_check_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_pincode_check_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
void HP_chr_parse_char_pincode_window(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_char_pincode_window_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_pincode_window_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_pincode_window_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_pincode_window(fd, sd);
	}
	if( HPMHooks.count.HP_chr_parse_char_pincode_window_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_pincode_window_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_pincode_window_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
void HP_chr_parse_char_pincode_change(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_char_pincode_change_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_pincode_change_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_pincode_change_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_pincode_change(fd, sd);
	}
	if( HPMHooks.count.HP_chr_parse_char_pincode_change_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_pincode_change_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_pincode_change_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
void HP_chr_parse_char_pincode_first_pin(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_char_pincode_first_pin_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_pincode_first_pin_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_pincode_first_pin_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_pincode_first_pin(fd, sd);
	}
	if( HPMHooks.count.HP_chr_parse_char_pincode_first_pin_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_pincode_first_pin_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_pincode_first_pin_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
void HP_chr_parse_char_request_chars(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_char_request_chars_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_request_chars_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_request_chars_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_request_chars(fd, sd);
	}
	if( HPMHooks.count.HP_chr_parse_char_request_chars_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_request_chars_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_request_chars_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
void HP_chr_change_character_slot_ack(int fd, bool ret) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_change_character_slot_ack_pre ) {
		void (*preHookFunc) (int *fd, bool *ret);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_change_character_slot_ack_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_change_character_slot_ack_pre[hIndex].func;
			preHookFunc(&fd, &ret);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.change_character_slot_ack(fd, ret);
	}
	if( HPMHooks.count.HP_chr_change_character_slot_ack_post ) {
		void (*postHookFunc) (int *fd, bool *ret);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_change_character_slot_ack_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_change_character_slot_ack_post[hIndex].func;
			postHookFunc(&fd, &ret);
		}
	}
	return;
}
void HP_chr_parse_char_move_character(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_parse_char_move_character_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_move_character_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_move_character_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.parse_char_move_character(fd, sd);
	}
	if( HPMHooks.count.HP_chr_parse_char_move_character_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_move_character_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_move_character_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
int HP_chr_parse_char_unknown_packet(int fd, uint32 ipl) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_parse_char_unknown_packet_pre ) {
		int (*preHookFunc) (int *fd, uint32 *ipl);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_unknown_packet_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_unknown_packet_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &ipl);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.parse_char_unknown_packet(fd, ipl);
	}
	if( HPMHooks.count.HP_chr_parse_char_unknown_packet_post ) {
		int (*postHookFunc) (int retVal___, int *fd, uint32 *ipl);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_unknown_packet_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_unknown_packet_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &ipl);
		}
	}
	return retVal___;
}
int HP_chr_parse_char(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_parse_char_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_parse_char_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.parse_char(fd);
	}
	if( HPMHooks.count.HP_chr_parse_char_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_parse_char_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_parse_char_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
int HP_chr_broadcast_user_count(int tid, int64 tick, int id, intptr_t data) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_broadcast_user_count_pre ) {
		int (*preHookFunc) (int *tid, int64 *tick, int *id, intptr_t *data);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_broadcast_user_count_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_broadcast_user_count_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &tick, &id, &data);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.broadcast_user_count(tid, tick, id, data);
	}
	if( HPMHooks.count.HP_chr_broadcast_user_count_post ) {
		int (*postHookFunc) (int retVal___, int *tid, int64 *tick, int *id, intptr_t *data);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_broadcast_user_count_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_broadcast_user_count_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &tid, &tick, &id, &data);
		}
	}
	return retVal___;
}
int HP_chr_send_accounts_tologin_sub(DBKey key, DBData *data, va_list ap) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_send_accounts_tologin_sub_pre ) {
		int (*preHookFunc) (DBKey *key, DBData *data, va_list ap);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_accounts_tologin_sub_pre; hIndex++ ) {
			va_list ap___copy; va_copy(ap___copy, ap);
			preHookFunc = HPMHooks.list.HP_chr_send_accounts_tologin_sub_pre[hIndex].func;
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
		retVal___ = HPMHooks.source.chr.send_accounts_tologin_sub(key, data, ap___copy);
		va_end(ap___copy);
	}
	if( HPMHooks.count.HP_chr_send_accounts_tologin_sub_post ) {
		int (*postHookFunc) (int retVal___, DBKey *key, DBData *data, va_list ap);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_accounts_tologin_sub_post; hIndex++ ) {
			va_list ap___copy; va_copy(ap___copy, ap);
			postHookFunc = HPMHooks.list.HP_chr_send_accounts_tologin_sub_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &key, data, ap___copy);
			va_end(ap___copy);
		}
	}
	return retVal___;
}
int HP_chr_send_accounts_tologin(int tid, int64 tick, int id, intptr_t data) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_send_accounts_tologin_pre ) {
		int (*preHookFunc) (int *tid, int64 *tick, int *id, intptr_t *data);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_accounts_tologin_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_send_accounts_tologin_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &tick, &id, &data);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.send_accounts_tologin(tid, tick, id, data);
	}
	if( HPMHooks.count.HP_chr_send_accounts_tologin_post ) {
		int (*postHookFunc) (int retVal___, int *tid, int64 *tick, int *id, intptr_t *data);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_send_accounts_tologin_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_send_accounts_tologin_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &tid, &tick, &id, &data);
		}
	}
	return retVal___;
}
int HP_chr_check_connect_login_server(int tid, int64 tick, int id, intptr_t data) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_check_connect_login_server_pre ) {
		int (*preHookFunc) (int *tid, int64 *tick, int *id, intptr_t *data);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_check_connect_login_server_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_check_connect_login_server_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &tick, &id, &data);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.check_connect_login_server(tid, tick, id, data);
	}
	if( HPMHooks.count.HP_chr_check_connect_login_server_post ) {
		int (*postHookFunc) (int retVal___, int *tid, int64 *tick, int *id, intptr_t *data);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_check_connect_login_server_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_check_connect_login_server_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &tid, &tick, &id, &data);
		}
	}
	return retVal___;
}
int HP_chr_online_data_cleanup_sub(DBKey key, DBData *data, va_list ap) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_online_data_cleanup_sub_pre ) {
		int (*preHookFunc) (DBKey *key, DBData *data, va_list ap);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_online_data_cleanup_sub_pre; hIndex++ ) {
			va_list ap___copy; va_copy(ap___copy, ap);
			preHookFunc = HPMHooks.list.HP_chr_online_data_cleanup_sub_pre[hIndex].func;
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
		retVal___ = HPMHooks.source.chr.online_data_cleanup_sub(key, data, ap___copy);
		va_end(ap___copy);
	}
	if( HPMHooks.count.HP_chr_online_data_cleanup_sub_post ) {
		int (*postHookFunc) (int retVal___, DBKey *key, DBData *data, va_list ap);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_online_data_cleanup_sub_post; hIndex++ ) {
			va_list ap___copy; va_copy(ap___copy, ap);
			postHookFunc = HPMHooks.list.HP_chr_online_data_cleanup_sub_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &key, data, ap___copy);
			va_end(ap___copy);
		}
	}
	return retVal___;
}
int HP_chr_online_data_cleanup(int tid, int64 tick, int id, intptr_t data) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_online_data_cleanup_pre ) {
		int (*preHookFunc) (int *tid, int64 *tick, int *id, intptr_t *data);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_online_data_cleanup_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_online_data_cleanup_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &tick, &id, &data);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.online_data_cleanup(tid, tick, id, data);
	}
	if( HPMHooks.count.HP_chr_online_data_cleanup_post ) {
		int (*postHookFunc) (int retVal___, int *tid, int64 *tick, int *id, intptr_t *data);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_online_data_cleanup_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_online_data_cleanup_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &tid, &tick, &id, &data);
		}
	}
	return retVal___;
}
int HP_chr_lan_config_read(const char *lancfgName) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_lan_config_read_pre ) {
		int (*preHookFunc) (const char *lancfgName);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_lan_config_read_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_lan_config_read_pre[hIndex].func;
			retVal___ = preHookFunc(lancfgName);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.lan_config_read(lancfgName);
	}
	if( HPMHooks.count.HP_chr_lan_config_read_post ) {
		int (*postHookFunc) (int retVal___, const char *lancfgName);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_lan_config_read_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_lan_config_read_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, lancfgName);
		}
	}
	return retVal___;
}
void HP_chr_sql_config_read(const char *cfgName) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_sql_config_read_pre ) {
		void (*preHookFunc) (const char *cfgName);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_sql_config_read_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_sql_config_read_pre[hIndex].func;
			preHookFunc(cfgName);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.sql_config_read(cfgName);
	}
	if( HPMHooks.count.HP_chr_sql_config_read_post ) {
		void (*postHookFunc) (const char *cfgName);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_sql_config_read_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_sql_config_read_post[hIndex].func;
			postHookFunc(cfgName);
		}
	}
	return;
}
void HP_chr_config_dispatch(char *w1, char *w2) {
	int hIndex = 0;
	if( HPMHooks.count.HP_chr_config_dispatch_pre ) {
		void (*preHookFunc) (char *w1, char *w2);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_dispatch_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_config_dispatch_pre[hIndex].func;
			preHookFunc(w1, w2);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.chr.config_dispatch(w1, w2);
	}
	if( HPMHooks.count.HP_chr_config_dispatch_post ) {
		void (*postHookFunc) (char *w1, char *w2);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_dispatch_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_config_dispatch_post[hIndex].func;
			postHookFunc(w1, w2);
		}
	}
	return;
}
int HP_chr_config_read(const char *cfgName) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_chr_config_read_pre ) {
		int (*preHookFunc) (const char *cfgName);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_read_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_chr_config_read_pre[hIndex].func;
			retVal___ = preHookFunc(cfgName);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.chr.config_read(cfgName);
	}
	if( HPMHooks.count.HP_chr_config_read_post ) {
		int (*postHookFunc) (int retVal___, const char *cfgName);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_chr_config_read_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_chr_config_read_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, cfgName);
		}
	}
	return retVal___;
}
/* geoip */
const char* HP_geoip_getcountry(uint32 ipnum) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if( HPMHooks.count.HP_geoip_getcountry_pre ) {
		const char* (*preHookFunc) (uint32 *ipnum);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_geoip_getcountry_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_geoip_getcountry_pre[hIndex].func;
			retVal___ = preHookFunc(&ipnum);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.geoip.getcountry(ipnum);
	}
	if( HPMHooks.count.HP_geoip_getcountry_post ) {
		const char* (*postHookFunc) (const char* retVal___, uint32 *ipnum);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_geoip_getcountry_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_geoip_getcountry_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &ipnum);
		}
	}
	return retVal___;
}
void HP_geoip_final(bool shutdown) {
	int hIndex = 0;
	if( HPMHooks.count.HP_geoip_final_pre ) {
		void (*preHookFunc) (bool *shutdown);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_geoip_final_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_geoip_final_pre[hIndex].func;
			preHookFunc(&shutdown);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.geoip.final(shutdown);
	}
	if( HPMHooks.count.HP_geoip_final_post ) {
		void (*postHookFunc) (bool *shutdown);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_geoip_final_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_geoip_final_post[hIndex].func;
			postHookFunc(&shutdown);
		}
	}
	return;
}
void HP_geoip_init(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_geoip_init_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_geoip_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_geoip_init_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.geoip.init();
	}
	if( HPMHooks.count.HP_geoip_init_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_geoip_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_geoip_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
/* inter_auction */
int HP_inter_auction_count(int char_id, bool buy) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_auction_count_pre ) {
		int (*preHookFunc) (int *char_id, bool *buy);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_count_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_auction_count_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &buy);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_auction.count(char_id, buy);
	}
	if( HPMHooks.count.HP_inter_auction_count_post ) {
		int (*postHookFunc) (int retVal___, int *char_id, bool *buy);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_count_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_auction_count_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &char_id, &buy);
		}
	}
	return retVal___;
}
void HP_inter_auction_save(struct auction_data *auction) {
	int hIndex = 0;
	if( HPMHooks.count.HP_inter_auction_save_pre ) {
		void (*preHookFunc) (struct auction_data *auction);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_save_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_auction_save_pre[hIndex].func;
			preHookFunc(auction);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_auction.save(auction);
	}
	if( HPMHooks.count.HP_inter_auction_save_post ) {
		void (*postHookFunc) (struct auction_data *auction);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_save_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_auction_save_post[hIndex].func;
			postHookFunc(auction);
		}
	}
	return;
}
unsigned int HP_inter_auction_create(struct auction_data *auction) {
	int hIndex = 0;
	unsigned int retVal___ = 0;
	if( HPMHooks.count.HP_inter_auction_create_pre ) {
		unsigned int (*preHookFunc) (struct auction_data *auction);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_create_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_auction_create_pre[hIndex].func;
			retVal___ = preHookFunc(auction);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_auction.create(auction);
	}
	if( HPMHooks.count.HP_inter_auction_create_post ) {
		unsigned int (*postHookFunc) (unsigned int retVal___, struct auction_data *auction);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_create_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_auction_create_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, auction);
		}
	}
	return retVal___;
}
int HP_inter_auction_end_timer(int tid, int64 tick, int id, intptr_t data) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_auction_end_timer_pre ) {
		int (*preHookFunc) (int *tid, int64 *tick, int *id, intptr_t *data);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_end_timer_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_auction_end_timer_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &tick, &id, &data);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_auction.end_timer(tid, tick, id, data);
	}
	if( HPMHooks.count.HP_inter_auction_end_timer_post ) {
		int (*postHookFunc) (int retVal___, int *tid, int64 *tick, int *id, intptr_t *data);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_end_timer_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_auction_end_timer_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &tid, &tick, &id, &data);
		}
	}
	return retVal___;
}
void HP_inter_auction_delete_(struct auction_data *auction) {
	int hIndex = 0;
	if( HPMHooks.count.HP_inter_auction_delete__pre ) {
		void (*preHookFunc) (struct auction_data *auction);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_delete__pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_auction_delete__pre[hIndex].func;
			preHookFunc(auction);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_auction.delete_(auction);
	}
	if( HPMHooks.count.HP_inter_auction_delete__post ) {
		void (*postHookFunc) (struct auction_data *auction);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_delete__post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_auction_delete__post[hIndex].func;
			postHookFunc(auction);
		}
	}
	return;
}
void HP_inter_auction_fromsql(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_inter_auction_fromsql_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_fromsql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_auction_fromsql_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_auction.fromsql();
	}
	if( HPMHooks.count.HP_inter_auction_fromsql_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_fromsql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_auction_fromsql_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_inter_auction_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_auction_parse_frommap_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_parse_frommap_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_auction_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_auction.parse_frommap(fd);
	}
	if( HPMHooks.count.HP_inter_auction_parse_frommap_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_parse_frommap_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_auction_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
int HP_inter_auction_sql_init(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_auction_sql_init_pre ) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_sql_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_auction_sql_init_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_auction.sql_init();
	}
	if( HPMHooks.count.HP_inter_auction_sql_init_post ) {
		int (*postHookFunc) (int retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_sql_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_auction_sql_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_inter_auction_sql_final(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_inter_auction_sql_final_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_sql_final_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_auction_sql_final_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_auction.sql_final();
	}
	if( HPMHooks.count.HP_inter_auction_sql_final_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_auction_sql_final_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_auction_sql_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
/* inter_elemental */
void HP_inter_elemental_sql_init(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_inter_elemental_sql_init_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_elemental_sql_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_elemental_sql_init_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_elemental.sql_init();
	}
	if( HPMHooks.count.HP_inter_elemental_sql_init_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_elemental_sql_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_elemental_sql_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_inter_elemental_sql_final(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_inter_elemental_sql_final_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_elemental_sql_final_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_elemental_sql_final_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_elemental.sql_final();
	}
	if( HPMHooks.count.HP_inter_elemental_sql_final_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_elemental_sql_final_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_elemental_sql_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_inter_elemental_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_elemental_parse_frommap_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_elemental_parse_frommap_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_elemental_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_elemental.parse_frommap(fd);
	}
	if( HPMHooks.count.HP_inter_elemental_parse_frommap_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_elemental_parse_frommap_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_elemental_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
/* inter_guild */
int HP_inter_guild_save_timer(int tid, int64 tick, int id, intptr_t data) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_guild_save_timer_pre ) {
		int (*preHookFunc) (int *tid, int64 *tick, int *id, intptr_t *data);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_save_timer_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_guild_save_timer_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &tick, &id, &data);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.save_timer(tid, tick, id, data);
	}
	if( HPMHooks.count.HP_inter_guild_save_timer_post ) {
		int (*postHookFunc) (int retVal___, int *tid, int64 *tick, int *id, intptr_t *data);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_save_timer_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_guild_save_timer_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &tid, &tick, &id, &data);
		}
	}
	return retVal___;
}
int HP_inter_guild_removemember_tosql(int account_id, int char_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_guild_removemember_tosql_pre ) {
		int (*preHookFunc) (int *account_id, int *char_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_removemember_tosql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_guild_removemember_tosql_pre[hIndex].func;
			retVal___ = preHookFunc(&account_id, &char_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.removemember_tosql(account_id, char_id);
	}
	if( HPMHooks.count.HP_inter_guild_removemember_tosql_post ) {
		int (*postHookFunc) (int retVal___, int *account_id, int *char_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_removemember_tosql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_guild_removemember_tosql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &account_id, &char_id);
		}
	}
	return retVal___;
}
int HP_inter_guild_tosql(struct guild *g, int flag) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_guild_tosql_pre ) {
		int (*preHookFunc) (struct guild *g, int *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_tosql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_guild_tosql_pre[hIndex].func;
			retVal___ = preHookFunc(g, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.tosql(g, flag);
	}
	if( HPMHooks.count.HP_inter_guild_tosql_post ) {
		int (*postHookFunc) (int retVal___, struct guild *g, int *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_tosql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_guild_tosql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, g, &flag);
		}
	}
	return retVal___;
}
struct guild* HP_inter_guild_fromsql(int guild_id) {
	int hIndex = 0;
	struct guild* retVal___ = NULL;
	if( HPMHooks.count.HP_inter_guild_fromsql_pre ) {
		struct guild* (*preHookFunc) (int *guild_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_fromsql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_guild_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.fromsql(guild_id);
	}
	if( HPMHooks.count.HP_inter_guild_fromsql_post ) {
		struct guild* (*postHookFunc) (struct guild* retVal___, int *guild_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_fromsql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_guild_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &guild_id);
		}
	}
	return retVal___;
}
int HP_inter_guild_castle_tosql(struct guild_castle *gc) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_guild_castle_tosql_pre ) {
		int (*preHookFunc) (struct guild_castle *gc);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_castle_tosql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_guild_castle_tosql_pre[hIndex].func;
			retVal___ = preHookFunc(gc);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.castle_tosql(gc);
	}
	if( HPMHooks.count.HP_inter_guild_castle_tosql_post ) {
		int (*postHookFunc) (int retVal___, struct guild_castle *gc);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_castle_tosql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_guild_castle_tosql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, gc);
		}
	}
	return retVal___;
}
struct guild_castle* HP_inter_guild_castle_fromsql(int castle_id) {
	int hIndex = 0;
	struct guild_castle* retVal___ = NULL;
	if( HPMHooks.count.HP_inter_guild_castle_fromsql_pre ) {
		struct guild_castle* (*preHookFunc) (int *castle_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_castle_fromsql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_guild_castle_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(&castle_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.castle_fromsql(castle_id);
	}
	if( HPMHooks.count.HP_inter_guild_castle_fromsql_post ) {
		struct guild_castle* (*postHookFunc) (struct guild_castle* retVal___, int *castle_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_castle_fromsql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_guild_castle_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &castle_id);
		}
	}
	return retVal___;
}
bool HP_inter_guild_exp_parse_row(char *split[], int column, int current) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_inter_guild_exp_parse_row_pre ) {
		bool (*preHookFunc) (char *split[], int *column, int *current);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_exp_parse_row_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_guild_exp_parse_row_pre[hIndex].func;
			retVal___ = preHookFunc(split, &column, &current);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.exp_parse_row(split, column, current);
	}
	if( HPMHooks.count.HP_inter_guild_exp_parse_row_post ) {
		bool (*postHookFunc) (bool retVal___, char *split[], int *column, int *current);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_exp_parse_row_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_guild_exp_parse_row_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, split, &column, &current);
		}
	}
	return retVal___;
}
int HP_inter_guild_CharOnline(int char_id, int guild_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_guild_CharOnline_pre ) {
		int (*preHookFunc) (int *char_id, int *guild_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_CharOnline_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_guild_CharOnline_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &guild_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.CharOnline(char_id, guild_id);
	}
	if( HPMHooks.count.HP_inter_guild_CharOnline_post ) {
		int (*postHookFunc) (int retVal___, int *char_id, int *guild_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_CharOnline_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_guild_CharOnline_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &char_id, &guild_id);
		}
	}
	return retVal___;
}
int HP_inter_guild_CharOffline(int char_id, int guild_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_guild_CharOffline_pre ) {
		int (*preHookFunc) (int *char_id, int *guild_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_CharOffline_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_guild_CharOffline_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &guild_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.CharOffline(char_id, guild_id);
	}
	if( HPMHooks.count.HP_inter_guild_CharOffline_post ) {
		int (*postHookFunc) (int retVal___, int *char_id, int *guild_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_CharOffline_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_guild_CharOffline_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &char_id, &guild_id);
		}
	}
	return retVal___;
}
int HP_inter_guild_sql_init(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_guild_sql_init_pre ) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_sql_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_guild_sql_init_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.sql_init();
	}
	if( HPMHooks.count.HP_inter_guild_sql_init_post ) {
		int (*postHookFunc) (int retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_sql_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_guild_sql_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
int HP_inter_guild_db_final(DBKey key, DBData *data, va_list ap) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_guild_db_final_pre ) {
		int (*preHookFunc) (DBKey *key, DBData *data, va_list ap);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_db_final_pre; hIndex++ ) {
			va_list ap___copy; va_copy(ap___copy, ap);
			preHookFunc = HPMHooks.list.HP_inter_guild_db_final_pre[hIndex].func;
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
		retVal___ = HPMHooks.source.inter_guild.db_final(key, data, ap___copy);
		va_end(ap___copy);
	}
	if( HPMHooks.count.HP_inter_guild_db_final_post ) {
		int (*postHookFunc) (int retVal___, DBKey *key, DBData *data, va_list ap);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_db_final_post; hIndex++ ) {
			va_list ap___copy; va_copy(ap___copy, ap);
			postHookFunc = HPMHooks.list.HP_inter_guild_db_final_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &key, data, ap___copy);
			va_end(ap___copy);
		}
	}
	return retVal___;
}
void HP_inter_guild_sql_final(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_inter_guild_sql_final_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_sql_final_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_guild_sql_final_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_guild.sql_final();
	}
	if( HPMHooks.count.HP_inter_guild_sql_final_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_sql_final_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_guild_sql_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_inter_guild_search_guildname(char *str) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_guild_search_guildname_pre ) {
		int (*preHookFunc) (char *str);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_search_guildname_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_guild_search_guildname_pre[hIndex].func;
			retVal___ = preHookFunc(str);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.search_guildname(str);
	}
	if( HPMHooks.count.HP_inter_guild_search_guildname_post ) {
		int (*postHookFunc) (int retVal___, char *str);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_search_guildname_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_guild_search_guildname_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, str);
		}
	}
	return retVal___;
}
bool HP_inter_guild_check_empty(struct guild *g) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_inter_guild_check_empty_pre ) {
		bool (*preHookFunc) (struct guild *g);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_check_empty_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_guild_check_empty_pre[hIndex].func;
			retVal___ = preHookFunc(g);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.check_empty(g);
	}
	if( HPMHooks.count.HP_inter_guild_check_empty_post ) {
		bool (*postHookFunc) (bool retVal___, struct guild *g);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_check_empty_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_guild_check_empty_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, g);
		}
	}
	return retVal___;
}
unsigned int HP_inter_guild_nextexp(int level) {
	int hIndex = 0;
	unsigned int retVal___ = 0;
	if( HPMHooks.count.HP_inter_guild_nextexp_pre ) {
		unsigned int (*preHookFunc) (int *level);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_nextexp_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_guild_nextexp_pre[hIndex].func;
			retVal___ = preHookFunc(&level);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.nextexp(level);
	}
	if( HPMHooks.count.HP_inter_guild_nextexp_post ) {
		unsigned int (*postHookFunc) (unsigned int retVal___, int *level);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_nextexp_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_guild_nextexp_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &level);
		}
	}
	return retVal___;
}
int HP_inter_guild_checkskill(struct guild *g, int id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_guild_checkskill_pre ) {
		int (*preHookFunc) (struct guild *g, int *id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_checkskill_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_guild_checkskill_pre[hIndex].func;
			retVal___ = preHookFunc(g, &id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.checkskill(g, id);
	}
	if( HPMHooks.count.HP_inter_guild_checkskill_post ) {
		int (*postHookFunc) (int retVal___, struct guild *g, int *id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_checkskill_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_guild_checkskill_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, g, &id);
		}
	}
	return retVal___;
}
int HP_inter_guild_calcinfo(struct guild *g) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_guild_calcinfo_pre ) {
		int (*preHookFunc) (struct guild *g);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_calcinfo_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_guild_calcinfo_pre[hIndex].func;
			retVal___ = preHookFunc(g);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.calcinfo(g);
	}
	if( HPMHooks.count.HP_inter_guild_calcinfo_post ) {
		int (*postHookFunc) (int retVal___, struct guild *g);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_calcinfo_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_guild_calcinfo_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, g);
		}
	}
	return retVal___;
}
int HP_inter_guild_sex_changed(int guild_id, int account_id, int char_id, short gender) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_guild_sex_changed_pre ) {
		int (*preHookFunc) (int *guild_id, int *account_id, int *char_id, short *gender);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_sex_changed_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_guild_sex_changed_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id, &account_id, &char_id, &gender);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.sex_changed(guild_id, account_id, char_id, gender);
	}
	if( HPMHooks.count.HP_inter_guild_sex_changed_post ) {
		int (*postHookFunc) (int retVal___, int *guild_id, int *account_id, int *char_id, short *gender);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_sex_changed_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_guild_sex_changed_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &guild_id, &account_id, &char_id, &gender);
		}
	}
	return retVal___;
}
int HP_inter_guild_charname_changed(int guild_id, int account_id, int char_id, char *name) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_guild_charname_changed_pre ) {
		int (*preHookFunc) (int *guild_id, int *account_id, int *char_id, char *name);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_charname_changed_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_guild_charname_changed_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id, &account_id, &char_id, name);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.charname_changed(guild_id, account_id, char_id, name);
	}
	if( HPMHooks.count.HP_inter_guild_charname_changed_post ) {
		int (*postHookFunc) (int retVal___, int *guild_id, int *account_id, int *char_id, char *name);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_charname_changed_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_guild_charname_changed_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &guild_id, &account_id, &char_id, name);
		}
	}
	return retVal___;
}
int HP_inter_guild_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_guild_parse_frommap_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_parse_frommap_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_guild_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.parse_frommap(fd);
	}
	if( HPMHooks.count.HP_inter_guild_parse_frommap_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_parse_frommap_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_guild_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
int HP_inter_guild_leave(int guild_id, int account_id, int char_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_guild_leave_pre ) {
		int (*preHookFunc) (int *guild_id, int *account_id, int *char_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_leave_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_guild_leave_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id, &account_id, &char_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.leave(guild_id, account_id, char_id);
	}
	if( HPMHooks.count.HP_inter_guild_leave_post ) {
		int (*postHookFunc) (int retVal___, int *guild_id, int *account_id, int *char_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_leave_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_guild_leave_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &guild_id, &account_id, &char_id);
		}
	}
	return retVal___;
}
int HP_inter_guild_broken(int guild_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_guild_broken_pre ) {
		int (*preHookFunc) (int *guild_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_broken_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_guild_broken_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_guild.broken(guild_id);
	}
	if( HPMHooks.count.HP_inter_guild_broken_post ) {
		int (*postHookFunc) (int retVal___, int *guild_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_guild_broken_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_guild_broken_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &guild_id);
		}
	}
	return retVal___;
}
/* inter_homunculus */
int HP_inter_homunculus_sql_init(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_homunculus_sql_init_pre ) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_homunculus_sql_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_homunculus_sql_init_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_homunculus.sql_init();
	}
	if( HPMHooks.count.HP_inter_homunculus_sql_init_post ) {
		int (*postHookFunc) (int retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_homunculus_sql_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_homunculus_sql_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_inter_homunculus_sql_final(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_inter_homunculus_sql_final_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_homunculus_sql_final_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_homunculus_sql_final_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_homunculus.sql_final();
	}
	if( HPMHooks.count.HP_inter_homunculus_sql_final_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_homunculus_sql_final_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_homunculus_sql_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_inter_homunculus_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_homunculus_parse_frommap_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_homunculus_parse_frommap_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_homunculus_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_homunculus.parse_frommap(fd);
	}
	if( HPMHooks.count.HP_inter_homunculus_parse_frommap_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_homunculus_parse_frommap_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_homunculus_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
/* inter */
const char* HP_inter_msg_txt(int msg_number) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if( HPMHooks.count.HP_inter_msg_txt_pre ) {
		const char* (*preHookFunc) (int *msg_number);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_msg_txt_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_msg_txt_pre[hIndex].func;
			retVal___ = preHookFunc(&msg_number);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter.msg_txt(msg_number);
	}
	if( HPMHooks.count.HP_inter_msg_txt_post ) {
		const char* (*postHookFunc) (const char* retVal___, int *msg_number);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_msg_txt_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_msg_txt_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &msg_number);
		}
	}
	return retVal___;
}
bool HP_inter_msg_config_read(const char *cfg_name, bool allow_override) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_inter_msg_config_read_pre ) {
		bool (*preHookFunc) (const char *cfg_name, bool *allow_override);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_msg_config_read_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_msg_config_read_pre[hIndex].func;
			retVal___ = preHookFunc(cfg_name, &allow_override);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter.msg_config_read(cfg_name, allow_override);
	}
	if( HPMHooks.count.HP_inter_msg_config_read_post ) {
		bool (*postHookFunc) (bool retVal___, const char *cfg_name, bool *allow_override);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_msg_config_read_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_msg_config_read_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, cfg_name, &allow_override);
		}
	}
	return retVal___;
}
void HP_inter_do_final_msg(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_inter_do_final_msg_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_do_final_msg_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_do_final_msg_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter.do_final_msg();
	}
	if( HPMHooks.count.HP_inter_do_final_msg_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_do_final_msg_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_do_final_msg_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
const char* HP_inter_job_name(int class_) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if( HPMHooks.count.HP_inter_job_name_pre ) {
		const char* (*preHookFunc) (int *class_);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_job_name_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_job_name_pre[hIndex].func;
			retVal___ = preHookFunc(&class_);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter.job_name(class_);
	}
	if( HPMHooks.count.HP_inter_job_name_post ) {
		const char* (*postHookFunc) (const char* retVal___, int *class_);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_job_name_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_job_name_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &class_);
		}
	}
	return retVal___;
}
void HP_inter_vmsg_to_fd(int fd, int u_fd, int aid, char *msg, va_list ap) {
	int hIndex = 0;
	if( HPMHooks.count.HP_inter_vmsg_to_fd_pre ) {
		void (*preHookFunc) (int *fd, int *u_fd, int *aid, char *msg, va_list ap);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_vmsg_to_fd_pre; hIndex++ ) {
			va_list ap___copy; va_copy(ap___copy, ap);
			preHookFunc = HPMHooks.list.HP_inter_vmsg_to_fd_pre[hIndex].func;
			preHookFunc(&fd, &u_fd, &aid, msg, ap___copy);
			va_end(ap___copy);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		va_list ap___copy; va_copy(ap___copy, ap);
		HPMHooks.source.inter.vmsg_to_fd(fd, u_fd, aid, msg, ap___copy);
		va_end(ap___copy);
	}
	if( HPMHooks.count.HP_inter_vmsg_to_fd_post ) {
		void (*postHookFunc) (int *fd, int *u_fd, int *aid, char *msg, va_list ap);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_vmsg_to_fd_post; hIndex++ ) {
			va_list ap___copy; va_copy(ap___copy, ap);
			postHookFunc = HPMHooks.list.HP_inter_vmsg_to_fd_post[hIndex].func;
			postHookFunc(&fd, &u_fd, &aid, msg, ap___copy);
			va_end(ap___copy);
		}
	}
	return;
}
void HP_inter_savereg(int account_id, int char_id, const char *key, unsigned int index, intptr_t val, bool is_string) {
	int hIndex = 0;
	if( HPMHooks.count.HP_inter_savereg_pre ) {
		void (*preHookFunc) (int *account_id, int *char_id, const char *key, unsigned int *index, intptr_t *val, bool *is_string);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_savereg_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_savereg_pre[hIndex].func;
			preHookFunc(&account_id, &char_id, key, &index, &val, &is_string);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter.savereg(account_id, char_id, key, index, val, is_string);
	}
	if( HPMHooks.count.HP_inter_savereg_post ) {
		void (*postHookFunc) (int *account_id, int *char_id, const char *key, unsigned int *index, intptr_t *val, bool *is_string);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_savereg_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_savereg_post[hIndex].func;
			postHookFunc(&account_id, &char_id, key, &index, &val, &is_string);
		}
	}
	return;
}
int HP_inter_accreg_fromsql(int account_id, int char_id, int fd, int type) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_accreg_fromsql_pre ) {
		int (*preHookFunc) (int *account_id, int *char_id, int *fd, int *type);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_accreg_fromsql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_accreg_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(&account_id, &char_id, &fd, &type);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter.accreg_fromsql(account_id, char_id, fd, type);
	}
	if( HPMHooks.count.HP_inter_accreg_fromsql_post ) {
		int (*postHookFunc) (int retVal___, int *account_id, int *char_id, int *fd, int *type);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_accreg_fromsql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_accreg_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &account_id, &char_id, &fd, &type);
		}
	}
	return retVal___;
}
int HP_inter_config_read(const char *cfgName) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_config_read_pre ) {
		int (*preHookFunc) (const char *cfgName);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_config_read_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_config_read_pre[hIndex].func;
			retVal___ = preHookFunc(cfgName);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter.config_read(cfgName);
	}
	if( HPMHooks.count.HP_inter_config_read_post ) {
		int (*postHookFunc) (int retVal___, const char *cfgName);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_config_read_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_config_read_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, cfgName);
		}
	}
	return retVal___;
}
int HP_inter_vlog(char *fmt, va_list ap) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_vlog_pre ) {
		int (*preHookFunc) (char *fmt, va_list ap);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_vlog_pre; hIndex++ ) {
			va_list ap___copy; va_copy(ap___copy, ap);
			preHookFunc = HPMHooks.list.HP_inter_vlog_pre[hIndex].func;
			retVal___ = preHookFunc(fmt, ap___copy);
			va_end(ap___copy);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		va_list ap___copy; va_copy(ap___copy, ap);
		retVal___ = HPMHooks.source.inter.vlog(fmt, ap___copy);
		va_end(ap___copy);
	}
	if( HPMHooks.count.HP_inter_vlog_post ) {
		int (*postHookFunc) (int retVal___, char *fmt, va_list ap);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_vlog_post; hIndex++ ) {
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
	if( HPMHooks.count.HP_inter_init_sql_pre ) {
		int (*preHookFunc) (const char *file);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_init_sql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_init_sql_pre[hIndex].func;
			retVal___ = preHookFunc(file);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter.init_sql(file);
	}
	if( HPMHooks.count.HP_inter_init_sql_post ) {
		int (*postHookFunc) (int retVal___, const char *file);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_init_sql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_init_sql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, file);
		}
	}
	return retVal___;
}
int HP_inter_mapif_init(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_mapif_init_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mapif_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_mapif_init_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter.mapif_init(fd);
	}
	if( HPMHooks.count.HP_inter_mapif_init_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mapif_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_mapif_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
int HP_inter_check_ttl_wisdata_sub(DBKey key, DBData *data, va_list ap) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_check_ttl_wisdata_sub_pre ) {
		int (*preHookFunc) (DBKey *key, DBData *data, va_list ap);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_check_ttl_wisdata_sub_pre; hIndex++ ) {
			va_list ap___copy; va_copy(ap___copy, ap);
			preHookFunc = HPMHooks.list.HP_inter_check_ttl_wisdata_sub_pre[hIndex].func;
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
		retVal___ = HPMHooks.source.inter.check_ttl_wisdata_sub(key, data, ap___copy);
		va_end(ap___copy);
	}
	if( HPMHooks.count.HP_inter_check_ttl_wisdata_sub_post ) {
		int (*postHookFunc) (int retVal___, DBKey *key, DBData *data, va_list ap);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_check_ttl_wisdata_sub_post; hIndex++ ) {
			va_list ap___copy; va_copy(ap___copy, ap);
			postHookFunc = HPMHooks.list.HP_inter_check_ttl_wisdata_sub_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &key, data, ap___copy);
			va_end(ap___copy);
		}
	}
	return retVal___;
}
int HP_inter_check_ttl_wisdata(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_check_ttl_wisdata_pre ) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_check_ttl_wisdata_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_check_ttl_wisdata_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter.check_ttl_wisdata();
	}
	if( HPMHooks.count.HP_inter_check_ttl_wisdata_post ) {
		int (*postHookFunc) (int retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_check_ttl_wisdata_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_check_ttl_wisdata_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
int HP_inter_check_length(int fd, int length) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_check_length_pre ) {
		int (*preHookFunc) (int *fd, int *length);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_check_length_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_check_length_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &length);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter.check_length(fd, length);
	}
	if( HPMHooks.count.HP_inter_check_length_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *length);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_check_length_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_check_length_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &length);
		}
	}
	return retVal___;
}
int HP_inter_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_parse_frommap_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_parse_frommap_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter.parse_frommap(fd);
	}
	if( HPMHooks.count.HP_inter_parse_frommap_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_parse_frommap_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
void HP_inter_final(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_inter_final_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_final_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_final_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter.final();
	}
	if( HPMHooks.count.HP_inter_final_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_final_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
/* inter_mail */
int HP_inter_mail_sql_init(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_mail_sql_init_pre ) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_sql_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_mail_sql_init_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_mail.sql_init();
	}
	if( HPMHooks.count.HP_inter_mail_sql_init_post ) {
		int (*postHookFunc) (int retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_sql_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_mail_sql_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_inter_mail_sql_final(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_inter_mail_sql_final_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_sql_final_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_mail_sql_final_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_mail.sql_final();
	}
	if( HPMHooks.count.HP_inter_mail_sql_final_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_sql_final_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_mail_sql_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_inter_mail_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_mail_parse_frommap_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_parse_frommap_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_mail_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_mail.parse_frommap(fd);
	}
	if( HPMHooks.count.HP_inter_mail_parse_frommap_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_parse_frommap_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_mail_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
int HP_inter_mail_fromsql(int char_id, struct mail_data *md) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_mail_fromsql_pre ) {
		int (*preHookFunc) (int *char_id, struct mail_data *md);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_fromsql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_mail_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, md);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_mail.fromsql(char_id, md);
	}
	if( HPMHooks.count.HP_inter_mail_fromsql_post ) {
		int (*postHookFunc) (int retVal___, int *char_id, struct mail_data *md);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_fromsql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_mail_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &char_id, md);
		}
	}
	return retVal___;
}
int HP_inter_mail_savemessage(struct mail_message *msg) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_mail_savemessage_pre ) {
		int (*preHookFunc) (struct mail_message *msg);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_savemessage_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_mail_savemessage_pre[hIndex].func;
			retVal___ = preHookFunc(msg);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_mail.savemessage(msg);
	}
	if( HPMHooks.count.HP_inter_mail_savemessage_post ) {
		int (*postHookFunc) (int retVal___, struct mail_message *msg);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_savemessage_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_mail_savemessage_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, msg);
		}
	}
	return retVal___;
}
bool HP_inter_mail_loadmessage(int mail_id, struct mail_message *msg) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_inter_mail_loadmessage_pre ) {
		bool (*preHookFunc) (int *mail_id, struct mail_message *msg);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_loadmessage_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_mail_loadmessage_pre[hIndex].func;
			retVal___ = preHookFunc(&mail_id, msg);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_mail.loadmessage(mail_id, msg);
	}
	if( HPMHooks.count.HP_inter_mail_loadmessage_post ) {
		bool (*postHookFunc) (bool retVal___, int *mail_id, struct mail_message *msg);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_loadmessage_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_mail_loadmessage_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &mail_id, msg);
		}
	}
	return retVal___;
}
bool HP_inter_mail_DeleteAttach(int mail_id) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_inter_mail_DeleteAttach_pre ) {
		bool (*preHookFunc) (int *mail_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_DeleteAttach_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_mail_DeleteAttach_pre[hIndex].func;
			retVal___ = preHookFunc(&mail_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_mail.DeleteAttach(mail_id);
	}
	if( HPMHooks.count.HP_inter_mail_DeleteAttach_post ) {
		bool (*postHookFunc) (bool retVal___, int *mail_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_DeleteAttach_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_mail_DeleteAttach_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &mail_id);
		}
	}
	return retVal___;
}
void HP_inter_mail_sendmail(int send_id, const char *send_name, int dest_id, const char *dest_name, const char *title, const char *body, int zeny, struct item *item) {
	int hIndex = 0;
	if( HPMHooks.count.HP_inter_mail_sendmail_pre ) {
		void (*preHookFunc) (int *send_id, const char *send_name, int *dest_id, const char *dest_name, const char *title, const char *body, int *zeny, struct item *item);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_sendmail_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_mail_sendmail_pre[hIndex].func;
			preHookFunc(&send_id, send_name, &dest_id, dest_name, title, body, &zeny, item);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_mail.sendmail(send_id, send_name, dest_id, dest_name, title, body, zeny, item);
	}
	if( HPMHooks.count.HP_inter_mail_sendmail_post ) {
		void (*postHookFunc) (int *send_id, const char *send_name, int *dest_id, const char *dest_name, const char *title, const char *body, int *zeny, struct item *item);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mail_sendmail_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_mail_sendmail_post[hIndex].func;
			postHookFunc(&send_id, send_name, &dest_id, dest_name, title, body, &zeny, item);
		}
	}
	return;
}
/* inter_mercenary */
bool HP_inter_mercenary_owner_fromsql(int char_id, struct mmo_charstatus *status) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_inter_mercenary_owner_fromsql_pre ) {
		bool (*preHookFunc) (int *char_id, struct mmo_charstatus *status);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mercenary_owner_fromsql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_mercenary_owner_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, status);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_mercenary.owner_fromsql(char_id, status);
	}
	if( HPMHooks.count.HP_inter_mercenary_owner_fromsql_post ) {
		bool (*postHookFunc) (bool retVal___, int *char_id, struct mmo_charstatus *status);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mercenary_owner_fromsql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_mercenary_owner_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &char_id, status);
		}
	}
	return retVal___;
}
bool HP_inter_mercenary_owner_tosql(int char_id, struct mmo_charstatus *status) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_inter_mercenary_owner_tosql_pre ) {
		bool (*preHookFunc) (int *char_id, struct mmo_charstatus *status);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mercenary_owner_tosql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_mercenary_owner_tosql_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, status);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_mercenary.owner_tosql(char_id, status);
	}
	if( HPMHooks.count.HP_inter_mercenary_owner_tosql_post ) {
		bool (*postHookFunc) (bool retVal___, int *char_id, struct mmo_charstatus *status);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mercenary_owner_tosql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_mercenary_owner_tosql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &char_id, status);
		}
	}
	return retVal___;
}
bool HP_inter_mercenary_owner_delete(int char_id) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_inter_mercenary_owner_delete_pre ) {
		bool (*preHookFunc) (int *char_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mercenary_owner_delete_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_mercenary_owner_delete_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_mercenary.owner_delete(char_id);
	}
	if( HPMHooks.count.HP_inter_mercenary_owner_delete_post ) {
		bool (*postHookFunc) (bool retVal___, int *char_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mercenary_owner_delete_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_mercenary_owner_delete_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &char_id);
		}
	}
	return retVal___;
}
int HP_inter_mercenary_sql_init(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_mercenary_sql_init_pre ) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mercenary_sql_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_mercenary_sql_init_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_mercenary.sql_init();
	}
	if( HPMHooks.count.HP_inter_mercenary_sql_init_post ) {
		int (*postHookFunc) (int retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mercenary_sql_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_mercenary_sql_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_inter_mercenary_sql_final(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_inter_mercenary_sql_final_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mercenary_sql_final_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_mercenary_sql_final_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_mercenary.sql_final();
	}
	if( HPMHooks.count.HP_inter_mercenary_sql_final_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mercenary_sql_final_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_mercenary_sql_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_inter_mercenary_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_mercenary_parse_frommap_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mercenary_parse_frommap_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_mercenary_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_mercenary.parse_frommap(fd);
	}
	if( HPMHooks.count.HP_inter_mercenary_parse_frommap_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_mercenary_parse_frommap_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_mercenary_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
/* inter_party */
int HP_inter_party_check_lv(struct party_data *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_party_check_lv_pre ) {
		int (*preHookFunc) (struct party_data *p);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_check_lv_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_party_check_lv_pre[hIndex].func;
			retVal___ = preHookFunc(p);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_party.check_lv(p);
	}
	if( HPMHooks.count.HP_inter_party_check_lv_post ) {
		int (*postHookFunc) (int retVal___, struct party_data *p);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_check_lv_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_party_check_lv_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, p);
		}
	}
	return retVal___;
}
void HP_inter_party_calc_state(struct party_data *p) {
	int hIndex = 0;
	if( HPMHooks.count.HP_inter_party_calc_state_pre ) {
		void (*preHookFunc) (struct party_data *p);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_calc_state_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_party_calc_state_pre[hIndex].func;
			preHookFunc(p);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_party.calc_state(p);
	}
	if( HPMHooks.count.HP_inter_party_calc_state_post ) {
		void (*postHookFunc) (struct party_data *p);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_calc_state_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_party_calc_state_post[hIndex].func;
			postHookFunc(p);
		}
	}
	return;
}
int HP_inter_party_tosql(struct party *p, int flag, int index) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_party_tosql_pre ) {
		int (*preHookFunc) (struct party *p, int *flag, int *index);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_tosql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_party_tosql_pre[hIndex].func;
			retVal___ = preHookFunc(p, &flag, &index);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_party.tosql(p, flag, index);
	}
	if( HPMHooks.count.HP_inter_party_tosql_post ) {
		int (*postHookFunc) (int retVal___, struct party *p, int *flag, int *index);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_tosql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_party_tosql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, p, &flag, &index);
		}
	}
	return retVal___;
}
struct party_data* HP_inter_party_fromsql(int party_id) {
	int hIndex = 0;
	struct party_data* retVal___ = NULL;
	if( HPMHooks.count.HP_inter_party_fromsql_pre ) {
		struct party_data* (*preHookFunc) (int *party_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_fromsql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_party_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(&party_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_party.fromsql(party_id);
	}
	if( HPMHooks.count.HP_inter_party_fromsql_post ) {
		struct party_data* (*postHookFunc) (struct party_data* retVal___, int *party_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_fromsql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_party_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &party_id);
		}
	}
	return retVal___;
}
int HP_inter_party_sql_init(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_party_sql_init_pre ) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_sql_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_party_sql_init_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_party.sql_init();
	}
	if( HPMHooks.count.HP_inter_party_sql_init_post ) {
		int (*postHookFunc) (int retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_sql_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_party_sql_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_inter_party_sql_final(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_inter_party_sql_final_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_sql_final_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_party_sql_final_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_party.sql_final();
	}
	if( HPMHooks.count.HP_inter_party_sql_final_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_sql_final_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_party_sql_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
struct party_data* HP_inter_party_search_partyname(const char *str) {
	int hIndex = 0;
	struct party_data* retVal___ = NULL;
	if( HPMHooks.count.HP_inter_party_search_partyname_pre ) {
		struct party_data* (*preHookFunc) (const char *str);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_search_partyname_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_party_search_partyname_pre[hIndex].func;
			retVal___ = preHookFunc(str);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_party.search_partyname(str);
	}
	if( HPMHooks.count.HP_inter_party_search_partyname_post ) {
		struct party_data* (*postHookFunc) (struct party_data* retVal___, const char *str);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_search_partyname_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_party_search_partyname_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, str);
		}
	}
	return retVal___;
}
int HP_inter_party_check_exp_share(struct party_data *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_party_check_exp_share_pre ) {
		int (*preHookFunc) (struct party_data *p);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_check_exp_share_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_party_check_exp_share_pre[hIndex].func;
			retVal___ = preHookFunc(p);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_party.check_exp_share(p);
	}
	if( HPMHooks.count.HP_inter_party_check_exp_share_post ) {
		int (*postHookFunc) (int retVal___, struct party_data *p);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_check_exp_share_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_party_check_exp_share_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, p);
		}
	}
	return retVal___;
}
int HP_inter_party_check_empty(struct party_data *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_party_check_empty_pre ) {
		int (*preHookFunc) (struct party_data *p);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_check_empty_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_party_check_empty_pre[hIndex].func;
			retVal___ = preHookFunc(p);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_party.check_empty(p);
	}
	if( HPMHooks.count.HP_inter_party_check_empty_post ) {
		int (*postHookFunc) (int retVal___, struct party_data *p);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_check_empty_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_party_check_empty_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, p);
		}
	}
	return retVal___;
}
int HP_inter_party_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_party_parse_frommap_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_parse_frommap_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_party_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_party.parse_frommap(fd);
	}
	if( HPMHooks.count.HP_inter_party_parse_frommap_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_parse_frommap_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_party_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
int HP_inter_party_leave(int party_id, int account_id, int char_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_party_leave_pre ) {
		int (*preHookFunc) (int *party_id, int *account_id, int *char_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_leave_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_party_leave_pre[hIndex].func;
			retVal___ = preHookFunc(&party_id, &account_id, &char_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_party.leave(party_id, account_id, char_id);
	}
	if( HPMHooks.count.HP_inter_party_leave_post ) {
		int (*postHookFunc) (int retVal___, int *party_id, int *account_id, int *char_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_leave_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_party_leave_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &party_id, &account_id, &char_id);
		}
	}
	return retVal___;
}
int HP_inter_party_CharOnline(int char_id, int party_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_party_CharOnline_pre ) {
		int (*preHookFunc) (int *char_id, int *party_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_CharOnline_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_party_CharOnline_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &party_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_party.CharOnline(char_id, party_id);
	}
	if( HPMHooks.count.HP_inter_party_CharOnline_post ) {
		int (*postHookFunc) (int retVal___, int *char_id, int *party_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_CharOnline_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_party_CharOnline_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &char_id, &party_id);
		}
	}
	return retVal___;
}
int HP_inter_party_CharOffline(int char_id, int party_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_party_CharOffline_pre ) {
		int (*preHookFunc) (int *char_id, int *party_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_CharOffline_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_party_CharOffline_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &party_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_party.CharOffline(char_id, party_id);
	}
	if( HPMHooks.count.HP_inter_party_CharOffline_post ) {
		int (*postHookFunc) (int retVal___, int *char_id, int *party_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_party_CharOffline_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_party_CharOffline_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &char_id, &party_id);
		}
	}
	return retVal___;
}
/* inter_pet */
int HP_inter_pet_tosql(int pet_id, struct s_pet *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_pet_tosql_pre ) {
		int (*preHookFunc) (int *pet_id, struct s_pet *p);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_pet_tosql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_pet_tosql_pre[hIndex].func;
			retVal___ = preHookFunc(&pet_id, p);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_pet.tosql(pet_id, p);
	}
	if( HPMHooks.count.HP_inter_pet_tosql_post ) {
		int (*postHookFunc) (int retVal___, int *pet_id, struct s_pet *p);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_pet_tosql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_pet_tosql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &pet_id, p);
		}
	}
	return retVal___;
}
int HP_inter_pet_fromsql(int pet_id, struct s_pet *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_pet_fromsql_pre ) {
		int (*preHookFunc) (int *pet_id, struct s_pet *p);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_pet_fromsql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_pet_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(&pet_id, p);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_pet.fromsql(pet_id, p);
	}
	if( HPMHooks.count.HP_inter_pet_fromsql_post ) {
		int (*postHookFunc) (int retVal___, int *pet_id, struct s_pet *p);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_pet_fromsql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_pet_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &pet_id, p);
		}
	}
	return retVal___;
}
int HP_inter_pet_sql_init(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_pet_sql_init_pre ) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_pet_sql_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_pet_sql_init_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_pet.sql_init();
	}
	if( HPMHooks.count.HP_inter_pet_sql_init_post ) {
		int (*postHookFunc) (int retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_pet_sql_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_pet_sql_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_inter_pet_sql_final(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_inter_pet_sql_final_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_pet_sql_final_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_pet_sql_final_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_pet.sql_final();
	}
	if( HPMHooks.count.HP_inter_pet_sql_final_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_pet_sql_final_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_pet_sql_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_inter_pet_delete_(int pet_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_pet_delete__pre ) {
		int (*preHookFunc) (int *pet_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_pet_delete__pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_pet_delete__pre[hIndex].func;
			retVal___ = preHookFunc(&pet_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_pet.delete_(pet_id);
	}
	if( HPMHooks.count.HP_inter_pet_delete__post ) {
		int (*postHookFunc) (int retVal___, int *pet_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_pet_delete__post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_pet_delete__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &pet_id);
		}
	}
	return retVal___;
}
int HP_inter_pet_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_pet_parse_frommap_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_pet_parse_frommap_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_pet_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_pet.parse_frommap(fd);
	}
	if( HPMHooks.count.HP_inter_pet_parse_frommap_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_pet_parse_frommap_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_pet_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
/* inter_quest */
int HP_inter_quest_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_quest_parse_frommap_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_quest_parse_frommap_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_quest_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_quest.parse_frommap(fd);
	}
	if( HPMHooks.count.HP_inter_quest_parse_frommap_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_quest_parse_frommap_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_quest_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
/* inter_storage */
int HP_inter_storage_tosql(int account_id, struct storage_data *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_storage_tosql_pre ) {
		int (*preHookFunc) (int *account_id, struct storage_data *p);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_tosql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_storage_tosql_pre[hIndex].func;
			retVal___ = preHookFunc(&account_id, p);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_storage.tosql(account_id, p);
	}
	if( HPMHooks.count.HP_inter_storage_tosql_post ) {
		int (*postHookFunc) (int retVal___, int *account_id, struct storage_data *p);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_tosql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_storage_tosql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &account_id, p);
		}
	}
	return retVal___;
}
int HP_inter_storage_fromsql(int account_id, struct storage_data *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_storage_fromsql_pre ) {
		int (*preHookFunc) (int *account_id, struct storage_data *p);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_fromsql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_storage_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(&account_id, p);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_storage.fromsql(account_id, p);
	}
	if( HPMHooks.count.HP_inter_storage_fromsql_post ) {
		int (*postHookFunc) (int retVal___, int *account_id, struct storage_data *p);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_fromsql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_storage_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &account_id, p);
		}
	}
	return retVal___;
}
int HP_inter_storage_guild_storage_tosql(int guild_id, struct guild_storage *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_storage_guild_storage_tosql_pre ) {
		int (*preHookFunc) (int *guild_id, struct guild_storage *p);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_guild_storage_tosql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_storage_guild_storage_tosql_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id, p);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_storage.guild_storage_tosql(guild_id, p);
	}
	if( HPMHooks.count.HP_inter_storage_guild_storage_tosql_post ) {
		int (*postHookFunc) (int retVal___, int *guild_id, struct guild_storage *p);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_guild_storage_tosql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_storage_guild_storage_tosql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &guild_id, p);
		}
	}
	return retVal___;
}
int HP_inter_storage_guild_storage_fromsql(int guild_id, struct guild_storage *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_storage_guild_storage_fromsql_pre ) {
		int (*preHookFunc) (int *guild_id, struct guild_storage *p);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_guild_storage_fromsql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_storage_guild_storage_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id, p);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_storage.guild_storage_fromsql(guild_id, p);
	}
	if( HPMHooks.count.HP_inter_storage_guild_storage_fromsql_post ) {
		int (*postHookFunc) (int retVal___, int *guild_id, struct guild_storage *p);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_guild_storage_fromsql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_storage_guild_storage_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &guild_id, p);
		}
	}
	return retVal___;
}
int HP_inter_storage_sql_init(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_storage_sql_init_pre ) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_sql_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_storage_sql_init_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_storage.sql_init();
	}
	if( HPMHooks.count.HP_inter_storage_sql_init_post ) {
		int (*postHookFunc) (int retVal___);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_sql_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_storage_sql_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
void HP_inter_storage_sql_final(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_inter_storage_sql_final_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_sql_final_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_storage_sql_final_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.inter_storage.sql_final();
	}
	if( HPMHooks.count.HP_inter_storage_sql_final_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_sql_final_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_storage_sql_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
int HP_inter_storage_delete_(int account_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_storage_delete__pre ) {
		int (*preHookFunc) (int *account_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_delete__pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_storage_delete__pre[hIndex].func;
			retVal___ = preHookFunc(&account_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_storage.delete_(account_id);
	}
	if( HPMHooks.count.HP_inter_storage_delete__post ) {
		int (*postHookFunc) (int retVal___, int *account_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_delete__post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_storage_delete__post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &account_id);
		}
	}
	return retVal___;
}
int HP_inter_storage_guild_storage_delete(int guild_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_storage_guild_storage_delete_pre ) {
		int (*preHookFunc) (int *guild_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_guild_storage_delete_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_storage_guild_storage_delete_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_storage.guild_storage_delete(guild_id);
	}
	if( HPMHooks.count.HP_inter_storage_guild_storage_delete_post ) {
		int (*postHookFunc) (int retVal___, int *guild_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_guild_storage_delete_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_storage_guild_storage_delete_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &guild_id);
		}
	}
	return retVal___;
}
int HP_inter_storage_parse_frommap(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_inter_storage_parse_frommap_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_parse_frommap_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_inter_storage_parse_frommap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.inter_storage.parse_frommap(fd);
	}
	if( HPMHooks.count.HP_inter_storage_parse_frommap_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_inter_storage_parse_frommap_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_inter_storage_parse_frommap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
/* loginif */
void HP_loginif_init(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_loginif_init_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_loginif_init_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.init();
	}
	if( HPMHooks.count.HP_loginif_init_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_loginif_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_loginif_final(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_loginif_final_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_final_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_loginif_final_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.final();
	}
	if( HPMHooks.count.HP_loginif_final_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_final_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_loginif_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_loginif_reset(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_loginif_reset_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_reset_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_loginif_reset_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.reset();
	}
	if( HPMHooks.count.HP_loginif_reset_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_reset_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_loginif_reset_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_loginif_check_shutdown(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_loginif_check_shutdown_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_check_shutdown_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_loginif_check_shutdown_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.check_shutdown();
	}
	if( HPMHooks.count.HP_loginif_check_shutdown_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_check_shutdown_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_loginif_check_shutdown_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_loginif_on_disconnect(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_loginif_on_disconnect_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_on_disconnect_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_loginif_on_disconnect_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.on_disconnect();
	}
	if( HPMHooks.count.HP_loginif_on_disconnect_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_on_disconnect_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_loginif_on_disconnect_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_loginif_on_ready(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_loginif_on_ready_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_on_ready_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_loginif_on_ready_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.on_ready();
	}
	if( HPMHooks.count.HP_loginif_on_ready_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_on_ready_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_loginif_on_ready_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_loginif_block_account(int account_id, int flag) {
	int hIndex = 0;
	if( HPMHooks.count.HP_loginif_block_account_pre ) {
		void (*preHookFunc) (int *account_id, int *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_block_account_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_loginif_block_account_pre[hIndex].func;
			preHookFunc(&account_id, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.block_account(account_id, flag);
	}
	if( HPMHooks.count.HP_loginif_block_account_post ) {
		void (*postHookFunc) (int *account_id, int *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_block_account_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_loginif_block_account_post[hIndex].func;
			postHookFunc(&account_id, &flag);
		}
	}
	return;
}
void HP_loginif_ban_account(int account_id, short year, short month, short day, short hour, short minute, short second) {
	int hIndex = 0;
	if( HPMHooks.count.HP_loginif_ban_account_pre ) {
		void (*preHookFunc) (int *account_id, short *year, short *month, short *day, short *hour, short *minute, short *second);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_ban_account_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_loginif_ban_account_pre[hIndex].func;
			preHookFunc(&account_id, &year, &month, &day, &hour, &minute, &second);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.ban_account(account_id, year, month, day, hour, minute, second);
	}
	if( HPMHooks.count.HP_loginif_ban_account_post ) {
		void (*postHookFunc) (int *account_id, short *year, short *month, short *day, short *hour, short *minute, short *second);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_ban_account_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_loginif_ban_account_post[hIndex].func;
			postHookFunc(&account_id, &year, &month, &day, &hour, &minute, &second);
		}
	}
	return;
}
void HP_loginif_unban_account(int account_id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_loginif_unban_account_pre ) {
		void (*preHookFunc) (int *account_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_unban_account_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_loginif_unban_account_pre[hIndex].func;
			preHookFunc(&account_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.unban_account(account_id);
	}
	if( HPMHooks.count.HP_loginif_unban_account_post ) {
		void (*postHookFunc) (int *account_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_unban_account_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_loginif_unban_account_post[hIndex].func;
			postHookFunc(&account_id);
		}
	}
	return;
}
void HP_loginif_changesex(int account_id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_loginif_changesex_pre ) {
		void (*preHookFunc) (int *account_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_changesex_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_loginif_changesex_pre[hIndex].func;
			preHookFunc(&account_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.changesex(account_id);
	}
	if( HPMHooks.count.HP_loginif_changesex_post ) {
		void (*postHookFunc) (int *account_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_changesex_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_loginif_changesex_post[hIndex].func;
			postHookFunc(&account_id);
		}
	}
	return;
}
void HP_loginif_auth(int fd, struct char_session_data *sd, uint32 ipl) {
	int hIndex = 0;
	if( HPMHooks.count.HP_loginif_auth_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd, uint32 *ipl);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_auth_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_loginif_auth_pre[hIndex].func;
			preHookFunc(&fd, sd, &ipl);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.auth(fd, sd, ipl);
	}
	if( HPMHooks.count.HP_loginif_auth_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd, uint32 *ipl);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_auth_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_loginif_auth_post[hIndex].func;
			postHookFunc(&fd, sd, &ipl);
		}
	}
	return;
}
void HP_loginif_send_users_count(int users) {
	int hIndex = 0;
	if( HPMHooks.count.HP_loginif_send_users_count_pre ) {
		void (*preHookFunc) (int *users);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_send_users_count_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_loginif_send_users_count_pre[hIndex].func;
			preHookFunc(&users);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.send_users_count(users);
	}
	if( HPMHooks.count.HP_loginif_send_users_count_post ) {
		void (*postHookFunc) (int *users);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_send_users_count_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_loginif_send_users_count_post[hIndex].func;
			postHookFunc(&users);
		}
	}
	return;
}
void HP_loginif_connect_to_server(void) {
	int hIndex = 0;
	if( HPMHooks.count.HP_loginif_connect_to_server_pre ) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_connect_to_server_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_loginif_connect_to_server_pre[hIndex].func;
			preHookFunc();
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.loginif.connect_to_server();
	}
	if( HPMHooks.count.HP_loginif_connect_to_server_post ) {
		void (*postHookFunc) (void);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_loginif_connect_to_server_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_loginif_connect_to_server_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
/* mapif */
void HP_mapif_ban(int id, unsigned int flag, int status) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_ban_pre ) {
		void (*preHookFunc) (int *id, unsigned int *flag, int *status);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_ban_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_ban_pre[hIndex].func;
			preHookFunc(&id, &flag, &status);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.ban(id, flag, status);
	}
	if( HPMHooks.count.HP_mapif_ban_post ) {
		void (*postHookFunc) (int *id, unsigned int *flag, int *status);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_ban_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_ban_post[hIndex].func;
			postHookFunc(&id, &flag, &status);
		}
	}
	return;
}
void HP_mapif_server_init(int id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_server_init_pre ) {
		void (*preHookFunc) (int *id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_server_init_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_server_init_pre[hIndex].func;
			preHookFunc(&id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.server_init(id);
	}
	if( HPMHooks.count.HP_mapif_server_init_post ) {
		void (*postHookFunc) (int *id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_server_init_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_server_init_post[hIndex].func;
			postHookFunc(&id);
		}
	}
	return;
}
void HP_mapif_server_destroy(int id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_server_destroy_pre ) {
		void (*preHookFunc) (int *id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_server_destroy_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_server_destroy_pre[hIndex].func;
			preHookFunc(&id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.server_destroy(id);
	}
	if( HPMHooks.count.HP_mapif_server_destroy_post ) {
		void (*postHookFunc) (int *id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_server_destroy_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_server_destroy_post[hIndex].func;
			postHookFunc(&id);
		}
	}
	return;
}
void HP_mapif_server_reset(int id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_server_reset_pre ) {
		void (*preHookFunc) (int *id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_server_reset_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_server_reset_pre[hIndex].func;
			preHookFunc(&id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.server_reset(id);
	}
	if( HPMHooks.count.HP_mapif_server_reset_post ) {
		void (*postHookFunc) (int *id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_server_reset_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_server_reset_post[hIndex].func;
			postHookFunc(&id);
		}
	}
	return;
}
void HP_mapif_on_disconnect(int id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_on_disconnect_pre ) {
		void (*preHookFunc) (int *id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_on_disconnect_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_on_disconnect_pre[hIndex].func;
			preHookFunc(&id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.on_disconnect(id);
	}
	if( HPMHooks.count.HP_mapif_on_disconnect_post ) {
		void (*postHookFunc) (int *id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_on_disconnect_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_on_disconnect_post[hIndex].func;
			postHookFunc(&id);
		}
	}
	return;
}
void HP_mapif_on_parse_accinfo(int account_id, int u_fd, int u_aid, int u_group, int map_fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_on_parse_accinfo_pre ) {
		void (*preHookFunc) (int *account_id, int *u_fd, int *u_aid, int *u_group, int *map_fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_on_parse_accinfo_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_on_parse_accinfo_pre[hIndex].func;
			preHookFunc(&account_id, &u_fd, &u_aid, &u_group, &map_fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.on_parse_accinfo(account_id, u_fd, u_aid, u_group, map_fd);
	}
	if( HPMHooks.count.HP_mapif_on_parse_accinfo_post ) {
		void (*postHookFunc) (int *account_id, int *u_fd, int *u_aid, int *u_group, int *map_fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_on_parse_accinfo_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_on_parse_accinfo_post[hIndex].func;
			postHookFunc(&account_id, &u_fd, &u_aid, &u_group, &map_fd);
		}
	}
	return;
}
void HP_mapif_char_ban(int char_id, time_t timestamp) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_char_ban_pre ) {
		void (*preHookFunc) (int *char_id, time_t *timestamp);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_char_ban_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_char_ban_pre[hIndex].func;
			preHookFunc(&char_id, &timestamp);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.char_ban(char_id, timestamp);
	}
	if( HPMHooks.count.HP_mapif_char_ban_post ) {
		void (*postHookFunc) (int *char_id, time_t *timestamp);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_char_ban_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_char_ban_post[hIndex].func;
			postHookFunc(&char_id, &timestamp);
		}
	}
	return;
}
int HP_mapif_sendall(unsigned char *buf, unsigned int len) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_sendall_pre ) {
		int (*preHookFunc) (unsigned char *buf, unsigned int *len);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_sendall_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_sendall_pre[hIndex].func;
			retVal___ = preHookFunc(buf, &len);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.sendall(buf, len);
	}
	if( HPMHooks.count.HP_mapif_sendall_post ) {
		int (*postHookFunc) (int retVal___, unsigned char *buf, unsigned int *len);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_sendall_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_sendall_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, buf, &len);
		}
	}
	return retVal___;
}
int HP_mapif_sendallwos(int sfd, unsigned char *buf, unsigned int len) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_sendallwos_pre ) {
		int (*preHookFunc) (int *sfd, unsigned char *buf, unsigned int *len);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_sendallwos_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_sendallwos_pre[hIndex].func;
			retVal___ = preHookFunc(&sfd, buf, &len);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.sendallwos(sfd, buf, len);
	}
	if( HPMHooks.count.HP_mapif_sendallwos_post ) {
		int (*postHookFunc) (int retVal___, int *sfd, unsigned char *buf, unsigned int *len);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_sendallwos_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_sendallwos_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &sfd, buf, &len);
		}
	}
	return retVal___;
}
int HP_mapif_send(int fd, unsigned char *buf, unsigned int len) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_send_pre ) {
		int (*preHookFunc) (int *fd, unsigned char *buf, unsigned int *len);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_send_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_send_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, buf, &len);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.send(fd, buf, len);
	}
	if( HPMHooks.count.HP_mapif_send_post ) {
		int (*postHookFunc) (int retVal___, int *fd, unsigned char *buf, unsigned int *len);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_send_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_send_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, buf, &len);
		}
	}
	return retVal___;
}
void HP_mapif_send_users_count(int users) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_send_users_count_pre ) {
		void (*preHookFunc) (int *users);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_send_users_count_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_send_users_count_pre[hIndex].func;
			preHookFunc(&users);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.send_users_count(users);
	}
	if( HPMHooks.count.HP_mapif_send_users_count_post ) {
		void (*postHookFunc) (int *users);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_send_users_count_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_send_users_count_post[hIndex].func;
			postHookFunc(&users);
		}
	}
	return;
}
void HP_mapif_auction_message(int char_id, unsigned char result) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_auction_message_pre ) {
		void (*preHookFunc) (int *char_id, unsigned char *result);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_auction_message_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_auction_message_pre[hIndex].func;
			preHookFunc(&char_id, &result);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.auction_message(char_id, result);
	}
	if( HPMHooks.count.HP_mapif_auction_message_post ) {
		void (*postHookFunc) (int *char_id, unsigned char *result);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_auction_message_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_auction_message_post[hIndex].func;
			postHookFunc(&char_id, &result);
		}
	}
	return;
}
void HP_mapif_auction_sendlist(int fd, int char_id, short count, short pages, unsigned char *buf) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_auction_sendlist_pre ) {
		void (*preHookFunc) (int *fd, int *char_id, short *count, short *pages, unsigned char *buf);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_auction_sendlist_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_auction_sendlist_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &count, &pages, buf);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.auction_sendlist(fd, char_id, count, pages, buf);
	}
	if( HPMHooks.count.HP_mapif_auction_sendlist_post ) {
		void (*postHookFunc) (int *fd, int *char_id, short *count, short *pages, unsigned char *buf);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_auction_sendlist_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_auction_sendlist_post[hIndex].func;
			postHookFunc(&fd, &char_id, &count, &pages, buf);
		}
	}
	return;
}
void HP_mapif_parse_auction_requestlist(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_auction_requestlist_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_auction_requestlist_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_auction_requestlist_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_auction_requestlist(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_auction_requestlist_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_auction_requestlist_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_auction_requestlist_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_mapif_auction_register(int fd, struct auction_data *auction) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_auction_register_pre ) {
		void (*preHookFunc) (int *fd, struct auction_data *auction);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_auction_register_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_auction_register_pre[hIndex].func;
			preHookFunc(&fd, auction);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.auction_register(fd, auction);
	}
	if( HPMHooks.count.HP_mapif_auction_register_post ) {
		void (*postHookFunc) (int *fd, struct auction_data *auction);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_auction_register_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_auction_register_post[hIndex].func;
			postHookFunc(&fd, auction);
		}
	}
	return;
}
void HP_mapif_parse_auction_register(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_auction_register_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_auction_register_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_auction_register_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_auction_register(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_auction_register_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_auction_register_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_auction_register_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_mapif_auction_cancel(int fd, int char_id, unsigned char result) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_auction_cancel_pre ) {
		void (*preHookFunc) (int *fd, int *char_id, unsigned char *result);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_auction_cancel_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_auction_cancel_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &result);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.auction_cancel(fd, char_id, result);
	}
	if( HPMHooks.count.HP_mapif_auction_cancel_post ) {
		void (*postHookFunc) (int *fd, int *char_id, unsigned char *result);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_auction_cancel_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_auction_cancel_post[hIndex].func;
			postHookFunc(&fd, &char_id, &result);
		}
	}
	return;
}
void HP_mapif_parse_auction_cancel(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_auction_cancel_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_auction_cancel_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_auction_cancel_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_auction_cancel(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_auction_cancel_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_auction_cancel_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_auction_cancel_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_mapif_auction_close(int fd, int char_id, unsigned char result) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_auction_close_pre ) {
		void (*preHookFunc) (int *fd, int *char_id, unsigned char *result);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_auction_close_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_auction_close_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &result);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.auction_close(fd, char_id, result);
	}
	if( HPMHooks.count.HP_mapif_auction_close_post ) {
		void (*postHookFunc) (int *fd, int *char_id, unsigned char *result);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_auction_close_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_auction_close_post[hIndex].func;
			postHookFunc(&fd, &char_id, &result);
		}
	}
	return;
}
void HP_mapif_parse_auction_close(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_auction_close_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_auction_close_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_auction_close_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_auction_close(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_auction_close_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_auction_close_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_auction_close_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_mapif_auction_bid(int fd, int char_id, int bid, unsigned char result) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_auction_bid_pre ) {
		void (*preHookFunc) (int *fd, int *char_id, int *bid, unsigned char *result);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_auction_bid_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_auction_bid_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &bid, &result);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.auction_bid(fd, char_id, bid, result);
	}
	if( HPMHooks.count.HP_mapif_auction_bid_post ) {
		void (*postHookFunc) (int *fd, int *char_id, int *bid, unsigned char *result);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_auction_bid_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_auction_bid_post[hIndex].func;
			postHookFunc(&fd, &char_id, &bid, &result);
		}
	}
	return;
}
void HP_mapif_parse_auction_bid(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_auction_bid_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_auction_bid_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_auction_bid_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_auction_bid(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_auction_bid_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_auction_bid_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_auction_bid_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
bool HP_mapif_elemental_save(struct s_elemental *ele) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_mapif_elemental_save_pre ) {
		bool (*preHookFunc) (struct s_elemental *ele);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_save_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_elemental_save_pre[hIndex].func;
			retVal___ = preHookFunc(ele);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.elemental_save(ele);
	}
	if( HPMHooks.count.HP_mapif_elemental_save_post ) {
		bool (*postHookFunc) (bool retVal___, struct s_elemental *ele);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_save_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_elemental_save_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, ele);
		}
	}
	return retVal___;
}
bool HP_mapif_elemental_load(int ele_id, int char_id, struct s_elemental *ele) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_mapif_elemental_load_pre ) {
		bool (*preHookFunc) (int *ele_id, int *char_id, struct s_elemental *ele);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_load_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_elemental_load_pre[hIndex].func;
			retVal___ = preHookFunc(&ele_id, &char_id, ele);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.elemental_load(ele_id, char_id, ele);
	}
	if( HPMHooks.count.HP_mapif_elemental_load_post ) {
		bool (*postHookFunc) (bool retVal___, int *ele_id, int *char_id, struct s_elemental *ele);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_load_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_elemental_load_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &ele_id, &char_id, ele);
		}
	}
	return retVal___;
}
bool HP_mapif_elemental_delete(int ele_id) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_mapif_elemental_delete_pre ) {
		bool (*preHookFunc) (int *ele_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_delete_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_elemental_delete_pre[hIndex].func;
			retVal___ = preHookFunc(&ele_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.elemental_delete(ele_id);
	}
	if( HPMHooks.count.HP_mapif_elemental_delete_post ) {
		bool (*postHookFunc) (bool retVal___, int *ele_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_delete_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_elemental_delete_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &ele_id);
		}
	}
	return retVal___;
}
void HP_mapif_elemental_send(int fd, struct s_elemental *ele, unsigned char flag) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_elemental_send_pre ) {
		void (*preHookFunc) (int *fd, struct s_elemental *ele, unsigned char *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_send_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_elemental_send_pre[hIndex].func;
			preHookFunc(&fd, ele, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.elemental_send(fd, ele, flag);
	}
	if( HPMHooks.count.HP_mapif_elemental_send_post ) {
		void (*postHookFunc) (int *fd, struct s_elemental *ele, unsigned char *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_send_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_elemental_send_post[hIndex].func;
			postHookFunc(&fd, ele, &flag);
		}
	}
	return;
}
void HP_mapif_parse_elemental_create(int fd, struct s_elemental *ele) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_elemental_create_pre ) {
		void (*preHookFunc) (int *fd, struct s_elemental *ele);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_elemental_create_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_elemental_create_pre[hIndex].func;
			preHookFunc(&fd, ele);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_elemental_create(fd, ele);
	}
	if( HPMHooks.count.HP_mapif_parse_elemental_create_post ) {
		void (*postHookFunc) (int *fd, struct s_elemental *ele);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_elemental_create_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_elemental_create_post[hIndex].func;
			postHookFunc(&fd, ele);
		}
	}
	return;
}
void HP_mapif_parse_elemental_load(int fd, int ele_id, int char_id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_elemental_load_pre ) {
		void (*preHookFunc) (int *fd, int *ele_id, int *char_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_elemental_load_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_elemental_load_pre[hIndex].func;
			preHookFunc(&fd, &ele_id, &char_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_elemental_load(fd, ele_id, char_id);
	}
	if( HPMHooks.count.HP_mapif_parse_elemental_load_post ) {
		void (*postHookFunc) (int *fd, int *ele_id, int *char_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_elemental_load_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_elemental_load_post[hIndex].func;
			postHookFunc(&fd, &ele_id, &char_id);
		}
	}
	return;
}
void HP_mapif_elemental_deleted(int fd, unsigned char flag) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_elemental_deleted_pre ) {
		void (*preHookFunc) (int *fd, unsigned char *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_deleted_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_elemental_deleted_pre[hIndex].func;
			preHookFunc(&fd, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.elemental_deleted(fd, flag);
	}
	if( HPMHooks.count.HP_mapif_elemental_deleted_post ) {
		void (*postHookFunc) (int *fd, unsigned char *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_deleted_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_elemental_deleted_post[hIndex].func;
			postHookFunc(&fd, &flag);
		}
	}
	return;
}
void HP_mapif_parse_elemental_delete(int fd, int ele_id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_elemental_delete_pre ) {
		void (*preHookFunc) (int *fd, int *ele_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_elemental_delete_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_elemental_delete_pre[hIndex].func;
			preHookFunc(&fd, &ele_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_elemental_delete(fd, ele_id);
	}
	if( HPMHooks.count.HP_mapif_parse_elemental_delete_post ) {
		void (*postHookFunc) (int *fd, int *ele_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_elemental_delete_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_elemental_delete_post[hIndex].func;
			postHookFunc(&fd, &ele_id);
		}
	}
	return;
}
void HP_mapif_elemental_saved(int fd, unsigned char flag) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_elemental_saved_pre ) {
		void (*preHookFunc) (int *fd, unsigned char *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_saved_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_elemental_saved_pre[hIndex].func;
			preHookFunc(&fd, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.elemental_saved(fd, flag);
	}
	if( HPMHooks.count.HP_mapif_elemental_saved_post ) {
		void (*postHookFunc) (int *fd, unsigned char *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_elemental_saved_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_elemental_saved_post[hIndex].func;
			postHookFunc(&fd, &flag);
		}
	}
	return;
}
void HP_mapif_parse_elemental_save(int fd, struct s_elemental *ele) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_elemental_save_pre ) {
		void (*preHookFunc) (int *fd, struct s_elemental *ele);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_elemental_save_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_elemental_save_pre[hIndex].func;
			preHookFunc(&fd, ele);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_elemental_save(fd, ele);
	}
	if( HPMHooks.count.HP_mapif_parse_elemental_save_post ) {
		void (*postHookFunc) (int *fd, struct s_elemental *ele);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_elemental_save_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_elemental_save_post[hIndex].func;
			postHookFunc(&fd, ele);
		}
	}
	return;
}
int HP_mapif_guild_created(int fd, int account_id, struct guild *g) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_guild_created_pre ) {
		int (*preHookFunc) (int *fd, int *account_id, struct guild *g);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_created_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_created_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, g);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_created(fd, account_id, g);
	}
	if( HPMHooks.count.HP_mapif_guild_created_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *account_id, struct guild *g);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_created_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_created_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &account_id, g);
		}
	}
	return retVal___;
}
int HP_mapif_guild_noinfo(int fd, int guild_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_guild_noinfo_pre ) {
		int (*preHookFunc) (int *fd, int *guild_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_noinfo_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_noinfo_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_noinfo(fd, guild_id);
	}
	if( HPMHooks.count.HP_mapif_guild_noinfo_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *guild_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_noinfo_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_noinfo_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &guild_id);
		}
	}
	return retVal___;
}
int HP_mapif_guild_info(int fd, struct guild *g) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_guild_info_pre ) {
		int (*preHookFunc) (int *fd, struct guild *g);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_info_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_info_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, g);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_info(fd, g);
	}
	if( HPMHooks.count.HP_mapif_guild_info_post ) {
		int (*postHookFunc) (int retVal___, int *fd, struct guild *g);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_info_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_info_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, g);
		}
	}
	return retVal___;
}
int HP_mapif_guild_memberadded(int fd, int guild_id, int account_id, int char_id, int flag) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_guild_memberadded_pre ) {
		int (*preHookFunc) (int *fd, int *guild_id, int *account_id, int *char_id, int *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_memberadded_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_memberadded_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id, &account_id, &char_id, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_memberadded(fd, guild_id, account_id, char_id, flag);
	}
	if( HPMHooks.count.HP_mapif_guild_memberadded_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *guild_id, int *account_id, int *char_id, int *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_memberadded_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_memberadded_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &guild_id, &account_id, &char_id, &flag);
		}
	}
	return retVal___;
}
int HP_mapif_guild_withdraw(int guild_id, int account_id, int char_id, int flag, const char *name, const char *mes) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_guild_withdraw_pre ) {
		int (*preHookFunc) (int *guild_id, int *account_id, int *char_id, int *flag, const char *name, const char *mes);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_withdraw_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_withdraw_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id, &account_id, &char_id, &flag, name, mes);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_withdraw(guild_id, account_id, char_id, flag, name, mes);
	}
	if( HPMHooks.count.HP_mapif_guild_withdraw_post ) {
		int (*postHookFunc) (int retVal___, int *guild_id, int *account_id, int *char_id, int *flag, const char *name, const char *mes);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_withdraw_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_withdraw_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &guild_id, &account_id, &char_id, &flag, name, mes);
		}
	}
	return retVal___;
}
int HP_mapif_guild_memberinfoshort(struct guild *g, int idx) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_guild_memberinfoshort_pre ) {
		int (*preHookFunc) (struct guild *g, int *idx);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_memberinfoshort_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_memberinfoshort_pre[hIndex].func;
			retVal___ = preHookFunc(g, &idx);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_memberinfoshort(g, idx);
	}
	if( HPMHooks.count.HP_mapif_guild_memberinfoshort_post ) {
		int (*postHookFunc) (int retVal___, struct guild *g, int *idx);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_memberinfoshort_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_memberinfoshort_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, g, &idx);
		}
	}
	return retVal___;
}
int HP_mapif_guild_broken(int guild_id, int flag) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_guild_broken_pre ) {
		int (*preHookFunc) (int *guild_id, int *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_broken_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_broken_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_broken(guild_id, flag);
	}
	if( HPMHooks.count.HP_mapif_guild_broken_post ) {
		int (*postHookFunc) (int retVal___, int *guild_id, int *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_broken_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_broken_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &guild_id, &flag);
		}
	}
	return retVal___;
}
int HP_mapif_guild_message(int guild_id, int account_id, char *mes, int len, int sfd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_guild_message_pre ) {
		int (*preHookFunc) (int *guild_id, int *account_id, char *mes, int *len, int *sfd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_message_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_message_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id, &account_id, mes, &len, &sfd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_message(guild_id, account_id, mes, len, sfd);
	}
	if( HPMHooks.count.HP_mapif_guild_message_post ) {
		int (*postHookFunc) (int retVal___, int *guild_id, int *account_id, char *mes, int *len, int *sfd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_message_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_message_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &guild_id, &account_id, mes, &len, &sfd);
		}
	}
	return retVal___;
}
int HP_mapif_guild_basicinfochanged(int guild_id, int type, const void *data, int len) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_guild_basicinfochanged_pre ) {
		int (*preHookFunc) (int *guild_id, int *type, const void *data, int *len);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_basicinfochanged_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_basicinfochanged_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id, &type, data, &len);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_basicinfochanged(guild_id, type, data, len);
	}
	if( HPMHooks.count.HP_mapif_guild_basicinfochanged_post ) {
		int (*postHookFunc) (int retVal___, int *guild_id, int *type, const void *data, int *len);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_basicinfochanged_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_basicinfochanged_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &guild_id, &type, data, &len);
		}
	}
	return retVal___;
}
int HP_mapif_guild_memberinfochanged(int guild_id, int account_id, int char_id, int type, const void *data, int len) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_guild_memberinfochanged_pre ) {
		int (*preHookFunc) (int *guild_id, int *account_id, int *char_id, int *type, const void *data, int *len);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_memberinfochanged_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_memberinfochanged_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id, &account_id, &char_id, &type, data, &len);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_memberinfochanged(guild_id, account_id, char_id, type, data, len);
	}
	if( HPMHooks.count.HP_mapif_guild_memberinfochanged_post ) {
		int (*postHookFunc) (int retVal___, int *guild_id, int *account_id, int *char_id, int *type, const void *data, int *len);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_memberinfochanged_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_memberinfochanged_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &guild_id, &account_id, &char_id, &type, data, &len);
		}
	}
	return retVal___;
}
int HP_mapif_guild_skillupack(int guild_id, uint16 skill_id, int account_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_guild_skillupack_pre ) {
		int (*preHookFunc) (int *guild_id, uint16 *skill_id, int *account_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_skillupack_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_skillupack_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id, &skill_id, &account_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_skillupack(guild_id, skill_id, account_id);
	}
	if( HPMHooks.count.HP_mapif_guild_skillupack_post ) {
		int (*postHookFunc) (int retVal___, int *guild_id, uint16 *skill_id, int *account_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_skillupack_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_skillupack_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &guild_id, &skill_id, &account_id);
		}
	}
	return retVal___;
}
int HP_mapif_guild_alliance(int guild_id1, int guild_id2, int account_id1, int account_id2, int flag, const char *name1, const char *name2) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_guild_alliance_pre ) {
		int (*preHookFunc) (int *guild_id1, int *guild_id2, int *account_id1, int *account_id2, int *flag, const char *name1, const char *name2);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_alliance_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_alliance_pre[hIndex].func;
			retVal___ = preHookFunc(&guild_id1, &guild_id2, &account_id1, &account_id2, &flag, name1, name2);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_alliance(guild_id1, guild_id2, account_id1, account_id2, flag, name1, name2);
	}
	if( HPMHooks.count.HP_mapif_guild_alliance_post ) {
		int (*postHookFunc) (int retVal___, int *guild_id1, int *guild_id2, int *account_id1, int *account_id2, int *flag, const char *name1, const char *name2);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_alliance_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_alliance_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &guild_id1, &guild_id2, &account_id1, &account_id2, &flag, name1, name2);
		}
	}
	return retVal___;
}
int HP_mapif_guild_position(struct guild *g, int idx) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_guild_position_pre ) {
		int (*preHookFunc) (struct guild *g, int *idx);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_position_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_position_pre[hIndex].func;
			retVal___ = preHookFunc(g, &idx);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_position(g, idx);
	}
	if( HPMHooks.count.HP_mapif_guild_position_post ) {
		int (*postHookFunc) (int retVal___, struct guild *g, int *idx);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_position_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_position_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, g, &idx);
		}
	}
	return retVal___;
}
int HP_mapif_guild_notice(struct guild *g) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_guild_notice_pre ) {
		int (*preHookFunc) (struct guild *g);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_notice_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_notice_pre[hIndex].func;
			retVal___ = preHookFunc(g);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_notice(g);
	}
	if( HPMHooks.count.HP_mapif_guild_notice_post ) {
		int (*postHookFunc) (int retVal___, struct guild *g);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_notice_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_notice_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, g);
		}
	}
	return retVal___;
}
int HP_mapif_guild_emblem(struct guild *g) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_guild_emblem_pre ) {
		int (*preHookFunc) (struct guild *g);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_emblem_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_emblem_pre[hIndex].func;
			retVal___ = preHookFunc(g);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_emblem(g);
	}
	if( HPMHooks.count.HP_mapif_guild_emblem_post ) {
		int (*postHookFunc) (int retVal___, struct guild *g);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_emblem_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_emblem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, g);
		}
	}
	return retVal___;
}
int HP_mapif_guild_master_changed(struct guild *g, int aid, int cid) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_guild_master_changed_pre ) {
		int (*preHookFunc) (struct guild *g, int *aid, int *cid);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_master_changed_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_master_changed_pre[hIndex].func;
			retVal___ = preHookFunc(g, &aid, &cid);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_master_changed(g, aid, cid);
	}
	if( HPMHooks.count.HP_mapif_guild_master_changed_post ) {
		int (*postHookFunc) (int retVal___, struct guild *g, int *aid, int *cid);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_master_changed_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_master_changed_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, g, &aid, &cid);
		}
	}
	return retVal___;
}
int HP_mapif_guild_castle_dataload(int fd, int sz, int *castle_ids) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_guild_castle_dataload_pre ) {
		int (*preHookFunc) (int *fd, int *sz, int *castle_ids);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_castle_dataload_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_guild_castle_dataload_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &sz, castle_ids);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.guild_castle_dataload(fd, sz, castle_ids);
	}
	if( HPMHooks.count.HP_mapif_guild_castle_dataload_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *sz, int *castle_ids);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_guild_castle_dataload_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_guild_castle_dataload_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &sz, castle_ids);
		}
	}
	return retVal___;
}
int HP_mapif_parse_CreateGuild(int fd, int account_id, char *name, struct guild_member *master) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_CreateGuild_pre ) {
		int (*preHookFunc) (int *fd, int *account_id, char *name, struct guild_member *master);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_CreateGuild_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_CreateGuild_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, name, master);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_CreateGuild(fd, account_id, name, master);
	}
	if( HPMHooks.count.HP_mapif_parse_CreateGuild_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *account_id, char *name, struct guild_member *master);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_CreateGuild_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_CreateGuild_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &account_id, name, master);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildInfo(int fd, int guild_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_GuildInfo_pre ) {
		int (*preHookFunc) (int *fd, int *guild_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildInfo_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildInfo_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildInfo(fd, guild_id);
	}
	if( HPMHooks.count.HP_mapif_parse_GuildInfo_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *guild_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildInfo_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildInfo_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &guild_id);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildAddMember(int fd, int guild_id, struct guild_member *m) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_GuildAddMember_pre ) {
		int (*preHookFunc) (int *fd, int *guild_id, struct guild_member *m);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildAddMember_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildAddMember_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id, m);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildAddMember(fd, guild_id, m);
	}
	if( HPMHooks.count.HP_mapif_parse_GuildAddMember_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *guild_id, struct guild_member *m);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildAddMember_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildAddMember_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &guild_id, m);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildLeave(int fd, int guild_id, int account_id, int char_id, int flag, const char *mes) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_GuildLeave_pre ) {
		int (*preHookFunc) (int *fd, int *guild_id, int *account_id, int *char_id, int *flag, const char *mes);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildLeave_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildLeave_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id, &account_id, &char_id, &flag, mes);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildLeave(fd, guild_id, account_id, char_id, flag, mes);
	}
	if( HPMHooks.count.HP_mapif_parse_GuildLeave_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *guild_id, int *account_id, int *char_id, int *flag, const char *mes);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildLeave_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildLeave_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &guild_id, &account_id, &char_id, &flag, mes);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildChangeMemberInfoShort(int fd, int guild_id, int account_id, int char_id, int online, int lv, int class_) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_GuildChangeMemberInfoShort_pre ) {
		int (*preHookFunc) (int *fd, int *guild_id, int *account_id, int *char_id, int *online, int *lv, int *class_);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildChangeMemberInfoShort_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildChangeMemberInfoShort_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id, &account_id, &char_id, &online, &lv, &class_);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildChangeMemberInfoShort(fd, guild_id, account_id, char_id, online, lv, class_);
	}
	if( HPMHooks.count.HP_mapif_parse_GuildChangeMemberInfoShort_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *guild_id, int *account_id, int *char_id, int *online, int *lv, int *class_);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildChangeMemberInfoShort_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildChangeMemberInfoShort_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &guild_id, &account_id, &char_id, &online, &lv, &class_);
		}
	}
	return retVal___;
}
int HP_mapif_parse_BreakGuild(int fd, int guild_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_BreakGuild_pre ) {
		int (*preHookFunc) (int *fd, int *guild_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_BreakGuild_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_BreakGuild_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_BreakGuild(fd, guild_id);
	}
	if( HPMHooks.count.HP_mapif_parse_BreakGuild_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *guild_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_BreakGuild_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_BreakGuild_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &guild_id);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildMessage(int fd, int guild_id, int account_id, char *mes, int len) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_GuildMessage_pre ) {
		int (*preHookFunc) (int *fd, int *guild_id, int *account_id, char *mes, int *len);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildMessage_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildMessage_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id, &account_id, mes, &len);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildMessage(fd, guild_id, account_id, mes, len);
	}
	if( HPMHooks.count.HP_mapif_parse_GuildMessage_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *guild_id, int *account_id, char *mes, int *len);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildMessage_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildMessage_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &guild_id, &account_id, mes, &len);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildBasicInfoChange(int fd, int guild_id, int type, const void *data, int len) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_GuildBasicInfoChange_pre ) {
		int (*preHookFunc) (int *fd, int *guild_id, int *type, const void *data, int *len);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildBasicInfoChange_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildBasicInfoChange_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id, &type, data, &len);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildBasicInfoChange(fd, guild_id, type, data, len);
	}
	if( HPMHooks.count.HP_mapif_parse_GuildBasicInfoChange_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *guild_id, int *type, const void *data, int *len);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildBasicInfoChange_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildBasicInfoChange_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &guild_id, &type, data, &len);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildMemberInfoChange(int fd, int guild_id, int account_id, int char_id, int type, const char *data, int len) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_GuildMemberInfoChange_pre ) {
		int (*preHookFunc) (int *fd, int *guild_id, int *account_id, int *char_id, int *type, const char *data, int *len);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildMemberInfoChange_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildMemberInfoChange_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id, &account_id, &char_id, &type, data, &len);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildMemberInfoChange(fd, guild_id, account_id, char_id, type, data, len);
	}
	if( HPMHooks.count.HP_mapif_parse_GuildMemberInfoChange_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *guild_id, int *account_id, int *char_id, int *type, const char *data, int *len);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildMemberInfoChange_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildMemberInfoChange_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &guild_id, &account_id, &char_id, &type, data, &len);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildPosition(int fd, int guild_id, int idx, struct guild_position *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_GuildPosition_pre ) {
		int (*preHookFunc) (int *fd, int *guild_id, int *idx, struct guild_position *p);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildPosition_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildPosition_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id, &idx, p);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildPosition(fd, guild_id, idx, p);
	}
	if( HPMHooks.count.HP_mapif_parse_GuildPosition_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *guild_id, int *idx, struct guild_position *p);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildPosition_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildPosition_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &guild_id, &idx, p);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildSkillUp(int fd, int guild_id, uint16 skill_id, int account_id, int max) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_GuildSkillUp_pre ) {
		int (*preHookFunc) (int *fd, int *guild_id, uint16 *skill_id, int *account_id, int *max);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildSkillUp_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildSkillUp_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id, &skill_id, &account_id, &max);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildSkillUp(fd, guild_id, skill_id, account_id, max);
	}
	if( HPMHooks.count.HP_mapif_parse_GuildSkillUp_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *guild_id, uint16 *skill_id, int *account_id, int *max);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildSkillUp_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildSkillUp_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &guild_id, &skill_id, &account_id, &max);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildDeleteAlliance(struct guild *g, int guild_id, int account_id1, int account_id2, int flag) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_GuildDeleteAlliance_pre ) {
		int (*preHookFunc) (struct guild *g, int *guild_id, int *account_id1, int *account_id2, int *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildDeleteAlliance_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildDeleteAlliance_pre[hIndex].func;
			retVal___ = preHookFunc(g, &guild_id, &account_id1, &account_id2, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildDeleteAlliance(g, guild_id, account_id1, account_id2, flag);
	}
	if( HPMHooks.count.HP_mapif_parse_GuildDeleteAlliance_post ) {
		int (*postHookFunc) (int retVal___, struct guild *g, int *guild_id, int *account_id1, int *account_id2, int *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildDeleteAlliance_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildDeleteAlliance_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, g, &guild_id, &account_id1, &account_id2, &flag);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildAlliance(int fd, int guild_id1, int guild_id2, int account_id1, int account_id2, int flag) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_GuildAlliance_pre ) {
		int (*preHookFunc) (int *fd, int *guild_id1, int *guild_id2, int *account_id1, int *account_id2, int *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildAlliance_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildAlliance_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id1, &guild_id2, &account_id1, &account_id2, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildAlliance(fd, guild_id1, guild_id2, account_id1, account_id2, flag);
	}
	if( HPMHooks.count.HP_mapif_parse_GuildAlliance_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *guild_id1, int *guild_id2, int *account_id1, int *account_id2, int *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildAlliance_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildAlliance_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &guild_id1, &guild_id2, &account_id1, &account_id2, &flag);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildNotice(int fd, int guild_id, const char *mes1, const char *mes2) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_GuildNotice_pre ) {
		int (*preHookFunc) (int *fd, int *guild_id, const char *mes1, const char *mes2);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildNotice_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildNotice_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id, mes1, mes2);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildNotice(fd, guild_id, mes1, mes2);
	}
	if( HPMHooks.count.HP_mapif_parse_GuildNotice_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *guild_id, const char *mes1, const char *mes2);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildNotice_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildNotice_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &guild_id, mes1, mes2);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildEmblem(int fd, int len, int guild_id, int dummy, const char *data) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_GuildEmblem_pre ) {
		int (*preHookFunc) (int *fd, int *len, int *guild_id, int *dummy, const char *data);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildEmblem_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildEmblem_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &len, &guild_id, &dummy, data);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildEmblem(fd, len, guild_id, dummy, data);
	}
	if( HPMHooks.count.HP_mapif_parse_GuildEmblem_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *len, int *guild_id, int *dummy, const char *data);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildEmblem_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildEmblem_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &len, &guild_id, &dummy, data);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildCastleDataLoad(int fd, int len, int *castle_ids) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_GuildCastleDataLoad_pre ) {
		int (*preHookFunc) (int *fd, int *len, int *castle_ids);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildCastleDataLoad_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildCastleDataLoad_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &len, castle_ids);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildCastleDataLoad(fd, len, castle_ids);
	}
	if( HPMHooks.count.HP_mapif_parse_GuildCastleDataLoad_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *len, int *castle_ids);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildCastleDataLoad_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildCastleDataLoad_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &len, castle_ids);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildCastleDataSave(int fd, int castle_id, int index, int value) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_GuildCastleDataSave_pre ) {
		int (*preHookFunc) (int *fd, int *castle_id, int *index, int *value);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildCastleDataSave_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildCastleDataSave_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &castle_id, &index, &value);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildCastleDataSave(fd, castle_id, index, value);
	}
	if( HPMHooks.count.HP_mapif_parse_GuildCastleDataSave_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *castle_id, int *index, int *value);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildCastleDataSave_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildCastleDataSave_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &castle_id, &index, &value);
		}
	}
	return retVal___;
}
int HP_mapif_parse_GuildMasterChange(int fd, int guild_id, const char *name, int len) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_GuildMasterChange_pre ) {
		int (*preHookFunc) (int *fd, int *guild_id, const char *name, int *len);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildMasterChange_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_GuildMasterChange_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &guild_id, name, &len);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_GuildMasterChange(fd, guild_id, name, len);
	}
	if( HPMHooks.count.HP_mapif_parse_GuildMasterChange_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *guild_id, const char *name, int *len);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_GuildMasterChange_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_GuildMasterChange_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &guild_id, name, &len);
		}
	}
	return retVal___;
}
void HP_mapif_homunculus_created(int fd, int account_id, struct s_homunculus *sh, unsigned char flag) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_homunculus_created_pre ) {
		void (*preHookFunc) (int *fd, int *account_id, struct s_homunculus *sh, unsigned char *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_created_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_homunculus_created_pre[hIndex].func;
			preHookFunc(&fd, &account_id, sh, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.homunculus_created(fd, account_id, sh, flag);
	}
	if( HPMHooks.count.HP_mapif_homunculus_created_post ) {
		void (*postHookFunc) (int *fd, int *account_id, struct s_homunculus *sh, unsigned char *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_created_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_homunculus_created_post[hIndex].func;
			postHookFunc(&fd, &account_id, sh, &flag);
		}
	}
	return;
}
void HP_mapif_homunculus_deleted(int fd, int flag) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_homunculus_deleted_pre ) {
		void (*preHookFunc) (int *fd, int *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_deleted_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_homunculus_deleted_pre[hIndex].func;
			preHookFunc(&fd, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.homunculus_deleted(fd, flag);
	}
	if( HPMHooks.count.HP_mapif_homunculus_deleted_post ) {
		void (*postHookFunc) (int *fd, int *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_deleted_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_homunculus_deleted_post[hIndex].func;
			postHookFunc(&fd, &flag);
		}
	}
	return;
}
void HP_mapif_homunculus_loaded(int fd, int account_id, struct s_homunculus *hd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_homunculus_loaded_pre ) {
		void (*preHookFunc) (int *fd, int *account_id, struct s_homunculus *hd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_loaded_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_homunculus_loaded_pre[hIndex].func;
			preHookFunc(&fd, &account_id, hd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.homunculus_loaded(fd, account_id, hd);
	}
	if( HPMHooks.count.HP_mapif_homunculus_loaded_post ) {
		void (*postHookFunc) (int *fd, int *account_id, struct s_homunculus *hd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_loaded_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_homunculus_loaded_post[hIndex].func;
			postHookFunc(&fd, &account_id, hd);
		}
	}
	return;
}
void HP_mapif_homunculus_saved(int fd, int account_id, bool flag) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_homunculus_saved_pre ) {
		void (*preHookFunc) (int *fd, int *account_id, bool *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_saved_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_homunculus_saved_pre[hIndex].func;
			preHookFunc(&fd, &account_id, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.homunculus_saved(fd, account_id, flag);
	}
	if( HPMHooks.count.HP_mapif_homunculus_saved_post ) {
		void (*postHookFunc) (int *fd, int *account_id, bool *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_saved_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_homunculus_saved_post[hIndex].func;
			postHookFunc(&fd, &account_id, &flag);
		}
	}
	return;
}
void HP_mapif_homunculus_renamed(int fd, int account_id, int char_id, unsigned char flag, char *name) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_homunculus_renamed_pre ) {
		void (*preHookFunc) (int *fd, int *account_id, int *char_id, unsigned char *flag, char *name);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_renamed_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_homunculus_renamed_pre[hIndex].func;
			preHookFunc(&fd, &account_id, &char_id, &flag, name);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.homunculus_renamed(fd, account_id, char_id, flag, name);
	}
	if( HPMHooks.count.HP_mapif_homunculus_renamed_post ) {
		void (*postHookFunc) (int *fd, int *account_id, int *char_id, unsigned char *flag, char *name);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_renamed_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_homunculus_renamed_post[hIndex].func;
			postHookFunc(&fd, &account_id, &char_id, &flag, name);
		}
	}
	return;
}
bool HP_mapif_homunculus_save(struct s_homunculus *hd) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_mapif_homunculus_save_pre ) {
		bool (*preHookFunc) (struct s_homunculus *hd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_save_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_homunculus_save_pre[hIndex].func;
			retVal___ = preHookFunc(hd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.homunculus_save(hd);
	}
	if( HPMHooks.count.HP_mapif_homunculus_save_post ) {
		bool (*postHookFunc) (bool retVal___, struct s_homunculus *hd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_save_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_homunculus_save_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, hd);
		}
	}
	return retVal___;
}
bool HP_mapif_homunculus_load(int homun_id, struct s_homunculus *hd) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_mapif_homunculus_load_pre ) {
		bool (*preHookFunc) (int *homun_id, struct s_homunculus *hd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_load_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_homunculus_load_pre[hIndex].func;
			retVal___ = preHookFunc(&homun_id, hd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.homunculus_load(homun_id, hd);
	}
	if( HPMHooks.count.HP_mapif_homunculus_load_post ) {
		bool (*postHookFunc) (bool retVal___, int *homun_id, struct s_homunculus *hd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_load_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_homunculus_load_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &homun_id, hd);
		}
	}
	return retVal___;
}
bool HP_mapif_homunculus_delete(int homun_id) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_mapif_homunculus_delete_pre ) {
		bool (*preHookFunc) (int *homun_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_delete_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_homunculus_delete_pre[hIndex].func;
			retVal___ = preHookFunc(&homun_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.homunculus_delete(homun_id);
	}
	if( HPMHooks.count.HP_mapif_homunculus_delete_post ) {
		bool (*postHookFunc) (bool retVal___, int *homun_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_delete_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_homunculus_delete_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &homun_id);
		}
	}
	return retVal___;
}
bool HP_mapif_homunculus_rename(char *name) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_mapif_homunculus_rename_pre ) {
		bool (*preHookFunc) (char *name);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_rename_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_homunculus_rename_pre[hIndex].func;
			retVal___ = preHookFunc(name);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.homunculus_rename(name);
	}
	if( HPMHooks.count.HP_mapif_homunculus_rename_post ) {
		bool (*postHookFunc) (bool retVal___, char *name);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_homunculus_rename_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_homunculus_rename_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, name);
		}
	}
	return retVal___;
}
void HP_mapif_parse_homunculus_create(int fd, int len, int account_id, struct s_homunculus *phd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_homunculus_create_pre ) {
		void (*preHookFunc) (int *fd, int *len, int *account_id, struct s_homunculus *phd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_homunculus_create_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_homunculus_create_pre[hIndex].func;
			preHookFunc(&fd, &len, &account_id, phd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_homunculus_create(fd, len, account_id, phd);
	}
	if( HPMHooks.count.HP_mapif_parse_homunculus_create_post ) {
		void (*postHookFunc) (int *fd, int *len, int *account_id, struct s_homunculus *phd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_homunculus_create_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_homunculus_create_post[hIndex].func;
			postHookFunc(&fd, &len, &account_id, phd);
		}
	}
	return;
}
void HP_mapif_parse_homunculus_delete(int fd, int homun_id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_homunculus_delete_pre ) {
		void (*preHookFunc) (int *fd, int *homun_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_homunculus_delete_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_homunculus_delete_pre[hIndex].func;
			preHookFunc(&fd, &homun_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_homunculus_delete(fd, homun_id);
	}
	if( HPMHooks.count.HP_mapif_parse_homunculus_delete_post ) {
		void (*postHookFunc) (int *fd, int *homun_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_homunculus_delete_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_homunculus_delete_post[hIndex].func;
			postHookFunc(&fd, &homun_id);
		}
	}
	return;
}
void HP_mapif_parse_homunculus_load(int fd, int account_id, int homun_id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_homunculus_load_pre ) {
		void (*preHookFunc) (int *fd, int *account_id, int *homun_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_homunculus_load_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_homunculus_load_pre[hIndex].func;
			preHookFunc(&fd, &account_id, &homun_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_homunculus_load(fd, account_id, homun_id);
	}
	if( HPMHooks.count.HP_mapif_parse_homunculus_load_post ) {
		void (*postHookFunc) (int *fd, int *account_id, int *homun_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_homunculus_load_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_homunculus_load_post[hIndex].func;
			postHookFunc(&fd, &account_id, &homun_id);
		}
	}
	return;
}
void HP_mapif_parse_homunculus_save(int fd, int len, int account_id, struct s_homunculus *phd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_homunculus_save_pre ) {
		void (*preHookFunc) (int *fd, int *len, int *account_id, struct s_homunculus *phd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_homunculus_save_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_homunculus_save_pre[hIndex].func;
			preHookFunc(&fd, &len, &account_id, phd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_homunculus_save(fd, len, account_id, phd);
	}
	if( HPMHooks.count.HP_mapif_parse_homunculus_save_post ) {
		void (*postHookFunc) (int *fd, int *len, int *account_id, struct s_homunculus *phd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_homunculus_save_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_homunculus_save_post[hIndex].func;
			postHookFunc(&fd, &len, &account_id, phd);
		}
	}
	return;
}
void HP_mapif_parse_homunculus_rename(int fd, int account_id, int char_id, char *name) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_homunculus_rename_pre ) {
		void (*preHookFunc) (int *fd, int *account_id, int *char_id, char *name);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_homunculus_rename_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_homunculus_rename_pre[hIndex].func;
			preHookFunc(&fd, &account_id, &char_id, name);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_homunculus_rename(fd, account_id, char_id, name);
	}
	if( HPMHooks.count.HP_mapif_parse_homunculus_rename_post ) {
		void (*postHookFunc) (int *fd, int *account_id, int *char_id, char *name);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_homunculus_rename_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_homunculus_rename_post[hIndex].func;
			postHookFunc(&fd, &account_id, &char_id, name);
		}
	}
	return;
}
void HP_mapif_mail_sendinbox(int fd, int char_id, unsigned char flag, struct mail_data *md) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_mail_sendinbox_pre ) {
		void (*preHookFunc) (int *fd, int *char_id, unsigned char *flag, struct mail_data *md);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_sendinbox_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_mail_sendinbox_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &flag, md);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.mail_sendinbox(fd, char_id, flag, md);
	}
	if( HPMHooks.count.HP_mapif_mail_sendinbox_post ) {
		void (*postHookFunc) (int *fd, int *char_id, unsigned char *flag, struct mail_data *md);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_sendinbox_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_mail_sendinbox_post[hIndex].func;
			postHookFunc(&fd, &char_id, &flag, md);
		}
	}
	return;
}
void HP_mapif_parse_mail_requestinbox(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_mail_requestinbox_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mail_requestinbox_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_mail_requestinbox_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_mail_requestinbox(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_mail_requestinbox_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mail_requestinbox_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_mail_requestinbox_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_mapif_parse_mail_read(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_mail_read_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mail_read_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_mail_read_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_mail_read(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_mail_read_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mail_read_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_mail_read_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_mapif_mail_sendattach(int fd, int char_id, struct mail_message *msg) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_mail_sendattach_pre ) {
		void (*preHookFunc) (int *fd, int *char_id, struct mail_message *msg);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_sendattach_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_mail_sendattach_pre[hIndex].func;
			preHookFunc(&fd, &char_id, msg);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.mail_sendattach(fd, char_id, msg);
	}
	if( HPMHooks.count.HP_mapif_mail_sendattach_post ) {
		void (*postHookFunc) (int *fd, int *char_id, struct mail_message *msg);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_sendattach_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_mail_sendattach_post[hIndex].func;
			postHookFunc(&fd, &char_id, msg);
		}
	}
	return;
}
void HP_mapif_mail_getattach(int fd, int char_id, int mail_id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_mail_getattach_pre ) {
		void (*preHookFunc) (int *fd, int *char_id, int *mail_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_getattach_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_mail_getattach_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &mail_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.mail_getattach(fd, char_id, mail_id);
	}
	if( HPMHooks.count.HP_mapif_mail_getattach_post ) {
		void (*postHookFunc) (int *fd, int *char_id, int *mail_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_getattach_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_mail_getattach_post[hIndex].func;
			postHookFunc(&fd, &char_id, &mail_id);
		}
	}
	return;
}
void HP_mapif_parse_mail_getattach(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_mail_getattach_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mail_getattach_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_mail_getattach_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_mail_getattach(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_mail_getattach_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mail_getattach_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_mail_getattach_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_mapif_mail_delete(int fd, int char_id, int mail_id, bool failed) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_mail_delete_pre ) {
		void (*preHookFunc) (int *fd, int *char_id, int *mail_id, bool *failed);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_delete_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_mail_delete_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &mail_id, &failed);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.mail_delete(fd, char_id, mail_id, failed);
	}
	if( HPMHooks.count.HP_mapif_mail_delete_post ) {
		void (*postHookFunc) (int *fd, int *char_id, int *mail_id, bool *failed);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_delete_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_mail_delete_post[hIndex].func;
			postHookFunc(&fd, &char_id, &mail_id, &failed);
		}
	}
	return;
}
void HP_mapif_parse_mail_delete(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_mail_delete_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mail_delete_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_mail_delete_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_mail_delete(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_mail_delete_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mail_delete_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_mail_delete_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_mapif_mail_new(struct mail_message *msg) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_mail_new_pre ) {
		void (*preHookFunc) (struct mail_message *msg);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_new_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_mail_new_pre[hIndex].func;
			preHookFunc(msg);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.mail_new(msg);
	}
	if( HPMHooks.count.HP_mapif_mail_new_post ) {
		void (*postHookFunc) (struct mail_message *msg);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_new_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_mail_new_post[hIndex].func;
			postHookFunc(msg);
		}
	}
	return;
}
void HP_mapif_mail_return(int fd, int char_id, int mail_id, int new_mail) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_mail_return_pre ) {
		void (*preHookFunc) (int *fd, int *char_id, int *mail_id, int *new_mail);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_return_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_mail_return_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &mail_id, &new_mail);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.mail_return(fd, char_id, mail_id, new_mail);
	}
	if( HPMHooks.count.HP_mapif_mail_return_post ) {
		void (*postHookFunc) (int *fd, int *char_id, int *mail_id, int *new_mail);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_return_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_mail_return_post[hIndex].func;
			postHookFunc(&fd, &char_id, &mail_id, &new_mail);
		}
	}
	return;
}
void HP_mapif_parse_mail_return(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_mail_return_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mail_return_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_mail_return_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_mail_return(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_mail_return_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mail_return_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_mail_return_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_mapif_mail_send(int fd, struct mail_message *msg) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_mail_send_pre ) {
		void (*preHookFunc) (int *fd, struct mail_message *msg);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_send_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_mail_send_pre[hIndex].func;
			preHookFunc(&fd, msg);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.mail_send(fd, msg);
	}
	if( HPMHooks.count.HP_mapif_mail_send_post ) {
		void (*postHookFunc) (int *fd, struct mail_message *msg);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mail_send_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_mail_send_post[hIndex].func;
			postHookFunc(&fd, msg);
		}
	}
	return;
}
void HP_mapif_parse_mail_send(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_mail_send_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mail_send_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_mail_send_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_mail_send(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_mail_send_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mail_send_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_mail_send_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
bool HP_mapif_mercenary_save(struct s_mercenary *merc) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_mapif_mercenary_save_pre ) {
		bool (*preHookFunc) (struct s_mercenary *merc);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_save_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_mercenary_save_pre[hIndex].func;
			retVal___ = preHookFunc(merc);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.mercenary_save(merc);
	}
	if( HPMHooks.count.HP_mapif_mercenary_save_post ) {
		bool (*postHookFunc) (bool retVal___, struct s_mercenary *merc);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_save_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_mercenary_save_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, merc);
		}
	}
	return retVal___;
}
bool HP_mapif_mercenary_load(int merc_id, int char_id, struct s_mercenary *merc) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_mapif_mercenary_load_pre ) {
		bool (*preHookFunc) (int *merc_id, int *char_id, struct s_mercenary *merc);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_load_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_mercenary_load_pre[hIndex].func;
			retVal___ = preHookFunc(&merc_id, &char_id, merc);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.mercenary_load(merc_id, char_id, merc);
	}
	if( HPMHooks.count.HP_mapif_mercenary_load_post ) {
		bool (*postHookFunc) (bool retVal___, int *merc_id, int *char_id, struct s_mercenary *merc);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_load_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_mercenary_load_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &merc_id, &char_id, merc);
		}
	}
	return retVal___;
}
bool HP_mapif_mercenary_delete(int merc_id) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_mapif_mercenary_delete_pre ) {
		bool (*preHookFunc) (int *merc_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_delete_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_mercenary_delete_pre[hIndex].func;
			retVal___ = preHookFunc(&merc_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.mercenary_delete(merc_id);
	}
	if( HPMHooks.count.HP_mapif_mercenary_delete_post ) {
		bool (*postHookFunc) (bool retVal___, int *merc_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_delete_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_mercenary_delete_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &merc_id);
		}
	}
	return retVal___;
}
void HP_mapif_mercenary_send(int fd, struct s_mercenary *merc, unsigned char flag) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_mercenary_send_pre ) {
		void (*preHookFunc) (int *fd, struct s_mercenary *merc, unsigned char *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_send_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_mercenary_send_pre[hIndex].func;
			preHookFunc(&fd, merc, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.mercenary_send(fd, merc, flag);
	}
	if( HPMHooks.count.HP_mapif_mercenary_send_post ) {
		void (*postHookFunc) (int *fd, struct s_mercenary *merc, unsigned char *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_send_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_mercenary_send_post[hIndex].func;
			postHookFunc(&fd, merc, &flag);
		}
	}
	return;
}
void HP_mapif_parse_mercenary_create(int fd, struct s_mercenary *merc) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_mercenary_create_pre ) {
		void (*preHookFunc) (int *fd, struct s_mercenary *merc);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mercenary_create_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_mercenary_create_pre[hIndex].func;
			preHookFunc(&fd, merc);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_mercenary_create(fd, merc);
	}
	if( HPMHooks.count.HP_mapif_parse_mercenary_create_post ) {
		void (*postHookFunc) (int *fd, struct s_mercenary *merc);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mercenary_create_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_mercenary_create_post[hIndex].func;
			postHookFunc(&fd, merc);
		}
	}
	return;
}
void HP_mapif_parse_mercenary_load(int fd, int merc_id, int char_id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_mercenary_load_pre ) {
		void (*preHookFunc) (int *fd, int *merc_id, int *char_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mercenary_load_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_mercenary_load_pre[hIndex].func;
			preHookFunc(&fd, &merc_id, &char_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_mercenary_load(fd, merc_id, char_id);
	}
	if( HPMHooks.count.HP_mapif_parse_mercenary_load_post ) {
		void (*postHookFunc) (int *fd, int *merc_id, int *char_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mercenary_load_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_mercenary_load_post[hIndex].func;
			postHookFunc(&fd, &merc_id, &char_id);
		}
	}
	return;
}
void HP_mapif_mercenary_deleted(int fd, unsigned char flag) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_mercenary_deleted_pre ) {
		void (*preHookFunc) (int *fd, unsigned char *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_deleted_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_mercenary_deleted_pre[hIndex].func;
			preHookFunc(&fd, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.mercenary_deleted(fd, flag);
	}
	if( HPMHooks.count.HP_mapif_mercenary_deleted_post ) {
		void (*postHookFunc) (int *fd, unsigned char *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_deleted_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_mercenary_deleted_post[hIndex].func;
			postHookFunc(&fd, &flag);
		}
	}
	return;
}
void HP_mapif_parse_mercenary_delete(int fd, int merc_id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_mercenary_delete_pre ) {
		void (*preHookFunc) (int *fd, int *merc_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mercenary_delete_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_mercenary_delete_pre[hIndex].func;
			preHookFunc(&fd, &merc_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_mercenary_delete(fd, merc_id);
	}
	if( HPMHooks.count.HP_mapif_parse_mercenary_delete_post ) {
		void (*postHookFunc) (int *fd, int *merc_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mercenary_delete_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_mercenary_delete_post[hIndex].func;
			postHookFunc(&fd, &merc_id);
		}
	}
	return;
}
void HP_mapif_mercenary_saved(int fd, unsigned char flag) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_mercenary_saved_pre ) {
		void (*preHookFunc) (int *fd, unsigned char *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_saved_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_mercenary_saved_pre[hIndex].func;
			preHookFunc(&fd, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.mercenary_saved(fd, flag);
	}
	if( HPMHooks.count.HP_mapif_mercenary_saved_post ) {
		void (*postHookFunc) (int *fd, unsigned char *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_mercenary_saved_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_mercenary_saved_post[hIndex].func;
			postHookFunc(&fd, &flag);
		}
	}
	return;
}
void HP_mapif_parse_mercenary_save(int fd, struct s_mercenary *merc) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_mercenary_save_pre ) {
		void (*preHookFunc) (int *fd, struct s_mercenary *merc);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mercenary_save_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_mercenary_save_pre[hIndex].func;
			preHookFunc(&fd, merc);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_mercenary_save(fd, merc);
	}
	if( HPMHooks.count.HP_mapif_parse_mercenary_save_post ) {
		void (*postHookFunc) (int *fd, struct s_mercenary *merc);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_mercenary_save_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_mercenary_save_post[hIndex].func;
			postHookFunc(&fd, merc);
		}
	}
	return;
}
int HP_mapif_party_created(int fd, int account_id, int char_id, struct party *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_party_created_pre ) {
		int (*preHookFunc) (int *fd, int *account_id, int *char_id, struct party *p);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_created_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_party_created_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, &char_id, p);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.party_created(fd, account_id, char_id, p);
	}
	if( HPMHooks.count.HP_mapif_party_created_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *account_id, int *char_id, struct party *p);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_created_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_party_created_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &account_id, &char_id, p);
		}
	}
	return retVal___;
}
void HP_mapif_party_noinfo(int fd, int party_id, int char_id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_party_noinfo_pre ) {
		void (*preHookFunc) (int *fd, int *party_id, int *char_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_noinfo_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_party_noinfo_pre[hIndex].func;
			preHookFunc(&fd, &party_id, &char_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.party_noinfo(fd, party_id, char_id);
	}
	if( HPMHooks.count.HP_mapif_party_noinfo_post ) {
		void (*postHookFunc) (int *fd, int *party_id, int *char_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_noinfo_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_party_noinfo_post[hIndex].func;
			postHookFunc(&fd, &party_id, &char_id);
		}
	}
	return;
}
void HP_mapif_party_info(int fd, struct party *p, int char_id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_party_info_pre ) {
		void (*preHookFunc) (int *fd, struct party *p, int *char_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_info_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_party_info_pre[hIndex].func;
			preHookFunc(&fd, p, &char_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.party_info(fd, p, char_id);
	}
	if( HPMHooks.count.HP_mapif_party_info_post ) {
		void (*postHookFunc) (int *fd, struct party *p, int *char_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_info_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_party_info_post[hIndex].func;
			postHookFunc(&fd, p, &char_id);
		}
	}
	return;
}
int HP_mapif_party_memberadded(int fd, int party_id, int account_id, int char_id, int flag) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_party_memberadded_pre ) {
		int (*preHookFunc) (int *fd, int *party_id, int *account_id, int *char_id, int *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_memberadded_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_party_memberadded_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &party_id, &account_id, &char_id, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.party_memberadded(fd, party_id, account_id, char_id, flag);
	}
	if( HPMHooks.count.HP_mapif_party_memberadded_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *party_id, int *account_id, int *char_id, int *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_memberadded_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_party_memberadded_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &party_id, &account_id, &char_id, &flag);
		}
	}
	return retVal___;
}
int HP_mapif_party_optionchanged(int fd, struct party *p, int account_id, int flag) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_party_optionchanged_pre ) {
		int (*preHookFunc) (int *fd, struct party *p, int *account_id, int *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_optionchanged_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_party_optionchanged_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, p, &account_id, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.party_optionchanged(fd, p, account_id, flag);
	}
	if( HPMHooks.count.HP_mapif_party_optionchanged_post ) {
		int (*postHookFunc) (int retVal___, int *fd, struct party *p, int *account_id, int *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_optionchanged_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_party_optionchanged_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, p, &account_id, &flag);
		}
	}
	return retVal___;
}
int HP_mapif_party_withdraw(int party_id, int account_id, int char_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_party_withdraw_pre ) {
		int (*preHookFunc) (int *party_id, int *account_id, int *char_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_withdraw_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_party_withdraw_pre[hIndex].func;
			retVal___ = preHookFunc(&party_id, &account_id, &char_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.party_withdraw(party_id, account_id, char_id);
	}
	if( HPMHooks.count.HP_mapif_party_withdraw_post ) {
		int (*postHookFunc) (int retVal___, int *party_id, int *account_id, int *char_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_withdraw_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_party_withdraw_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &party_id, &account_id, &char_id);
		}
	}
	return retVal___;
}
int HP_mapif_party_membermoved(struct party *p, int idx) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_party_membermoved_pre ) {
		int (*preHookFunc) (struct party *p, int *idx);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_membermoved_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_party_membermoved_pre[hIndex].func;
			retVal___ = preHookFunc(p, &idx);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.party_membermoved(p, idx);
	}
	if( HPMHooks.count.HP_mapif_party_membermoved_post ) {
		int (*postHookFunc) (int retVal___, struct party *p, int *idx);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_membermoved_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_party_membermoved_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, p, &idx);
		}
	}
	return retVal___;
}
int HP_mapif_party_broken(int party_id, int flag) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_party_broken_pre ) {
		int (*preHookFunc) (int *party_id, int *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_broken_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_party_broken_pre[hIndex].func;
			retVal___ = preHookFunc(&party_id, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.party_broken(party_id, flag);
	}
	if( HPMHooks.count.HP_mapif_party_broken_post ) {
		int (*postHookFunc) (int retVal___, int *party_id, int *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_broken_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_party_broken_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &party_id, &flag);
		}
	}
	return retVal___;
}
int HP_mapif_party_message(int party_id, int account_id, char *mes, int len, int sfd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_party_message_pre ) {
		int (*preHookFunc) (int *party_id, int *account_id, char *mes, int *len, int *sfd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_message_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_party_message_pre[hIndex].func;
			retVal___ = preHookFunc(&party_id, &account_id, mes, &len, &sfd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.party_message(party_id, account_id, mes, len, sfd);
	}
	if( HPMHooks.count.HP_mapif_party_message_post ) {
		int (*postHookFunc) (int retVal___, int *party_id, int *account_id, char *mes, int *len, int *sfd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_party_message_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_party_message_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &party_id, &account_id, mes, &len, &sfd);
		}
	}
	return retVal___;
}
int HP_mapif_parse_CreateParty(int fd, char *name, int item, int item2, struct party_member *leader) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_CreateParty_pre ) {
		int (*preHookFunc) (int *fd, char *name, int *item, int *item2, struct party_member *leader);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_CreateParty_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_CreateParty_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, name, &item, &item2, leader);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_CreateParty(fd, name, item, item2, leader);
	}
	if( HPMHooks.count.HP_mapif_parse_CreateParty_post ) {
		int (*postHookFunc) (int retVal___, int *fd, char *name, int *item, int *item2, struct party_member *leader);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_CreateParty_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_CreateParty_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, name, &item, &item2, leader);
		}
	}
	return retVal___;
}
void HP_mapif_parse_PartyInfo(int fd, int party_id, int char_id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_PartyInfo_pre ) {
		void (*preHookFunc) (int *fd, int *party_id, int *char_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyInfo_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_PartyInfo_pre[hIndex].func;
			preHookFunc(&fd, &party_id, &char_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_PartyInfo(fd, party_id, char_id);
	}
	if( HPMHooks.count.HP_mapif_parse_PartyInfo_post ) {
		void (*postHookFunc) (int *fd, int *party_id, int *char_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyInfo_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_PartyInfo_post[hIndex].func;
			postHookFunc(&fd, &party_id, &char_id);
		}
	}
	return;
}
int HP_mapif_parse_PartyAddMember(int fd, int party_id, struct party_member *member) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_PartyAddMember_pre ) {
		int (*preHookFunc) (int *fd, int *party_id, struct party_member *member);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyAddMember_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_PartyAddMember_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &party_id, member);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_PartyAddMember(fd, party_id, member);
	}
	if( HPMHooks.count.HP_mapif_parse_PartyAddMember_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *party_id, struct party_member *member);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyAddMember_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_PartyAddMember_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &party_id, member);
		}
	}
	return retVal___;
}
int HP_mapif_parse_PartyChangeOption(int fd, int party_id, int account_id, int exp, int item) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_PartyChangeOption_pre ) {
		int (*preHookFunc) (int *fd, int *party_id, int *account_id, int *exp, int *item);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyChangeOption_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_PartyChangeOption_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &party_id, &account_id, &exp, &item);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_PartyChangeOption(fd, party_id, account_id, exp, item);
	}
	if( HPMHooks.count.HP_mapif_parse_PartyChangeOption_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *party_id, int *account_id, int *exp, int *item);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyChangeOption_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_PartyChangeOption_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &party_id, &account_id, &exp, &item);
		}
	}
	return retVal___;
}
int HP_mapif_parse_PartyLeave(int fd, int party_id, int account_id, int char_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_PartyLeave_pre ) {
		int (*preHookFunc) (int *fd, int *party_id, int *account_id, int *char_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyLeave_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_PartyLeave_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &party_id, &account_id, &char_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_PartyLeave(fd, party_id, account_id, char_id);
	}
	if( HPMHooks.count.HP_mapif_parse_PartyLeave_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *party_id, int *account_id, int *char_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyLeave_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_PartyLeave_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &party_id, &account_id, &char_id);
		}
	}
	return retVal___;
}
int HP_mapif_parse_PartyChangeMap(int fd, int party_id, int account_id, int char_id, unsigned short map, int online, unsigned int lv) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_PartyChangeMap_pre ) {
		int (*preHookFunc) (int *fd, int *party_id, int *account_id, int *char_id, unsigned short *map, int *online, unsigned int *lv);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyChangeMap_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_PartyChangeMap_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &party_id, &account_id, &char_id, &map, &online, &lv);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_PartyChangeMap(fd, party_id, account_id, char_id, map, online, lv);
	}
	if( HPMHooks.count.HP_mapif_parse_PartyChangeMap_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *party_id, int *account_id, int *char_id, unsigned short *map, int *online, unsigned int *lv);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyChangeMap_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_PartyChangeMap_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &party_id, &account_id, &char_id, &map, &online, &lv);
		}
	}
	return retVal___;
}
int HP_mapif_parse_BreakParty(int fd, int party_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_BreakParty_pre ) {
		int (*preHookFunc) (int *fd, int *party_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_BreakParty_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_BreakParty_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &party_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_BreakParty(fd, party_id);
	}
	if( HPMHooks.count.HP_mapif_parse_BreakParty_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *party_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_BreakParty_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_BreakParty_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &party_id);
		}
	}
	return retVal___;
}
int HP_mapif_parse_PartyMessage(int fd, int party_id, int account_id, char *mes, int len) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_PartyMessage_pre ) {
		int (*preHookFunc) (int *fd, int *party_id, int *account_id, char *mes, int *len);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyMessage_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_PartyMessage_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &party_id, &account_id, mes, &len);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_PartyMessage(fd, party_id, account_id, mes, len);
	}
	if( HPMHooks.count.HP_mapif_parse_PartyMessage_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *party_id, int *account_id, char *mes, int *len);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyMessage_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_PartyMessage_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &party_id, &account_id, mes, &len);
		}
	}
	return retVal___;
}
int HP_mapif_parse_PartyLeaderChange(int fd, int party_id, int account_id, int char_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_PartyLeaderChange_pre ) {
		int (*preHookFunc) (int *fd, int *party_id, int *account_id, int *char_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyLeaderChange_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_PartyLeaderChange_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &party_id, &account_id, &char_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_PartyLeaderChange(fd, party_id, account_id, char_id);
	}
	if( HPMHooks.count.HP_mapif_parse_PartyLeaderChange_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *party_id, int *account_id, int *char_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_PartyLeaderChange_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_PartyLeaderChange_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &party_id, &account_id, &char_id);
		}
	}
	return retVal___;
}
int HP_mapif_pet_created(int fd, int account_id, struct s_pet *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_pet_created_pre ) {
		int (*preHookFunc) (int *fd, int *account_id, struct s_pet *p);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_pet_created_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_pet_created_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, p);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.pet_created(fd, account_id, p);
	}
	if( HPMHooks.count.HP_mapif_pet_created_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *account_id, struct s_pet *p);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_pet_created_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_pet_created_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &account_id, p);
		}
	}
	return retVal___;
}
int HP_mapif_pet_info(int fd, int account_id, struct s_pet *p) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_pet_info_pre ) {
		int (*preHookFunc) (int *fd, int *account_id, struct s_pet *p);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_pet_info_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_pet_info_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, p);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.pet_info(fd, account_id, p);
	}
	if( HPMHooks.count.HP_mapif_pet_info_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *account_id, struct s_pet *p);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_pet_info_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_pet_info_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &account_id, p);
		}
	}
	return retVal___;
}
int HP_mapif_pet_noinfo(int fd, int account_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_pet_noinfo_pre ) {
		int (*preHookFunc) (int *fd, int *account_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_pet_noinfo_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_pet_noinfo_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.pet_noinfo(fd, account_id);
	}
	if( HPMHooks.count.HP_mapif_pet_noinfo_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *account_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_pet_noinfo_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_pet_noinfo_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &account_id);
		}
	}
	return retVal___;
}
int HP_mapif_save_pet_ack(int fd, int account_id, int flag) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_save_pet_ack_pre ) {
		int (*preHookFunc) (int *fd, int *account_id, int *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_save_pet_ack_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_save_pet_ack_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.save_pet_ack(fd, account_id, flag);
	}
	if( HPMHooks.count.HP_mapif_save_pet_ack_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *account_id, int *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_save_pet_ack_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_save_pet_ack_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &account_id, &flag);
		}
	}
	return retVal___;
}
int HP_mapif_delete_pet_ack(int fd, int flag) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_delete_pet_ack_pre ) {
		int (*preHookFunc) (int *fd, int *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_delete_pet_ack_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_delete_pet_ack_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.delete_pet_ack(fd, flag);
	}
	if( HPMHooks.count.HP_mapif_delete_pet_ack_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_delete_pet_ack_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_delete_pet_ack_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &flag);
		}
	}
	return retVal___;
}
int HP_mapif_create_pet(int fd, int account_id, int char_id, short pet_class, short pet_lv, short pet_egg_id, short pet_equip, short intimate, short hungry, char rename_flag, char incubate, char *pet_name) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_create_pet_pre ) {
		int (*preHookFunc) (int *fd, int *account_id, int *char_id, short *pet_class, short *pet_lv, short *pet_egg_id, short *pet_equip, short *intimate, short *hungry, char *rename_flag, char *incubate, char *pet_name);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_create_pet_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_create_pet_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, &char_id, &pet_class, &pet_lv, &pet_egg_id, &pet_equip, &intimate, &hungry, &rename_flag, &incubate, pet_name);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.create_pet(fd, account_id, char_id, pet_class, pet_lv, pet_egg_id, pet_equip, intimate, hungry, rename_flag, incubate, pet_name);
	}
	if( HPMHooks.count.HP_mapif_create_pet_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *account_id, int *char_id, short *pet_class, short *pet_lv, short *pet_egg_id, short *pet_equip, short *intimate, short *hungry, char *rename_flag, char *incubate, char *pet_name);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_create_pet_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_create_pet_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &account_id, &char_id, &pet_class, &pet_lv, &pet_egg_id, &pet_equip, &intimate, &hungry, &rename_flag, &incubate, pet_name);
		}
	}
	return retVal___;
}
int HP_mapif_load_pet(int fd, int account_id, int char_id, int pet_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_load_pet_pre ) {
		int (*preHookFunc) (int *fd, int *account_id, int *char_id, int *pet_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_load_pet_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_load_pet_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, &char_id, &pet_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.load_pet(fd, account_id, char_id, pet_id);
	}
	if( HPMHooks.count.HP_mapif_load_pet_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *account_id, int *char_id, int *pet_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_load_pet_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_load_pet_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &account_id, &char_id, &pet_id);
		}
	}
	return retVal___;
}
int HP_mapif_save_pet(int fd, int account_id, struct s_pet *data) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_save_pet_pre ) {
		int (*preHookFunc) (int *fd, int *account_id, struct s_pet *data);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_save_pet_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_save_pet_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, data);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.save_pet(fd, account_id, data);
	}
	if( HPMHooks.count.HP_mapif_save_pet_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *account_id, struct s_pet *data);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_save_pet_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_save_pet_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &account_id, data);
		}
	}
	return retVal___;
}
int HP_mapif_delete_pet(int fd, int pet_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_delete_pet_pre ) {
		int (*preHookFunc) (int *fd, int *pet_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_delete_pet_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_delete_pet_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &pet_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.delete_pet(fd, pet_id);
	}
	if( HPMHooks.count.HP_mapif_delete_pet_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *pet_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_delete_pet_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_delete_pet_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &pet_id);
		}
	}
	return retVal___;
}
int HP_mapif_parse_CreatePet(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_CreatePet_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_CreatePet_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_CreatePet_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_CreatePet(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_CreatePet_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_CreatePet_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_CreatePet_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
int HP_mapif_parse_LoadPet(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_LoadPet_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_LoadPet_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_LoadPet_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_LoadPet(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_LoadPet_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_LoadPet_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_LoadPet_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
int HP_mapif_parse_SavePet(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_SavePet_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_SavePet_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_SavePet_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_SavePet(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_SavePet_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_SavePet_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_SavePet_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
int HP_mapif_parse_DeletePet(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_DeletePet_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_DeletePet_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_DeletePet_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_DeletePet(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_DeletePet_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_DeletePet_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_DeletePet_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
struct quest* HP_mapif_quests_fromsql(int char_id, int *count) {
	int hIndex = 0;
	struct quest* retVal___ = NULL;
	if( HPMHooks.count.HP_mapif_quests_fromsql_pre ) {
		struct quest* (*preHookFunc) (int *char_id, int *count);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_quests_fromsql_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_quests_fromsql_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, count);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.quests_fromsql(char_id, count);
	}
	if( HPMHooks.count.HP_mapif_quests_fromsql_post ) {
		struct quest* (*postHookFunc) (struct quest* retVal___, int *char_id, int *count);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_quests_fromsql_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_quests_fromsql_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &char_id, count);
		}
	}
	return retVal___;
}
bool HP_mapif_quest_delete(int char_id, int quest_id) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_mapif_quest_delete_pre ) {
		bool (*preHookFunc) (int *char_id, int *quest_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_quest_delete_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_quest_delete_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &quest_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.quest_delete(char_id, quest_id);
	}
	if( HPMHooks.count.HP_mapif_quest_delete_post ) {
		bool (*postHookFunc) (bool retVal___, int *char_id, int *quest_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_quest_delete_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_quest_delete_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &char_id, &quest_id);
		}
	}
	return retVal___;
}
bool HP_mapif_quest_add(int char_id, struct quest qd) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_mapif_quest_add_pre ) {
		bool (*preHookFunc) (int *char_id, struct quest *qd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_quest_add_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_quest_add_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &qd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.quest_add(char_id, qd);
	}
	if( HPMHooks.count.HP_mapif_quest_add_post ) {
		bool (*postHookFunc) (bool retVal___, int *char_id, struct quest *qd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_quest_add_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_quest_add_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &char_id, &qd);
		}
	}
	return retVal___;
}
bool HP_mapif_quest_update(int char_id, struct quest qd) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_mapif_quest_update_pre ) {
		bool (*preHookFunc) (int *char_id, struct quest *qd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_quest_update_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_quest_update_pre[hIndex].func;
			retVal___ = preHookFunc(&char_id, &qd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.quest_update(char_id, qd);
	}
	if( HPMHooks.count.HP_mapif_quest_update_post ) {
		bool (*postHookFunc) (bool retVal___, int *char_id, struct quest *qd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_quest_update_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_quest_update_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &char_id, &qd);
		}
	}
	return retVal___;
}
void HP_mapif_quest_save_ack(int fd, int char_id, bool success) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_quest_save_ack_pre ) {
		void (*preHookFunc) (int *fd, int *char_id, bool *success);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_quest_save_ack_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_quest_save_ack_pre[hIndex].func;
			preHookFunc(&fd, &char_id, &success);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.quest_save_ack(fd, char_id, success);
	}
	if( HPMHooks.count.HP_mapif_quest_save_ack_post ) {
		void (*postHookFunc) (int *fd, int *char_id, bool *success);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_quest_save_ack_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_quest_save_ack_post[hIndex].func;
			postHookFunc(&fd, &char_id, &success);
		}
	}
	return;
}
int HP_mapif_parse_quest_save(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_quest_save_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_quest_save_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_quest_save_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_quest_save(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_quest_save_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_quest_save_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_quest_save_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
void HP_mapif_send_quests(int fd, int char_id, struct quest *tmp_questlog, int num_quests) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_send_quests_pre ) {
		void (*preHookFunc) (int *fd, int *char_id, struct quest *tmp_questlog, int *num_quests);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_send_quests_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_send_quests_pre[hIndex].func;
			preHookFunc(&fd, &char_id, tmp_questlog, &num_quests);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.send_quests(fd, char_id, tmp_questlog, num_quests);
	}
	if( HPMHooks.count.HP_mapif_send_quests_post ) {
		void (*postHookFunc) (int *fd, int *char_id, struct quest *tmp_questlog, int *num_quests);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_send_quests_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_send_quests_post[hIndex].func;
			postHookFunc(&fd, &char_id, tmp_questlog, &num_quests);
		}
	}
	return;
}
int HP_mapif_parse_quest_load(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_quest_load_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_quest_load_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_quest_load_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_quest_load(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_quest_load_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_quest_load_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_quest_load_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
int HP_mapif_load_guild_storage(int fd, int account_id, int guild_id, char flag) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_load_guild_storage_pre ) {
		int (*preHookFunc) (int *fd, int *account_id, int *guild_id, char *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_load_guild_storage_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_load_guild_storage_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, &guild_id, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.load_guild_storage(fd, account_id, guild_id, flag);
	}
	if( HPMHooks.count.HP_mapif_load_guild_storage_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *account_id, int *guild_id, char *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_load_guild_storage_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_load_guild_storage_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &account_id, &guild_id, &flag);
		}
	}
	return retVal___;
}
int HP_mapif_save_guild_storage_ack(int fd, int account_id, int guild_id, int fail) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_save_guild_storage_ack_pre ) {
		int (*preHookFunc) (int *fd, int *account_id, int *guild_id, int *fail);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_save_guild_storage_ack_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_save_guild_storage_ack_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, &guild_id, &fail);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.save_guild_storage_ack(fd, account_id, guild_id, fail);
	}
	if( HPMHooks.count.HP_mapif_save_guild_storage_ack_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *account_id, int *guild_id, int *fail);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_save_guild_storage_ack_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_save_guild_storage_ack_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &account_id, &guild_id, &fail);
		}
	}
	return retVal___;
}
int HP_mapif_parse_LoadGuildStorage(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_LoadGuildStorage_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_LoadGuildStorage_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_LoadGuildStorage_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_LoadGuildStorage(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_LoadGuildStorage_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_LoadGuildStorage_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_LoadGuildStorage_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
int HP_mapif_parse_SaveGuildStorage(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_SaveGuildStorage_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_SaveGuildStorage_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_SaveGuildStorage_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_SaveGuildStorage(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_SaveGuildStorage_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_SaveGuildStorage_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_SaveGuildStorage_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
int HP_mapif_itembound_ack(int fd, int aid, int guild_id) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_itembound_ack_pre ) {
		int (*preHookFunc) (int *fd, int *aid, int *guild_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_itembound_ack_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_itembound_ack_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &aid, &guild_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.itembound_ack(fd, aid, guild_id);
	}
	if( HPMHooks.count.HP_mapif_itembound_ack_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *aid, int *guild_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_itembound_ack_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_itembound_ack_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &aid, &guild_id);
		}
	}
	return retVal___;
}
int HP_mapif_parse_ItemBoundRetrieve_sub(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_ItemBoundRetrieve_sub_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_ItemBoundRetrieve_sub_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_ItemBoundRetrieve_sub_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_ItemBoundRetrieve_sub(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_ItemBoundRetrieve_sub_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_ItemBoundRetrieve_sub_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_ItemBoundRetrieve_sub_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
void HP_mapif_parse_ItemBoundRetrieve(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_ItemBoundRetrieve_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_ItemBoundRetrieve_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_ItemBoundRetrieve_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_ItemBoundRetrieve(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_ItemBoundRetrieve_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_ItemBoundRetrieve_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_ItemBoundRetrieve_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_mapif_parse_accinfo(int fd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_accinfo_pre ) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_accinfo_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_accinfo_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_accinfo(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_accinfo_post ) {
		void (*postHookFunc) (int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_accinfo_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_accinfo_post[hIndex].func;
			postHookFunc(&fd);
		}
	}
	return;
}
void HP_mapif_parse_accinfo2(bool success, int map_fd, int u_fd, int u_aid, int account_id, const char *userid, const char *user_pass, const char *email, const char *last_ip, const char *lastlogin, const char *pin_code, const char *birthdate, int group_id, int logincount, int state) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_parse_accinfo2_pre ) {
		void (*preHookFunc) (bool *success, int *map_fd, int *u_fd, int *u_aid, int *account_id, const char *userid, const char *user_pass, const char *email, const char *last_ip, const char *lastlogin, const char *pin_code, const char *birthdate, int *group_id, int *logincount, int *state);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_accinfo2_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_accinfo2_pre[hIndex].func;
			preHookFunc(&success, &map_fd, &u_fd, &u_aid, &account_id, userid, user_pass, email, last_ip, lastlogin, pin_code, birthdate, &group_id, &logincount, &state);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.parse_accinfo2(success, map_fd, u_fd, u_aid, account_id, userid, user_pass, email, last_ip, lastlogin, pin_code, birthdate, group_id, logincount, state);
	}
	if( HPMHooks.count.HP_mapif_parse_accinfo2_post ) {
		void (*postHookFunc) (bool *success, int *map_fd, int *u_fd, int *u_aid, int *account_id, const char *userid, const char *user_pass, const char *email, const char *last_ip, const char *lastlogin, const char *pin_code, const char *birthdate, int *group_id, int *logincount, int *state);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_accinfo2_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_accinfo2_post[hIndex].func;
			postHookFunc(&success, &map_fd, &u_fd, &u_aid, &account_id, userid, user_pass, email, last_ip, lastlogin, pin_code, birthdate, &group_id, &logincount, &state);
		}
	}
	return;
}
int HP_mapif_broadcast(unsigned char *mes, int len, unsigned int fontColor, short fontType, short fontSize, short fontAlign, short fontY, int sfd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_broadcast_pre ) {
		int (*preHookFunc) (unsigned char *mes, int *len, unsigned int *fontColor, short *fontType, short *fontSize, short *fontAlign, short *fontY, int *sfd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_broadcast_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_broadcast_pre[hIndex].func;
			retVal___ = preHookFunc(mes, &len, &fontColor, &fontType, &fontSize, &fontAlign, &fontY, &sfd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.broadcast(mes, len, fontColor, fontType, fontSize, fontAlign, fontY, sfd);
	}
	if( HPMHooks.count.HP_mapif_broadcast_post ) {
		int (*postHookFunc) (int retVal___, unsigned char *mes, int *len, unsigned int *fontColor, short *fontType, short *fontSize, short *fontAlign, short *fontY, int *sfd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_broadcast_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_broadcast_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, mes, &len, &fontColor, &fontType, &fontSize, &fontAlign, &fontY, &sfd);
		}
	}
	return retVal___;
}
int HP_mapif_wis_message(struct WisData *wd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_wis_message_pre ) {
		int (*preHookFunc) (struct WisData *wd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_wis_message_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_wis_message_pre[hIndex].func;
			retVal___ = preHookFunc(wd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.wis_message(wd);
	}
	if( HPMHooks.count.HP_mapif_wis_message_post ) {
		int (*postHookFunc) (int retVal___, struct WisData *wd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_wis_message_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_wis_message_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, wd);
		}
	}
	return retVal___;
}
void HP_mapif_wis_response(int fd, unsigned char *src, int flag) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_wis_response_pre ) {
		void (*preHookFunc) (int *fd, unsigned char *src, int *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_wis_response_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_wis_response_pre[hIndex].func;
			preHookFunc(&fd, src, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.wis_response(fd, src, flag);
	}
	if( HPMHooks.count.HP_mapif_wis_response_post ) {
		void (*postHookFunc) (int *fd, unsigned char *src, int *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_wis_response_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_wis_response_post[hIndex].func;
			postHookFunc(&fd, src, &flag);
		}
	}
	return;
}
int HP_mapif_wis_end(struct WisData *wd, int flag) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_wis_end_pre ) {
		int (*preHookFunc) (struct WisData *wd, int *flag);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_wis_end_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_wis_end_pre[hIndex].func;
			retVal___ = preHookFunc(wd, &flag);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.wis_end(wd, flag);
	}
	if( HPMHooks.count.HP_mapif_wis_end_post ) {
		int (*postHookFunc) (int retVal___, struct WisData *wd, int *flag);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_wis_end_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_wis_end_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, wd, &flag);
		}
	}
	return retVal___;
}
int HP_mapif_account_reg_reply(int fd, int account_id, int char_id, int type) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_account_reg_reply_pre ) {
		int (*preHookFunc) (int *fd, int *account_id, int *char_id, int *type);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_account_reg_reply_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_account_reg_reply_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, &char_id, &type);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.account_reg_reply(fd, account_id, char_id, type);
	}
	if( HPMHooks.count.HP_mapif_account_reg_reply_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *account_id, int *char_id, int *type);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_account_reg_reply_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_account_reg_reply_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &account_id, &char_id, &type);
		}
	}
	return retVal___;
}
int HP_mapif_disconnectplayer(int fd, int account_id, int char_id, int reason) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_disconnectplayer_pre ) {
		int (*preHookFunc) (int *fd, int *account_id, int *char_id, int *reason);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_disconnectplayer_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_disconnectplayer_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &account_id, &char_id, &reason);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.disconnectplayer(fd, account_id, char_id, reason);
	}
	if( HPMHooks.count.HP_mapif_disconnectplayer_post ) {
		int (*postHookFunc) (int retVal___, int *fd, int *account_id, int *char_id, int *reason);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_disconnectplayer_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_disconnectplayer_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, &account_id, &char_id, &reason);
		}
	}
	return retVal___;
}
int HP_mapif_parse_broadcast(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_broadcast_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_broadcast_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_broadcast_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_broadcast(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_broadcast_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_broadcast_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_broadcast_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
int HP_mapif_parse_WisRequest(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_WisRequest_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_WisRequest_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_WisRequest_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_WisRequest(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_WisRequest_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_WisRequest_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_WisRequest_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
int HP_mapif_parse_WisReply(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_WisReply_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_WisReply_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_WisReply_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_WisReply(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_WisReply_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_WisReply_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_WisReply_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
int HP_mapif_parse_WisToGM(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_WisToGM_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_WisToGM_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_WisToGM_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_WisToGM(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_WisToGM_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_WisToGM_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_WisToGM_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
int HP_mapif_parse_Registry(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_Registry_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_Registry_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_Registry_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_Registry(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_Registry_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_Registry_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_Registry_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
int HP_mapif_parse_RegistryRequest(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_RegistryRequest_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_RegistryRequest_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_RegistryRequest_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_RegistryRequest(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_RegistryRequest_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_RegistryRequest_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_RegistryRequest_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
void HP_mapif_namechange_ack(int fd, int account_id, int char_id, int type, int flag, const char *name) {
	int hIndex = 0;
	if( HPMHooks.count.HP_mapif_namechange_ack_pre ) {
		void (*preHookFunc) (int *fd, int *account_id, int *char_id, int *type, int *flag, const char *name);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_namechange_ack_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_namechange_ack_pre[hIndex].func;
			preHookFunc(&fd, &account_id, &char_id, &type, &flag, name);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.mapif.namechange_ack(fd, account_id, char_id, type, flag, name);
	}
	if( HPMHooks.count.HP_mapif_namechange_ack_post ) {
		void (*postHookFunc) (int *fd, int *account_id, int *char_id, int *type, int *flag, const char *name);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_namechange_ack_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_namechange_ack_post[hIndex].func;
			postHookFunc(&fd, &account_id, &char_id, &type, &flag, name);
		}
	}
	return;
}
int HP_mapif_parse_NameChangeRequest(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_mapif_parse_NameChangeRequest_pre ) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_NameChangeRequest_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_mapif_parse_NameChangeRequest_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.mapif.parse_NameChangeRequest(fd);
	}
	if( HPMHooks.count.HP_mapif_parse_NameChangeRequest_post ) {
		int (*postHookFunc) (int retVal___, int *fd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_mapif_parse_NameChangeRequest_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_mapif_parse_NameChangeRequest_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd);
		}
	}
	return retVal___;
}
/* pincode */
void HP_pincode_handle(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_pincode_handle_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_pincode_handle_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_pincode_handle_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.pincode.handle(fd, sd);
	}
	if( HPMHooks.count.HP_pincode_handle_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_pincode_handle_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_pincode_handle_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
void HP_pincode_decrypt(unsigned int userSeed, char *pin) {
	int hIndex = 0;
	if( HPMHooks.count.HP_pincode_decrypt_pre ) {
		void (*preHookFunc) (unsigned int *userSeed, char *pin);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_pincode_decrypt_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_pincode_decrypt_pre[hIndex].func;
			preHookFunc(&userSeed, pin);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.pincode.decrypt(userSeed, pin);
	}
	if( HPMHooks.count.HP_pincode_decrypt_post ) {
		void (*postHookFunc) (unsigned int *userSeed, char *pin);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_pincode_decrypt_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_pincode_decrypt_post[hIndex].func;
			postHookFunc(&userSeed, pin);
		}
	}
	return;
}
void HP_pincode_error(int account_id) {
	int hIndex = 0;
	if( HPMHooks.count.HP_pincode_error_pre ) {
		void (*preHookFunc) (int *account_id);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_pincode_error_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_pincode_error_pre[hIndex].func;
			preHookFunc(&account_id);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.pincode.error(account_id);
	}
	if( HPMHooks.count.HP_pincode_error_post ) {
		void (*postHookFunc) (int *account_id);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_pincode_error_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_pincode_error_post[hIndex].func;
			postHookFunc(&account_id);
		}
	}
	return;
}
void HP_pincode_update(int account_id, char *pin) {
	int hIndex = 0;
	if( HPMHooks.count.HP_pincode_update_pre ) {
		void (*preHookFunc) (int *account_id, char *pin);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_pincode_update_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_pincode_update_pre[hIndex].func;
			preHookFunc(&account_id, pin);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.pincode.update(account_id, pin);
	}
	if( HPMHooks.count.HP_pincode_update_post ) {
		void (*postHookFunc) (int *account_id, char *pin);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_pincode_update_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_pincode_update_post[hIndex].func;
			postHookFunc(&account_id, pin);
		}
	}
	return;
}
void HP_pincode_sendstate(int fd, struct char_session_data *sd, uint16 state) {
	int hIndex = 0;
	if( HPMHooks.count.HP_pincode_sendstate_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd, uint16 *state);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_pincode_sendstate_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_pincode_sendstate_pre[hIndex].func;
			preHookFunc(&fd, sd, &state);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.pincode.sendstate(fd, sd, state);
	}
	if( HPMHooks.count.HP_pincode_sendstate_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd, uint16 *state);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_pincode_sendstate_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_pincode_sendstate_post[hIndex].func;
			postHookFunc(&fd, sd, &state);
		}
	}
	return;
}
void HP_pincode_setnew(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_pincode_setnew_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_pincode_setnew_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_pincode_setnew_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.pincode.setnew(fd, sd);
	}
	if( HPMHooks.count.HP_pincode_setnew_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_pincode_setnew_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_pincode_setnew_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
void HP_pincode_change(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_pincode_change_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_pincode_change_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_pincode_change_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.pincode.change(fd, sd);
	}
	if( HPMHooks.count.HP_pincode_change_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_pincode_change_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_pincode_change_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
int HP_pincode_compare(int fd, struct char_session_data *sd, char *pin) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_pincode_compare_pre ) {
		int (*preHookFunc) (int *fd, struct char_session_data *sd, char *pin);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_pincode_compare_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_pincode_compare_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, sd, pin);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.pincode.compare(fd, sd, pin);
	}
	if( HPMHooks.count.HP_pincode_compare_post ) {
		int (*postHookFunc) (int retVal___, int *fd, struct char_session_data *sd, char *pin);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_pincode_compare_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_pincode_compare_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &fd, sd, pin);
		}
	}
	return retVal___;
}
void HP_pincode_check(int fd, struct char_session_data *sd) {
	int hIndex = 0;
	if( HPMHooks.count.HP_pincode_check_pre ) {
		void (*preHookFunc) (int *fd, struct char_session_data *sd);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_pincode_check_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_pincode_check_pre[hIndex].func;
			preHookFunc(&fd, sd);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.pincode.check(fd, sd);
	}
	if( HPMHooks.count.HP_pincode_check_post ) {
		void (*postHookFunc) (int *fd, struct char_session_data *sd);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_pincode_check_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_pincode_check_post[hIndex].func;
			postHookFunc(&fd, sd);
		}
	}
	return;
}
bool HP_pincode_config_read(char *w1, char *w2) {
	int hIndex = 0;
	bool retVal___ = false;
	if( HPMHooks.count.HP_pincode_config_read_pre ) {
		bool (*preHookFunc) (char *w1, char *w2);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_pincode_config_read_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_pincode_config_read_pre[hIndex].func;
			retVal___ = preHookFunc(w1, w2);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.pincode.config_read(w1, w2);
	}
	if( HPMHooks.count.HP_pincode_config_read_post ) {
		bool (*postHookFunc) (bool retVal___, char *w1, char *w2);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_pincode_config_read_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_pincode_config_read_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, w1, w2);
		}
	}
	return retVal___;
}
