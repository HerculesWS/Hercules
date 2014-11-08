// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
//
// NOTE: This file was auto-generated and should never be manually edited,
//       as it will get overwritten.

/* login */
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
DBData HP_login_create_online_user(DBKey key, va_list args) {
	int hIndex = 0;
	DBData retVal___;
	memset(&retVal___, '\0', sizeof(DBData));
	if( HPMHooks.count.HP_login_create_online_user_pre ) {
		DBData (*preHookFunc) (DBKey *key, va_list args);
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
		DBData (*postHookFunc) (DBData retVal___, DBKey *key, va_list args);
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
int HP_login_online_db_setoffline(DBKey key, DBData *data, va_list ap) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_login_online_db_setoffline_pre ) {
		int (*preHookFunc) (DBKey *key, DBData *data, va_list ap);
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
		int (*postHookFunc) (int retVal___, DBKey *key, DBData *data, va_list ap);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_online_db_setoffline_post; hIndex++ ) {
			va_list ap___copy; va_copy(ap___copy, ap);
			postHookFunc = HPMHooks.list.HP_login_online_db_setoffline_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &key, data, ap___copy);
			va_end(ap___copy);
		}
	}
	return retVal___;
}
int HP_login_online_data_cleanup_sub(DBKey key, DBData *data, va_list ap) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_login_online_data_cleanup_sub_pre ) {
		int (*preHookFunc) (DBKey *key, DBData *data, va_list ap);
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
		int (*postHookFunc) (int retVal___, DBKey *key, DBData *data, va_list ap);
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
int HP_login_lan_subnetcheck(uint32 ip) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_login_lan_subnetcheck_pre ) {
		int (*preHookFunc) (uint32 *ip);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_lan_subnetcheck_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_lan_subnetcheck_pre[hIndex].func;
			retVal___ = preHookFunc(&ip);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.login.lan_subnetcheck(ip);
	}
	if( HPMHooks.count.HP_login_lan_subnetcheck_post ) {
		int (*postHookFunc) (int retVal___, uint32 *ip);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_lan_subnetcheck_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_lan_subnetcheck_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, &ip);
		}
	}
	return retVal___;
}
int HP_login_lan_config_read(const char *lancfgName) {
	int hIndex = 0;
	int retVal___ = 0;
	if( HPMHooks.count.HP_login_lan_config_read_pre ) {
		int (*preHookFunc) (const char *lancfgName);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_lan_config_read_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_lan_config_read_pre[hIndex].func;
			retVal___ = preHookFunc(lancfgName);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.login.lan_config_read(lancfgName);
	}
	if( HPMHooks.count.HP_login_lan_config_read_post ) {
		int (*postHookFunc) (int retVal___, const char *lancfgName);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_lan_config_read_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_lan_config_read_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, lancfgName);
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
void HP_login_parse_request_connection(int fd, struct login_session_data *sd, const char *ip) {
	int hIndex = 0;
	if( HPMHooks.count.HP_login_parse_request_connection_pre ) {
		void (*preHookFunc) (int *fd, struct login_session_data *sd, const char *ip);
		*HPMforce_return = false;
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_parse_request_connection_pre; hIndex++ ) {
			preHookFunc = HPMHooks.list.HP_login_parse_request_connection_pre[hIndex].func;
			preHookFunc(&fd, sd, ip);
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.login.parse_request_connection(fd, sd, ip);
	}
	if( HPMHooks.count.HP_login_parse_request_connection_post ) {
		void (*postHookFunc) (int *fd, struct login_session_data *sd, const char *ip);
		for(hIndex = 0; hIndex < HPMHooks.count.HP_login_parse_request_connection_post; hIndex++ ) {
			postHookFunc = HPMHooks.list.HP_login_parse_request_connection_post[hIndex].func;
			postHookFunc(&fd, sd, ip);
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
