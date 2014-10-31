// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
//
// NOTE: This file was auto-generated and should never be manually edited,
//       as it will get overwritten.

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
