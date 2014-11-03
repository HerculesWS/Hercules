// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
//
// NOTE: This file was auto-generated and should never be manually edited,
//       as it will get overwritten.

struct {
	struct HPMHookPoint *HP_pincode_handle_pre;
	struct HPMHookPoint *HP_pincode_handle_post;
	struct HPMHookPoint *HP_pincode_decrypt_pre;
	struct HPMHookPoint *HP_pincode_decrypt_post;
	struct HPMHookPoint *HP_pincode_error_pre;
	struct HPMHookPoint *HP_pincode_error_post;
	struct HPMHookPoint *HP_pincode_update_pre;
	struct HPMHookPoint *HP_pincode_update_post;
	struct HPMHookPoint *HP_pincode_sendstate_pre;
	struct HPMHookPoint *HP_pincode_sendstate_post;
	struct HPMHookPoint *HP_pincode_setnew_pre;
	struct HPMHookPoint *HP_pincode_setnew_post;
	struct HPMHookPoint *HP_pincode_change_pre;
	struct HPMHookPoint *HP_pincode_change_post;
	struct HPMHookPoint *HP_pincode_compare_pre;
	struct HPMHookPoint *HP_pincode_compare_post;
	struct HPMHookPoint *HP_pincode_check_pre;
	struct HPMHookPoint *HP_pincode_check_post;
	struct HPMHookPoint *HP_pincode_config_read_pre;
	struct HPMHookPoint *HP_pincode_config_read_post;
} list;

struct {
	int HP_pincode_handle_pre;
	int HP_pincode_handle_post;
	int HP_pincode_decrypt_pre;
	int HP_pincode_decrypt_post;
	int HP_pincode_error_pre;
	int HP_pincode_error_post;
	int HP_pincode_update_pre;
	int HP_pincode_update_post;
	int HP_pincode_sendstate_pre;
	int HP_pincode_sendstate_post;
	int HP_pincode_setnew_pre;
	int HP_pincode_setnew_post;
	int HP_pincode_change_pre;
	int HP_pincode_change_post;
	int HP_pincode_compare_pre;
	int HP_pincode_compare_post;
	int HP_pincode_check_pre;
	int HP_pincode_check_post;
	int HP_pincode_config_read_pre;
	int HP_pincode_config_read_post;
} count;

struct {
	struct pincode_interface pincode;
} source;
