// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef CHAR_CHAR_H
#define CHAR_CHAR_H

#include "common/hercules.h"
#include "common/core.h" // CORE_ST_LAST
#include "common/db.h"
#include "common/mmo.h"

enum E_CHARSERVER_ST {
	CHARSERVER_ST_RUNNING = CORE_ST_LAST,
	CHARSERVER_ST_SHUTDOWN,
	CHARSERVER_ST_LAST
};

struct char_session_data {
	bool auth; // whether the session is authed or not
	int account_id, login_id1, login_id2, sex;
	int found_char[MAX_CHARS]; // ids of chars on this account
	time_t unban_time[MAX_CHARS]; // char unban time array
	char email[40]; // e-mail (default: a@a.com) by [Yor]
	time_t expiration_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
	int group_id; // permission
	uint8 char_slots;
	uint32 version;
	uint8 clienttype;
	char pincode[4+1];
	uint32 pincode_seed;
	uint16 pincode_try;
	uint32 pincode_change;
	char new_name[NAME_LENGTH];
	char birthdate[10+1];  // YYYY-MM-DD
};

struct online_char_data {
	int account_id;
	int char_id;
	int fd;
	int waiting_disconnect;
	short server; // -2: unknown server, -1: not connected, 0+: id of server
	int pincode_enable;
};

struct mmo_map_server {
	int fd;
	uint32 ip;
	uint16 port;
	int users;
	VECTOR_DECL(uint16) maps;
};

#define MAX_MAP_SERVERS 2

#define DEFAULT_AUTOSAVE_INTERVAL (300*1000)

enum {
	TABLE_INVENTORY,
	TABLE_CART,
	TABLE_STORAGE,
	TABLE_GUILD_STORAGE,
};

struct char_auth_node {
	int account_id;
	int char_id;
	uint32 login_id1;
	uint32 login_id2;
	uint32 ip;
	int sex;
	time_t expiration_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
	int group_id;
	unsigned changing_mapservers : 1;
};

/**
 * char interface
 **/
struct char_interface {
	struct mmo_map_server server[MAX_MAP_SERVERS];
	int login_fd;
	int char_fd;
	DBMap* online_char_db; // int account_id -> struct online_char_data*
	DBMap* char_db_;
	char userid[NAME_LENGTH];
	char passwd[NAME_LENGTH];
	char server_name[20];
	uint32 ip;
	uint16 port;
	int server_type;
	int new_display;

	char *CHAR_CONF_NAME;
	char *NET_CONF_NAME; ///< Network config filename
	char *SQL_CONF_NAME;
	char *INTER_CONF_NAME;

	int (*waiting_disconnect) (int tid, int64 tick, int id, intptr_t data);
	int (*delete_char_sql) (int char_id);
	DBData (*create_online_char_data) (DBKey key, va_list args);
	void (*set_account_online) (int account_id);
	void (*set_account_offline) (int account_id);
	void (*set_char_charselect) (int account_id);
	void (*set_char_online) (int map_id, int char_id, int account_id);
	void (*set_char_offline) (int char_id, int account_id);
	int (*db_setoffline) (DBKey key, DBData *data, va_list ap);
	int (*db_kickoffline) (DBKey key, DBData *data, va_list ap);
	void (*set_login_all_offline) (void);
	void (*set_all_offline) (int id);
	void (*set_all_offline_sql) (void);
	DBData (*create_charstatus) (DBKey key, va_list args);
	int (*mmo_char_tosql) (int char_id, struct mmo_charstatus* p);
	int (*memitemdata_to_sql) (const struct item items[], int max, int id, int tableswitch);
	int (*inventory_to_sql) (const struct item items[], int max, int id);
	int (*mmo_gender) (const struct char_session_data *sd, const struct mmo_charstatus *p, char sex);
	int (*mmo_chars_fromsql) (struct char_session_data* sd, uint8* buf);
	int (*mmo_char_fromsql) (int char_id, struct mmo_charstatus* p, bool load_everything);
	int (*mmo_char_sql_init) (void);
	bool (*char_slotchange) (struct char_session_data *sd, int fd, unsigned short from, unsigned short to);
	int (*rename_char_sql) (struct char_session_data *sd, int char_id);
	int (*check_char_name) (char * name, char * esc_name);
	int (*make_new_char_sql) (struct char_session_data* sd, char* name_, int str, int agi, int vit, int int_, int dex, int luk, int slot, int hair_color, int hair_style);
	int (*divorce_char_sql) (int partner_id1, int partner_id2);
	int (*count_users) (void);
	int (*mmo_char_tobuf) (uint8* buffer, struct mmo_charstatus* p);
	void (*mmo_char_send099d) (int fd, struct char_session_data *sd);
	void (*mmo_char_send_ban_list) (int fd, struct char_session_data *sd);
	void (*mmo_char_send_slots_info) (int fd, struct char_session_data* sd);
	int (*mmo_char_send_characters) (int fd, struct char_session_data* sd);
	int (*char_married) (int pl1, int pl2);
	int (*char_child) (int parent_id, int child_id);
	int (*char_family) (int cid1, int cid2, int cid3);
	void (*disconnect_player) (int account_id);
	void (*authfail_fd) (int fd, int type);
	void (*request_account_data) (int account_id);
	void (*auth_ok) (int fd, struct char_session_data *sd);
	void (*ping_login_server) (int fd);
	int (*parse_fromlogin_connection_state) (int fd);
	void (*auth_error) (int fd, unsigned char flag);
	void (*parse_fromlogin_auth_state) (int fd);
	void (*parse_fromlogin_account_data) (int fd);
	void (*parse_fromlogin_login_pong) (int fd);
	void (*changesex) (int account_id, int sex);
	int (*parse_fromlogin_changesex_reply) (int fd);
	void (*parse_fromlogin_account_reg2) (int fd);
	void (*parse_fromlogin_ban) (int fd);
	void (*parse_fromlogin_kick) (int fd);
	void (*update_ip) (int fd);
	void (*parse_fromlogin_update_ip) (int fd);
	void (*parse_fromlogin_accinfo2_failed) (int fd);
	void (*parse_fromlogin_accinfo2_ok) (int fd);
	int (*parse_fromlogin) (int fd);
	int (*request_accreg2) (int account_id, int char_id);
	void (*global_accreg_to_login_start) (int account_id, int char_id);
	void (*global_accreg_to_login_send) (void);
	void (*global_accreg_to_login_add) (const char *key, unsigned int index, intptr_t val, bool is_string);
	void (*read_fame_list) (void);
	int (*send_fame_list) (int fd);
	void (*update_fame_list) (int type, int index, int fame);
	int (*loadName) (int char_id, char* name);
	void (*parse_frommap_datasync) (int fd);
	void (*parse_frommap_skillid2idx) (int fd);
	void (*map_received_ok) (int fd);
	void (*send_maps) (int fd, int id, int j);
	void (*parse_frommap_map_names) (int fd, int id);
	void (*send_scdata) (int fd, int aid, int cid);
	void (*parse_frommap_request_scdata) (int fd);
	void (*parse_frommap_set_users_count) (int fd, int id);
	void (*parse_frommap_set_users) (int fd, int id);
	void (*save_character_ack) (int fd, int aid, int cid);
	void (*parse_frommap_save_character) (int fd, int id);
	void (*select_ack) (int fd, int account_id, uint8 flag);
	void (*parse_frommap_char_select_req) (int fd);
	void (*change_map_server_ack) (int fd, uint8 *data, bool ok);
	void (*parse_frommap_change_map_server) (int fd);
	void (*parse_frommap_remove_friend) (int fd);
	void (*char_name_ack) (int fd, int char_id);
	void (*parse_frommap_char_name_request) (int fd);
	void (*parse_frommap_change_email) (int fd);
	void (*ban) (int account_id, int char_id, time_t *unban_time, short year, short month, short day, short hour, short minute, short second);
	void (*unban) (int char_id, int *result);
	void (*ask_name_ack) (int fd, int acc, const char* name, int type, int result);
	int (*changecharsex) (int char_id, int sex);
	void (*parse_frommap_change_account) (int fd);
	void (*parse_frommap_fame_list) (int fd);
	void (*parse_frommap_divorce_char) (int fd);
	void (*parse_frommap_ragsrvinfo) (int fd);
	void (*parse_frommap_set_char_offline) (int fd);
	void (*parse_frommap_set_all_offline) (int fd, int id);
	void (*parse_frommap_set_char_online) (int fd, int id);
	void (*parse_frommap_build_fame_list) (int fd);
	void (*parse_frommap_save_status_change_data) (int fd);
	void (*send_pong) (int fd);
	void (*parse_frommap_ping) (int fd);
	void (*map_auth_ok) (int fd, int account_id, struct char_auth_node* node, struct mmo_charstatus* cd);
	void (*map_auth_failed) (int fd, int account_id, int char_id, int login_id1, char sex, uint32 ip);
	void (*parse_frommap_auth_request) (int fd, int id);
	void (*parse_frommap_update_ip) (int fd, int id);
	void (*parse_frommap_request_stats_report) (int fd);
	void (*parse_frommap_scdata_update) (int fd);
	void (*parse_frommap_scdata_delete) (int fd);
	int (*parse_frommap) (int fd);
	int (*search_mapserver) (unsigned short map, uint32 ip, uint16 port);
	int (*mapif_init) (int fd);
	uint32 (*lan_subnet_check) (uint32 ip);
	void (*delete2_ack) (int fd, int char_id, uint32 result, time_t delete_date);
	void (*delete2_accept_actual_ack) (int fd, int char_id, uint32 result);
	void (*delete2_accept_ack) (int fd, int char_id, uint32 result);
	void (*delete2_cancel_ack) (int fd, int char_id, uint32 result);
	void (*delete2_req) (int fd, struct char_session_data* sd);
	void (*delete2_accept) (int fd, struct char_session_data* sd);
	void (*delete2_cancel) (int fd, struct char_session_data* sd);
	void (*send_account_id) (int fd, int account_id);
	void (*parse_char_connect) (int fd, struct char_session_data* sd, uint32 ipl);
	void (*send_map_info) (int fd, int i, uint32 subnet_map_ip, struct mmo_charstatus *cd);
	void (*send_wait_char_server) (int fd);
	int (*search_default_maps_mapserver) (struct mmo_charstatus *cd);
	void (*parse_char_select) (int fd, struct char_session_data* sd, uint32 ipl);
	void (*creation_failed) (int fd, int result);
	void (*creation_ok) (int fd, struct mmo_charstatus *char_dat);
	void (*parse_char_create_new_char) (int fd, struct char_session_data* sd);
	void (*delete_char_failed) (int fd, int flag);
	void (*delete_char_ok) (int fd);
	void (*parse_char_delete_char) (int fd, struct char_session_data* sd, unsigned short cmd);
	void (*parse_char_ping) (int fd);
	void (*allow_rename) (int fd, int flag);
	void (*parse_char_rename_char) (int fd, struct char_session_data* sd);
	void (*parse_char_rename_char2) (int fd, struct char_session_data* sd);
	void (*rename_char_ack) (int fd, int flag);
	void (*parse_char_rename_char_confirm) (int fd, struct char_session_data* sd);
	void (*captcha_notsupported) (int fd);
	void (*parse_char_request_captcha) (int fd);
	void (*parse_char_check_captcha) (int fd);
	void (*parse_char_delete2_req) (int fd, struct char_session_data* sd);
	void (*parse_char_delete2_accept) (int fd, struct char_session_data* sd);
	void (*parse_char_delete2_cancel) (int fd, struct char_session_data* sd);
	void (*login_map_server_ack) (int fd, uint8 flag);
	void (*parse_char_login_map_server) (int fd, uint32 ipl);
	void (*parse_char_pincode_check) (int fd, struct char_session_data* sd);
	void (*parse_char_pincode_window) (int fd, struct char_session_data* sd);
	void (*parse_char_pincode_change) (int fd, struct char_session_data* sd);
	void (*parse_char_pincode_first_pin) (int fd, struct char_session_data* sd);
	void (*parse_char_request_chars) (int fd, struct char_session_data* sd);
	void (*change_character_slot_ack) (int fd, bool ret);
	void (*parse_char_move_character) (int fd, struct char_session_data* sd);
	int (*parse_char_unknown_packet) (int fd, uint32 ipl);
	int (*parse_char) (int fd);
	int (*broadcast_user_count) (int tid, int64 tick, int id, intptr_t data);
	int (*send_accounts_tologin_sub) (DBKey key, DBData *data, va_list ap);
	int (*send_accounts_tologin) (int tid, int64 tick, int id, intptr_t data);
	int (*check_connect_login_server) (int tid, int64 tick, int id, intptr_t data);
	int (*online_data_cleanup_sub) (DBKey key, DBData *data, va_list ap);
	int (*online_data_cleanup) (int tid, int64 tick, int id, intptr_t data);
	void (*sql_config_read) (const char* cfgName);
	void (*config_dispatch) (char *w1, char *w2);
	int (*config_read) (const char* cfgName);
};

#ifdef HERCULES_CORE
extern int char_name_option;
extern char char_name_letters[];
extern bool char_gm_read;
extern int autosave_interval;
extern int save_log;
extern char db_path[];
extern char char_db[256];
extern char scdata_db[256];
extern char cart_db[256];
extern char inventory_db[256];
extern char charlog_db[256];
extern char storage_db[256];
extern char interlog_db[256];
extern char skill_db[256];
extern char memo_db[256];
extern char guild_db[256];
extern char guild_alliance_db[256];
extern char guild_castle_db[256];
extern char guild_expulsion_db[256];
extern char guild_member_db[256];
extern char guild_position_db[256];
extern char guild_skill_db[256];
extern char guild_storage_db[256];
extern char party_db[256];
extern char pet_db[256];
extern char mail_db[256];
extern char auction_db[256];
extern char quest_db[256];
extern char homunculus_db[256];
extern char skill_homunculus_db[256];
extern char mercenary_db[256];
extern char mercenary_owner_db[256];
extern char ragsrvinfo_db[256];
extern char elemental_db[256];
extern char acc_reg_num_db[32];
extern char acc_reg_str_db[32];
extern char char_reg_str_db[32];
extern char char_reg_num_db[32];

extern int guild_exp_rate;
extern int log_inter;

void char_load_defaults();
void char_defaults();
#endif // HERCULES_CORE

HPShared struct char_interface *chr;

#endif /* CHAR_CHAR_H */
