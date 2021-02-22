/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-2021 Hercules Dev Team
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
/* aclif_interface */
int HP_aclif_init(bool minimal) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_aclif_init_pre > 0) {
		int (*preHookFunc) (bool *minimal);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_init_pre[hIndex].func;
			retVal___ = preHookFunc(&minimal);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aclif.init(minimal);
	}
	if (HPMHooks.count.HP_aclif_init_post > 0) {
		int (*postHookFunc) (int retVal___, bool minimal);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, minimal);
		}
	}
	return retVal___;
}
void HP_aclif_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.final();
	}
	if (HPMHooks.count.HP_aclif_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
bool HP_aclif_setip(const char *ip) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_aclif_setip_pre > 0) {
		bool (*preHookFunc) (const char **ip);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_setip_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_setip_pre[hIndex].func;
			retVal___ = preHookFunc(&ip);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aclif.setip(ip);
	}
	if (HPMHooks.count.HP_aclif_setip_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *ip);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_setip_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_setip_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, ip);
		}
	}
	return retVal___;
}
bool HP_aclif_setbindip(const char *ip) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_aclif_setbindip_pre > 0) {
		bool (*preHookFunc) (const char **ip);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_setbindip_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_setbindip_pre[hIndex].func;
			retVal___ = preHookFunc(&ip);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aclif.setbindip(ip);
	}
	if (HPMHooks.count.HP_aclif_setbindip_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *ip);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_setbindip_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_setbindip_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, ip);
		}
	}
	return retVal___;
}
void HP_aclif_setport(uint16 port) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_setport_pre > 0) {
		void (*preHookFunc) (uint16 *port);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_setport_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_setport_pre[hIndex].func;
			preHookFunc(&port);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.setport(port);
	}
	if (HPMHooks.count.HP_aclif_setport_post > 0) {
		void (*postHookFunc) (uint16 port);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_setport_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_setport_post[hIndex].func;
			postHookFunc(port);
		}
	}
	return;
}
uint32 HP_aclif_refresh_ip(void) {
	int hIndex = 0;
	uint32 retVal___ = 0;
	if (HPMHooks.count.HP_aclif_refresh_ip_pre > 0) {
		uint32 (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_refresh_ip_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_refresh_ip_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aclif.refresh_ip();
	}
	if (HPMHooks.count.HP_aclif_refresh_ip_post > 0) {
		uint32 (*postHookFunc) (uint32 retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_refresh_ip_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_refresh_ip_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
int HP_aclif_parse(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_aclif_parse_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_parse_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_parse_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aclif.parse(fd);
	}
	if (HPMHooks.count.HP_aclif_parse_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_parse_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_parse_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_aclif_parse_request(int fd, struct api_session_data *sd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_aclif_parse_request_pre > 0) {
		int (*preHookFunc) (int *fd, struct api_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_parse_request_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_parse_request_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aclif.parse_request(fd, sd);
	}
	if (HPMHooks.count.HP_aclif_parse_request_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, struct api_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_parse_request_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_parse_request_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, sd);
		}
	}
	return retVal___;
}
void HP_aclif_terminate_connection(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_terminate_connection_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_terminate_connection_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_terminate_connection_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.terminate_connection(fd);
	}
	if (HPMHooks.count.HP_aclif_terminate_connection_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_terminate_connection_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_terminate_connection_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
int HP_aclif_connected(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_aclif_connected_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_connected_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_connected_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aclif.connected(fd);
	}
	if (HPMHooks.count.HP_aclif_connected_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_connected_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_connected_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_aclif_session_delete(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_aclif_session_delete_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_session_delete_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_session_delete_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aclif.session_delete(fd);
	}
	if (HPMHooks.count.HP_aclif_session_delete_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_session_delete_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_session_delete_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
void HP_aclif_init_handlers(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_init_handlers_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_init_handlers_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_init_handlers_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.init_handlers();
	}
	if (HPMHooks.count.HP_aclif_init_handlers_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_init_handlers_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_init_handlers_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_aclif_register_handlers(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_register_handlers_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_register_handlers_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_register_handlers_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.register_handlers();
	}
	if (HPMHooks.count.HP_aclif_register_handlers_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_register_handlers_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_register_handlers_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_aclif_add_handler(http_method method, const char *url, HttpParseHandler func, Handler_func func2, int msg_id, int flags) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_add_handler_pre > 0) {
		void (*preHookFunc) (http_method *method, const char **url, HttpParseHandler *func, Handler_func *func2, int *msg_id, int *flags);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_add_handler_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_add_handler_pre[hIndex].func;
			preHookFunc(&method, &url, &func, &func2, &msg_id, &flags);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.add_handler(method, url, func, func2, msg_id, flags);
	}
	if (HPMHooks.count.HP_aclif_add_handler_post > 0) {
		void (*postHookFunc) (http_method method, const char *url, HttpParseHandler func, Handler_func func2, int msg_id, int flags);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_add_handler_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_add_handler_post[hIndex].func;
			postHookFunc(method, url, func, func2, msg_id, flags);
		}
	}
	return;
}
void HP_aclif_add_packet_handler(Handler_func func2, int msg_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_add_packet_handler_pre > 0) {
		void (*preHookFunc) (Handler_func *func2, int *msg_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_add_packet_handler_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_add_packet_handler_pre[hIndex].func;
			preHookFunc(&func2, &msg_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.add_packet_handler(func2, msg_id);
	}
	if (HPMHooks.count.HP_aclif_add_packet_handler_post > 0) {
		void (*postHookFunc) (Handler_func func2, int msg_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_add_packet_handler_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_add_packet_handler_post[hIndex].func;
			postHookFunc(func2, msg_id);
		}
	}
	return;
}
void HP_aclif_set_url(int fd, http_method method, const char *url, size_t size) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_set_url_pre > 0) {
		void (*preHookFunc) (int *fd, http_method *method, const char **url, size_t *size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_set_url_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_set_url_pre[hIndex].func;
			preHookFunc(&fd, &method, &url, &size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.set_url(fd, method, url, size);
	}
	if (HPMHooks.count.HP_aclif_set_url_post > 0) {
		void (*postHookFunc) (int fd, http_method method, const char *url, size_t size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_set_url_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_set_url_post[hIndex].func;
			postHookFunc(fd, method, url, size);
		}
	}
	return;
}
void HP_aclif_set_body(int fd, const char *body, size_t size) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_set_body_pre > 0) {
		void (*preHookFunc) (int *fd, const char **body, size_t *size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_set_body_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_set_body_pre[hIndex].func;
			preHookFunc(&fd, &body, &size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.set_body(fd, body, size);
	}
	if (HPMHooks.count.HP_aclif_set_body_post > 0) {
		void (*postHookFunc) (int fd, const char *body, size_t size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_set_body_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_set_body_post[hIndex].func;
			postHookFunc(fd, body, size);
		}
	}
	return;
}
void HP_aclif_set_header_name(int fd, const char *name, size_t size) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_set_header_name_pre > 0) {
		void (*preHookFunc) (int *fd, const char **name, size_t *size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_set_header_name_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_set_header_name_pre[hIndex].func;
			preHookFunc(&fd, &name, &size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.set_header_name(fd, name, size);
	}
	if (HPMHooks.count.HP_aclif_set_header_name_post > 0) {
		void (*postHookFunc) (int fd, const char *name, size_t size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_set_header_name_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_set_header_name_post[hIndex].func;
			postHookFunc(fd, name, size);
		}
	}
	return;
}
void HP_aclif_set_header_value(int fd, const char *value, size_t size) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_set_header_value_pre > 0) {
		void (*preHookFunc) (int *fd, const char **value, size_t *size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_set_header_value_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_set_header_value_pre[hIndex].func;
			preHookFunc(&fd, &value, &size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.set_header_value(fd, value, size);
	}
	if (HPMHooks.count.HP_aclif_set_header_value_post > 0) {
		void (*postHookFunc) (int fd, const char *value, size_t size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_set_header_value_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_set_header_value_post[hIndex].func;
			postHookFunc(fd, value, size);
		}
	}
	return;
}
void HP_aclif_set_post_header_name(int fd, const char *name, size_t size) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_set_post_header_name_pre > 0) {
		void (*preHookFunc) (int *fd, const char **name, size_t *size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_set_post_header_name_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_set_post_header_name_pre[hIndex].func;
			preHookFunc(&fd, &name, &size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.set_post_header_name(fd, name, size);
	}
	if (HPMHooks.count.HP_aclif_set_post_header_name_post > 0) {
		void (*postHookFunc) (int fd, const char *name, size_t size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_set_post_header_name_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_set_post_header_name_post[hIndex].func;
			postHookFunc(fd, name, size);
		}
	}
	return;
}
void HP_aclif_set_post_header_value(int fd, const char *value, size_t size) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_set_post_header_value_pre > 0) {
		void (*preHookFunc) (int *fd, const char **value, size_t *size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_set_post_header_value_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_set_post_header_value_pre[hIndex].func;
			preHookFunc(&fd, &value, &size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.set_post_header_value(fd, value, size);
	}
	if (HPMHooks.count.HP_aclif_set_post_header_value_post > 0) {
		void (*postHookFunc) (int fd, const char *value, size_t size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_set_post_header_value_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_set_post_header_value_post[hIndex].func;
			postHookFunc(fd, value, size);
		}
	}
	return;
}
void HP_aclif_set_post_header_data(int fd, const char *data, size_t size) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_set_post_header_data_pre > 0) {
		void (*preHookFunc) (int *fd, const char **data, size_t *size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_set_post_header_data_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_set_post_header_data_pre[hIndex].func;
			preHookFunc(&fd, &data, &size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.set_post_header_data(fd, data, size);
	}
	if (HPMHooks.count.HP_aclif_set_post_header_data_post > 0) {
		void (*postHookFunc) (int fd, const char *data, size_t size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_set_post_header_data_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_set_post_header_data_post[hIndex].func;
			postHookFunc(fd, data, size);
		}
	}
	return;
}
bool HP_aclif_check_header(int fd, struct api_session_data *sd, const char *name, const char *value, size_t value_size) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_aclif_check_header_pre > 0) {
		bool (*preHookFunc) (int *fd, struct api_session_data **sd, const char **name, const char **value, size_t *value_size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_check_header_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_check_header_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &sd, &name, &value, &value_size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aclif.check_header(fd, sd, name, value, value_size);
	}
	if (HPMHooks.count.HP_aclif_check_header_post > 0) {
		bool (*postHookFunc) (bool retVal___, int fd, struct api_session_data *sd, const char *name, const char *value, size_t value_size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_check_header_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_check_header_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, sd, name, value, value_size);
		}
	}
	return retVal___;
}
void HP_aclif_multi_part_start(int fd, struct api_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_multi_part_start_pre > 0) {
		void (*preHookFunc) (int *fd, struct api_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_multi_part_start_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_multi_part_start_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.multi_part_start(fd, sd);
	}
	if (HPMHooks.count.HP_aclif_multi_part_start_post > 0) {
		void (*postHookFunc) (int fd, struct api_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_multi_part_start_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_multi_part_start_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_aclif_multi_part_complete(int fd, struct api_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_multi_part_complete_pre > 0) {
		void (*preHookFunc) (int *fd, struct api_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_multi_part_complete_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_multi_part_complete_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.multi_part_complete(fd, sd);
	}
	if (HPMHooks.count.HP_aclif_multi_part_complete_post > 0) {
		void (*postHookFunc) (int fd, struct api_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_multi_part_complete_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_multi_part_complete_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_aclif_multi_body_complete(int fd, struct api_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_multi_body_complete_pre > 0) {
		void (*preHookFunc) (int *fd, struct api_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_multi_body_complete_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_multi_body_complete_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.multi_body_complete(fd, sd);
	}
	if (HPMHooks.count.HP_aclif_multi_body_complete_post > 0) {
		void (*postHookFunc) (int fd, struct api_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_multi_body_complete_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_multi_body_complete_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
int HP_aclif_post_headers_destroy_sub(union DBKey key, struct DBData *data, va_list ap) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_aclif_post_headers_destroy_sub_pre > 0) {
		int (*preHookFunc) (union DBKey *key, struct DBData **data, va_list ap);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_post_headers_destroy_sub_pre; hIndex++) {
			va_list ap___copy; va_copy(ap___copy, ap);
			preHookFunc = HPMHooks.list.HP_aclif_post_headers_destroy_sub_pre[hIndex].func;
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
		retVal___ = HPMHooks.source.aclif.post_headers_destroy_sub(key, data, ap___copy);
		va_end(ap___copy);
	}
	if (HPMHooks.count.HP_aclif_post_headers_destroy_sub_post > 0) {
		int (*postHookFunc) (int retVal___, union DBKey key, struct DBData *data, va_list ap);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_post_headers_destroy_sub_post; hIndex++) {
			va_list ap___copy; va_copy(ap___copy, ap);
			postHookFunc = HPMHooks.list.HP_aclif_post_headers_destroy_sub_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, key, data, ap___copy);
			va_end(ap___copy);
		}
	}
	return retVal___;
}
void HP_aclif_reportError(int fd, struct api_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_reportError_pre > 0) {
		void (*preHookFunc) (int *fd, struct api_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_reportError_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_reportError_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.reportError(fd, sd);
	}
	if (HPMHooks.count.HP_aclif_reportError_post > 0) {
		void (*postHookFunc) (int fd, struct api_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_reportError_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_reportError_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_aclif_check_headers(int fd, struct api_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_check_headers_pre > 0) {
		void (*preHookFunc) (int *fd, struct api_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_check_headers_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_check_headers_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.check_headers(fd, sd);
	}
	if (HPMHooks.count.HP_aclif_check_headers_post > 0) {
		void (*postHookFunc) (int fd, struct api_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_check_headers_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_check_headers_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
bool HP_aclif_decode_post_headers(int fd, struct api_session_data *sd) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_aclif_decode_post_headers_pre > 0) {
		bool (*preHookFunc) (int *fd, struct api_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_decode_post_headers_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_decode_post_headers_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aclif.decode_post_headers(fd, sd);
	}
	if (HPMHooks.count.HP_aclif_decode_post_headers_post > 0) {
		bool (*postHookFunc) (bool retVal___, int fd, struct api_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_decode_post_headers_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_decode_post_headers_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, sd);
		}
	}
	return retVal___;
}
int HP_aclif_print_header(union DBKey key, struct DBData *data, va_list ap) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_aclif_print_header_pre > 0) {
		int (*preHookFunc) (union DBKey *key, struct DBData **data, va_list ap);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_print_header_pre; hIndex++) {
			va_list ap___copy; va_copy(ap___copy, ap);
			preHookFunc = HPMHooks.list.HP_aclif_print_header_pre[hIndex].func;
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
		retVal___ = HPMHooks.source.aclif.print_header(key, data, ap___copy);
		va_end(ap___copy);
	}
	if (HPMHooks.count.HP_aclif_print_header_post > 0) {
		int (*postHookFunc) (int retVal___, union DBKey key, struct DBData *data, va_list ap);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_print_header_post; hIndex++) {
			va_list ap___copy; va_copy(ap___copy, ap);
			postHookFunc = HPMHooks.list.HP_aclif_print_header_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, key, data, ap___copy);
			va_end(ap___copy);
		}
	}
	return retVal___;
}
bool HP_aclif_is_post_header_present(struct api_session_data *sd, const char *name) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_aclif_is_post_header_present_pre > 0) {
		bool (*preHookFunc) (struct api_session_data **sd, const char **name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_is_post_header_present_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_is_post_header_present_pre[hIndex].func;
			retVal___ = preHookFunc(&sd, &name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aclif.is_post_header_present(sd, name);
	}
	if (HPMHooks.count.HP_aclif_is_post_header_present_post > 0) {
		bool (*postHookFunc) (bool retVal___, struct api_session_data *sd, const char *name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_is_post_header_present_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_is_post_header_present_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, sd, name);
		}
	}
	return retVal___;
}
bool HP_aclif_get_post_header_data_int(struct api_session_data *sd, const char *name, int *account_id) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_aclif_get_post_header_data_int_pre > 0) {
		bool (*preHookFunc) (struct api_session_data **sd, const char **name, int **account_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_get_post_header_data_int_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_get_post_header_data_int_pre[hIndex].func;
			retVal___ = preHookFunc(&sd, &name, &account_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aclif.get_post_header_data_int(sd, name, account_id);
	}
	if (HPMHooks.count.HP_aclif_get_post_header_data_int_post > 0) {
		bool (*postHookFunc) (bool retVal___, struct api_session_data *sd, const char *name, int *account_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_get_post_header_data_int_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_get_post_header_data_int_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, sd, name, account_id);
		}
	}
	return retVal___;
}
bool HP_aclif_get_post_header_data_str(struct api_session_data *sd, const char *name, char **data, uint32_t *data_size) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_aclif_get_post_header_data_str_pre > 0) {
		bool (*preHookFunc) (struct api_session_data **sd, const char **name, char ***data, uint32_t **data_size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_get_post_header_data_str_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_get_post_header_data_str_pre[hIndex].func;
			retVal___ = preHookFunc(&sd, &name, &data, &data_size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aclif.get_post_header_data_str(sd, name, data, data_size);
	}
	if (HPMHooks.count.HP_aclif_get_post_header_data_str_post > 0) {
		bool (*postHookFunc) (bool retVal___, struct api_session_data *sd, const char *name, char **data, uint32_t *data_size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_get_post_header_data_str_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_get_post_header_data_str_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, sd, name, data, data_size);
		}
	}
	return retVal___;
}
bool HP_aclif_get_post_header_data_json(struct api_session_data *sd, const char *name, JsonP **json) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_aclif_get_post_header_data_json_pre > 0) {
		bool (*preHookFunc) (struct api_session_data **sd, const char **name, JsonP ***json);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_get_post_header_data_json_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_get_post_header_data_json_pre[hIndex].func;
			retVal___ = preHookFunc(&sd, &name, &json);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aclif.get_post_header_data_json(sd, name, json);
	}
	if (HPMHooks.count.HP_aclif_get_post_header_data_json_post > 0) {
		bool (*postHookFunc) (bool retVal___, struct api_session_data *sd, const char *name, JsonP **json);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_get_post_header_data_json_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_get_post_header_data_json_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, sd, name, json);
		}
	}
	return retVal___;
}
bool HP_aclif_get_post_header_content_type(struct api_session_data *sd, const char *name, char **content_type) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_aclif_get_post_header_content_type_pre > 0) {
		bool (*preHookFunc) (struct api_session_data **sd, const char **name, char ***content_type);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_get_post_header_content_type_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_get_post_header_content_type_pre[hIndex].func;
			retVal___ = preHookFunc(&sd, &name, &content_type);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aclif.get_post_header_content_type(sd, name, content_type);
	}
	if (HPMHooks.count.HP_aclif_get_post_header_content_type_post > 0) {
		bool (*postHookFunc) (bool retVal___, struct api_session_data *sd, const char *name, char **content_type);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_get_post_header_content_type_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_get_post_header_content_type_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, sd, name, content_type);
		}
	}
	return retVal___;
}
int HP_aclif_get_post_headers_count(struct api_session_data *sd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_aclif_get_post_headers_count_pre > 0) {
		int (*preHookFunc) (struct api_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_get_post_headers_count_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_get_post_headers_count_pre[hIndex].func;
			retVal___ = preHookFunc(&sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aclif.get_post_headers_count(sd);
	}
	if (HPMHooks.count.HP_aclif_get_post_headers_count_post > 0) {
		int (*postHookFunc) (int retVal___, struct api_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_get_post_headers_count_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_get_post_headers_count_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, sd);
		}
	}
	return retVal___;
}
void HP_aclif_delete_online_player(int account_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_delete_online_player_pre > 0) {
		void (*preHookFunc) (int *account_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_delete_online_player_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_delete_online_player_pre[hIndex].func;
			preHookFunc(&account_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.delete_online_player(account_id);
	}
	if (HPMHooks.count.HP_aclif_delete_online_player_post > 0) {
		void (*postHookFunc) (int account_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_delete_online_player_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_delete_online_player_post[hIndex].func;
			postHookFunc(account_id);
		}
	}
	return;
}
void HP_aclif_real_delete_online_player(int account_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_real_delete_online_player_pre > 0) {
		void (*preHookFunc) (int *account_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_real_delete_online_player_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_real_delete_online_player_pre[hIndex].func;
			preHookFunc(&account_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.real_delete_online_player(account_id);
	}
	if (HPMHooks.count.HP_aclif_real_delete_online_player_post > 0) {
		void (*postHookFunc) (int account_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_real_delete_online_player_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_real_delete_online_player_post[hIndex].func;
			postHookFunc(account_id);
		}
	}
	return;
}
void HP_aclif_add_online_player(int account_id, const unsigned char *auth_token) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_add_online_player_pre > 0) {
		void (*preHookFunc) (int *account_id, const unsigned char **auth_token);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_add_online_player_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_add_online_player_pre[hIndex].func;
			preHookFunc(&account_id, &auth_token);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.add_online_player(account_id, auth_token);
	}
	if (HPMHooks.count.HP_aclif_add_online_player_post > 0) {
		void (*postHookFunc) (int account_id, const unsigned char *auth_token);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_add_online_player_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_add_online_player_post[hIndex].func;
			postHookFunc(account_id, auth_token);
		}
	}
	return;
}
void HP_aclif_add_online_char(int account_id, int char_id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_add_online_char_pre > 0) {
		void (*preHookFunc) (int *account_id, int *char_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_add_online_char_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_add_online_char_pre[hIndex].func;
			preHookFunc(&account_id, &char_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.add_online_char(account_id, char_id);
	}
	if (HPMHooks.count.HP_aclif_add_online_char_post > 0) {
		void (*postHookFunc) (int account_id, int char_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_add_online_char_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_add_online_char_post[hIndex].func;
			postHookFunc(account_id, char_id);
		}
	}
	return;
}
struct DBData HP_aclif_create_online_login_data(union DBKey key, va_list args) {
	int hIndex = 0;
	struct DBData retVal___ = { 0 };
	if (HPMHooks.count.HP_aclif_create_online_login_data_pre > 0) {
		struct DBData (*preHookFunc) (union DBKey *key, va_list args);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_create_online_login_data_pre; hIndex++) {
			va_list args___copy; va_copy(args___copy, args);
			preHookFunc = HPMHooks.list.HP_aclif_create_online_login_data_pre[hIndex].func;
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
		retVal___ = HPMHooks.source.aclif.create_online_login_data(key, args___copy);
		va_end(args___copy);
	}
	if (HPMHooks.count.HP_aclif_create_online_login_data_post > 0) {
		struct DBData (*postHookFunc) (struct DBData retVal___, union DBKey key, va_list args);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_create_online_login_data_post; hIndex++) {
			va_list args___copy; va_copy(args___copy, args);
			postHookFunc = HPMHooks.list.HP_aclif_create_online_login_data_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, key, args___copy);
			va_end(args___copy);
		}
	}
	return retVal___;
}
void HP_aclif_add_char_server(int char_server_id, const char *name) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_add_char_server_pre > 0) {
		void (*preHookFunc) (int *char_server_id, const char **name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_add_char_server_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_add_char_server_pre[hIndex].func;
			preHookFunc(&char_server_id, &name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.add_char_server(char_server_id, name);
	}
	if (HPMHooks.count.HP_aclif_add_char_server_post > 0) {
		void (*postHookFunc) (int char_server_id, const char *name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_add_char_server_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_add_char_server_post[hIndex].func;
			postHookFunc(char_server_id, name);
		}
	}
	return;
}
void HP_aclif_remove_char_server(int char_server_id, const char *name) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_remove_char_server_pre > 0) {
		void (*preHookFunc) (int *char_server_id, const char **name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_remove_char_server_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_remove_char_server_pre[hIndex].func;
			preHookFunc(&char_server_id, &name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.remove_char_server(char_server_id, name);
	}
	if (HPMHooks.count.HP_aclif_remove_char_server_post > 0) {
		void (*postHookFunc) (int char_server_id, const char *name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_remove_char_server_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_remove_char_server_post[hIndex].func;
			postHookFunc(char_server_id, name);
		}
	}
	return;
}
int HP_aclif_purge_disconnected_users(int tid, int64 tick, int id, intptr_t data) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_aclif_purge_disconnected_users_pre > 0) {
		int (*preHookFunc) (int *tid, int64 *tick, int *id, intptr_t *data);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_purge_disconnected_users_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_purge_disconnected_users_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &tick, &id, &data);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aclif.purge_disconnected_users(tid, tick, id, data);
	}
	if (HPMHooks.count.HP_aclif_purge_disconnected_users_post > 0) {
		int (*postHookFunc) (int retVal___, int tid, int64 tick, int id, intptr_t data);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_purge_disconnected_users_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_purge_disconnected_users_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, tid, tick, id, data);
		}
	}
	return retVal___;
}
int HP_aclif_purge_disconnected_user(union DBKey key, struct DBData *data, va_list ap) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_aclif_purge_disconnected_user_pre > 0) {
		int (*preHookFunc) (union DBKey *key, struct DBData **data, va_list ap);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_purge_disconnected_user_pre; hIndex++) {
			va_list ap___copy; va_copy(ap___copy, ap);
			preHookFunc = HPMHooks.list.HP_aclif_purge_disconnected_user_pre[hIndex].func;
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
		retVal___ = HPMHooks.source.aclif.purge_disconnected_user(key, data, ap___copy);
		va_end(ap___copy);
	}
	if (HPMHooks.count.HP_aclif_purge_disconnected_user_post > 0) {
		int (*postHookFunc) (int retVal___, union DBKey key, struct DBData *data, va_list ap);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_purge_disconnected_user_post; hIndex++) {
			va_list ap___copy; va_copy(ap___copy, ap);
			postHookFunc = HPMHooks.list.HP_aclif_purge_disconnected_user_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, key, data, ap___copy);
			va_end(ap___copy);
		}
	}
	return retVal___;
}
int HP_aclif_get_char_server_id(struct api_session_data *sd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_aclif_get_char_server_id_pre > 0) {
		int (*preHookFunc) (struct api_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_get_char_server_id_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_get_char_server_id_pre[hIndex].func;
			retVal___ = preHookFunc(&sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aclif.get_char_server_id(sd);
	}
	if (HPMHooks.count.HP_aclif_get_char_server_id_post > 0) {
		int (*postHookFunc) (int retVal___, struct api_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_get_char_server_id_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_get_char_server_id_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, sd);
		}
	}
	return retVal___;
}
void HP_aclif_add_remove_timer(struct online_api_login_data *data) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_add_remove_timer_pre > 0) {
		void (*preHookFunc) (struct online_api_login_data **data);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_add_remove_timer_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_add_remove_timer_pre[hIndex].func;
			preHookFunc(&data);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.add_remove_timer(data);
	}
	if (HPMHooks.count.HP_aclif_add_remove_timer_post > 0) {
		void (*postHookFunc) (struct online_api_login_data *data);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_add_remove_timer_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_add_remove_timer_post[hIndex].func;
			postHookFunc(data);
		}
	}
	return;
}
void HP_aclif_remove_remove_timer(struct online_api_login_data *data) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_remove_remove_timer_pre > 0) {
		void (*preHookFunc) (struct online_api_login_data **data);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_remove_remove_timer_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_remove_remove_timer_pre[hIndex].func;
			preHookFunc(&data);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.remove_remove_timer(data);
	}
	if (HPMHooks.count.HP_aclif_remove_remove_timer_post > 0) {
		void (*postHookFunc) (struct online_api_login_data *data);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_remove_remove_timer_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_remove_remove_timer_post[hIndex].func;
			postHookFunc(data);
		}
	}
	return;
}
void HP_aclif_show_request(int fd, struct api_session_data *sd, bool show_http_headers) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aclif_show_request_pre > 0) {
		void (*preHookFunc) (int *fd, struct api_session_data **sd, bool *show_http_headers);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_show_request_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aclif_show_request_pre[hIndex].func;
			preHookFunc(&fd, &sd, &show_http_headers);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aclif.show_request(fd, sd, show_http_headers);
	}
	if (HPMHooks.count.HP_aclif_show_request_post > 0) {
		void (*postHookFunc) (int fd, struct api_session_data *sd, bool show_http_headers);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aclif_show_request_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aclif_show_request_post[hIndex].func;
			postHookFunc(fd, sd, show_http_headers);
		}
	}
	return;
}
/* aloginif_interface */
void HP_aloginif_init(bool minimal) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aloginif_init_pre > 0) {
		void (*preHookFunc) (bool *minimal);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aloginif_init_pre[hIndex].func;
			preHookFunc(&minimal);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aloginif.init(minimal);
	}
	if (HPMHooks.count.HP_aloginif_init_post > 0) {
		void (*postHookFunc) (bool minimal);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aloginif_init_post[hIndex].func;
			postHookFunc(minimal);
		}
	}
	return;
}
void HP_aloginif_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aloginif_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aloginif_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aloginif.final();
	}
	if (HPMHooks.count.HP_aloginif_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aloginif_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_aloginif_setuserid(char *id) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aloginif_setuserid_pre > 0) {
		void (*preHookFunc) (char **id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_setuserid_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aloginif_setuserid_pre[hIndex].func;
			preHookFunc(&id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aloginif.setuserid(id);
	}
	if (HPMHooks.count.HP_aloginif_setuserid_post > 0) {
		void (*postHookFunc) (char *id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_setuserid_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aloginif_setuserid_post[hIndex].func;
			postHookFunc(id);
		}
	}
	return;
}
void HP_aloginif_setpasswd(char *pwd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aloginif_setpasswd_pre > 0) {
		void (*preHookFunc) (char **pwd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_setpasswd_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aloginif_setpasswd_pre[hIndex].func;
			preHookFunc(&pwd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aloginif.setpasswd(pwd);
	}
	if (HPMHooks.count.HP_aloginif_setpasswd_post > 0) {
		void (*postHookFunc) (char *pwd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_setpasswd_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aloginif_setpasswd_post[hIndex].func;
			postHookFunc(pwd);
		}
	}
	return;
}
void HP_aloginif_checkdefaultlogin(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aloginif_checkdefaultlogin_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_checkdefaultlogin_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aloginif_checkdefaultlogin_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aloginif.checkdefaultlogin();
	}
	if (HPMHooks.count.HP_aloginif_checkdefaultlogin_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_checkdefaultlogin_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aloginif_checkdefaultlogin_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
bool HP_aloginif_setip(const char *ip) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_aloginif_setip_pre > 0) {
		bool (*preHookFunc) (const char **ip);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_setip_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aloginif_setip_pre[hIndex].func;
			retVal___ = preHookFunc(&ip);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aloginif.setip(ip);
	}
	if (HPMHooks.count.HP_aloginif_setip_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *ip);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_setip_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aloginif_setip_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, ip);
		}
	}
	return retVal___;
}
void HP_aloginif_setport(uint16 port) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aloginif_setport_pre > 0) {
		void (*preHookFunc) (uint16 *port);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_setport_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aloginif_setport_pre[hIndex].func;
			preHookFunc(&port);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aloginif.setport(port);
	}
	if (HPMHooks.count.HP_aloginif_setport_post > 0) {
		void (*postHookFunc) (uint16 port);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_setport_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aloginif_setport_post[hIndex].func;
			postHookFunc(port);
		}
	}
	return;
}
void HP_aloginif_connect_to_server(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aloginif_connect_to_server_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_connect_to_server_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aloginif_connect_to_server_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aloginif.connect_to_server();
	}
	if (HPMHooks.count.HP_aloginif_connect_to_server_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_connect_to_server_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aloginif_connect_to_server_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_aloginif_on_disconnect(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aloginif_on_disconnect_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_on_disconnect_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aloginif_on_disconnect_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aloginif.on_disconnect();
	}
	if (HPMHooks.count.HP_aloginif_on_disconnect_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_on_disconnect_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aloginif_on_disconnect_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_aloginif_keepalive(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aloginif_keepalive_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_keepalive_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aloginif_keepalive_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aloginif.keepalive(fd);
	}
	if (HPMHooks.count.HP_aloginif_keepalive_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_keepalive_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aloginif_keepalive_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_aloginif_on_ready(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aloginif_on_ready_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_on_ready_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aloginif_on_ready_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aloginif.on_ready();
	}
	if (HPMHooks.count.HP_aloginif_on_ready_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_on_ready_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aloginif_on_ready_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_aloginif_send_to_char(int fd, struct api_session_data *sd, int msg_id, void *data, size_t data_len) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aloginif_send_to_char_pre > 0) {
		void (*preHookFunc) (int *fd, struct api_session_data **sd, int *msg_id, void **data, size_t *data_len);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_send_to_char_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aloginif_send_to_char_pre[hIndex].func;
			preHookFunc(&fd, &sd, &msg_id, &data, &data_len);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aloginif.send_to_char(fd, sd, msg_id, data, data_len);
	}
	if (HPMHooks.count.HP_aloginif_send_to_char_post > 0) {
		void (*postHookFunc) (int fd, struct api_session_data *sd, int msg_id, void *data, size_t data_len);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_send_to_char_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aloginif_send_to_char_post[hIndex].func;
			postHookFunc(fd, sd, msg_id, data, data_len);
		}
	}
	return;
}
void HP_aloginif_send_split_to_char(int fd, struct api_session_data *sd, int msg_id, char *data, size_t data_len) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aloginif_send_split_to_char_pre > 0) {
		void (*preHookFunc) (int *fd, struct api_session_data **sd, int *msg_id, char **data, size_t *data_len);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_send_split_to_char_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aloginif_send_split_to_char_pre[hIndex].func;
			preHookFunc(&fd, &sd, &msg_id, &data, &data_len);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aloginif.send_split_to_char(fd, sd, msg_id, data, data_len);
	}
	if (HPMHooks.count.HP_aloginif_send_split_to_char_post > 0) {
		void (*postHookFunc) (int fd, struct api_session_data *sd, int msg_id, char *data, size_t data_len);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_send_split_to_char_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aloginif_send_split_to_char_post[hIndex].func;
			postHookFunc(fd, sd, msg_id, data, data_len);
		}
	}
	return;
}
int HP_aloginif_parse(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_aloginif_parse_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_parse_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aloginif_parse_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aloginif.parse(fd);
	}
	if (HPMHooks.count.HP_aloginif_parse_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_parse_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aloginif_parse_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_aloginif_parse_connection_state(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_aloginif_parse_connection_state_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_parse_connection_state_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aloginif_parse_connection_state_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aloginif.parse_connection_state(fd);
	}
	if (HPMHooks.count.HP_aloginif_parse_connection_state_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_parse_connection_state_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aloginif_parse_connection_state_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_aloginif_parse_pong(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_aloginif_parse_pong_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_parse_pong_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aloginif_parse_pong_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aloginif.parse_pong(fd);
	}
	if (HPMHooks.count.HP_aloginif_parse_pong_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_parse_pong_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aloginif_parse_pong_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_aloginif_parse_disconnect_user(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_aloginif_parse_disconnect_user_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_parse_disconnect_user_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aloginif_parse_disconnect_user_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aloginif.parse_disconnect_user(fd);
	}
	if (HPMHooks.count.HP_aloginif_parse_disconnect_user_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_parse_disconnect_user_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aloginif_parse_disconnect_user_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_aloginif_parse_connect_user(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_aloginif_parse_connect_user_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_parse_connect_user_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aloginif_parse_connect_user_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aloginif.parse_connect_user(fd);
	}
	if (HPMHooks.count.HP_aloginif_parse_connect_user_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_parse_connect_user_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aloginif_parse_connect_user_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_aloginif_parse_char_servers_list(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_aloginif_parse_char_servers_list_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_parse_char_servers_list_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aloginif_parse_char_servers_list_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aloginif.parse_char_servers_list(fd);
	}
	if (HPMHooks.count.HP_aloginif_parse_char_servers_list_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_parse_char_servers_list_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aloginif_parse_char_servers_list_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_aloginif_parse_add_char_server(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_aloginif_parse_add_char_server_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_parse_add_char_server_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aloginif_parse_add_char_server_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aloginif.parse_add_char_server(fd);
	}
	if (HPMHooks.count.HP_aloginif_parse_add_char_server_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_parse_add_char_server_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aloginif_parse_add_char_server_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_aloginif_parse_remove_char_server(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_aloginif_parse_remove_char_server_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_parse_remove_char_server_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aloginif_parse_remove_char_server_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aloginif.parse_remove_char_server(fd);
	}
	if (HPMHooks.count.HP_aloginif_parse_remove_char_server_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_parse_remove_char_server_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aloginif_parse_remove_char_server_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_aloginif_parse_proxy_from_char_server(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_aloginif_parse_proxy_from_char_server_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_parse_proxy_from_char_server_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aloginif_parse_proxy_from_char_server_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aloginif.parse_proxy_from_char_server(fd);
	}
	if (HPMHooks.count.HP_aloginif_parse_proxy_from_char_server_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_parse_proxy_from_char_server_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aloginif_parse_proxy_from_char_server_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
int HP_aloginif_parse_set_char_online(int fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_aloginif_parse_set_char_online_pre > 0) {
		int (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_parse_set_char_online_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aloginif_parse_set_char_online_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.aloginif.parse_set_char_online(fd);
	}
	if (HPMHooks.count.HP_aloginif_parse_set_char_online_post > 0) {
		int (*postHookFunc) (int retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_parse_set_char_online_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aloginif_parse_set_char_online_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
void HP_aloginif_parse_from_char(int fd, Handler_func func) {
	int hIndex = 0;
	if (HPMHooks.count.HP_aloginif_parse_from_char_pre > 0) {
		void (*preHookFunc) (int *fd, Handler_func *func);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_parse_from_char_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_aloginif_parse_from_char_pre[hIndex].func;
			preHookFunc(&fd, &func);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.aloginif.parse_from_char(fd, func);
	}
	if (HPMHooks.count.HP_aloginif_parse_from_char_post > 0) {
		void (*postHookFunc) (int fd, Handler_func func);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_aloginif_parse_from_char_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_aloginif_parse_from_char_post[hIndex].func;
			postHookFunc(fd, func);
		}
	}
	return;
}
/* api_interface */
int HP_api_setipport(unsigned short map_index, uint32 ip, uint16 port) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_api_setipport_pre > 0) {
		int (*preHookFunc) (unsigned short *map_index, uint32 *ip, uint16 *port);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_api_setipport_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_api_setipport_pre[hIndex].func;
			retVal___ = preHookFunc(&map_index, &ip, &port);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.api.setipport(map_index, ip, port);
	}
	if (HPMHooks.count.HP_api_setipport_post > 0) {
		int (*postHookFunc) (int retVal___, unsigned short map_index, uint32 ip, uint16 port);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_api_setipport_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_api_setipport_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, map_index, ip, port);
		}
	}
	return retVal___;
}
bool HP_api_config_read(const char *filename, bool imported) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_api_config_read_pre > 0) {
		bool (*preHookFunc) (const char **filename, bool *imported);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_api_config_read_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_api_config_read_pre[hIndex].func;
			retVal___ = preHookFunc(&filename, &imported);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.api.config_read(filename, imported);
	}
	if (HPMHooks.count.HP_api_config_read_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *filename, bool imported);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_api_config_read_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_api_config_read_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, filename, imported);
		}
	}
	return retVal___;
}
bool HP_api_config_read_console(const char *filename, struct config_t *config, bool imported) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_api_config_read_console_pre > 0) {
		bool (*preHookFunc) (const char **filename, struct config_t **config, bool *imported);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_api_config_read_console_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_api_config_read_console_pre[hIndex].func;
			retVal___ = preHookFunc(&filename, &config, &imported);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.api.config_read_console(filename, config, imported);
	}
	if (HPMHooks.count.HP_api_config_read_console_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *filename, struct config_t *config, bool imported);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_api_config_read_console_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_api_config_read_console_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, filename, config, imported);
		}
	}
	return retVal___;
}
bool HP_api_config_read_connection(const char *filename, struct config_t *config, bool imported) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_api_config_read_connection_pre > 0) {
		bool (*preHookFunc) (const char **filename, struct config_t **config, bool *imported);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_api_config_read_connection_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_api_config_read_connection_pre[hIndex].func;
			retVal___ = preHookFunc(&filename, &config, &imported);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.api.config_read_connection(filename, config, imported);
	}
	if (HPMHooks.count.HP_api_config_read_connection_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *filename, struct config_t *config, bool imported);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_api_config_read_connection_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_api_config_read_connection_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, filename, config, imported);
		}
	}
	return retVal___;
}
bool HP_api_config_read_inter(const char *filename, struct config_t *config, bool imported) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_api_config_read_inter_pre > 0) {
		bool (*preHookFunc) (const char **filename, struct config_t **config, bool *imported);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_api_config_read_inter_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_api_config_read_inter_pre[hIndex].func;
			retVal___ = preHookFunc(&filename, &config, &imported);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.api.config_read_inter(filename, config, imported);
	}
	if (HPMHooks.count.HP_api_config_read_inter_post > 0) {
		bool (*postHookFunc) (bool retVal___, const char *filename, struct config_t *config, bool imported);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_api_config_read_inter_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_api_config_read_inter_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, filename, config, imported);
		}
	}
	return retVal___;
}
int HP_api_check_connect_login_server(int tid, int64 tick, int id, intptr_t data) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_api_check_connect_login_server_pre > 0) {
		int (*preHookFunc) (int *tid, int64 *tick, int *id, intptr_t *data);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_api_check_connect_login_server_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_api_check_connect_login_server_pre[hIndex].func;
			retVal___ = preHookFunc(&tid, &tick, &id, &data);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.api.check_connect_login_server(tid, tick, id, data);
	}
	if (HPMHooks.count.HP_api_check_connect_login_server_post > 0) {
		int (*postHookFunc) (int retVal___, int tid, int64 tick, int id, intptr_t data);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_api_check_connect_login_server_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_api_check_connect_login_server_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, tid, tick, id, data);
		}
	}
	return retVal___;
}
void HP_api_do_shutdown(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_api_do_shutdown_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_api_do_shutdown_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_api_do_shutdown_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.api.do_shutdown();
	}
	if (HPMHooks.count.HP_api_do_shutdown_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_api_do_shutdown_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_api_do_shutdown_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
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
/* handlers_interface */
int HP_handlers_init(bool minimal) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_handlers_init_pre > 0) {
		int (*preHookFunc) (bool *minimal);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_handlers_init_pre[hIndex].func;
			retVal___ = preHookFunc(&minimal);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.handlers.init(minimal);
	}
	if (HPMHooks.count.HP_handlers_init_post > 0) {
		int (*postHookFunc) (int retVal___, bool minimal);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_handlers_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, minimal);
		}
	}
	return retVal___;
}
void HP_handlers_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_handlers_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_handlers_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.handlers.final();
	}
	if (HPMHooks.count.HP_handlers_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_handlers_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_handlers_sendHotkeyV2Tab(JsonP *json, struct userconfig_userhotkeys_v2 *hotkeys) {
	int hIndex = 0;
	if (HPMHooks.count.HP_handlers_sendHotkeyV2Tab_pre > 0) {
		void (*preHookFunc) (JsonP **json, struct userconfig_userhotkeys_v2 **hotkeys);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_sendHotkeyV2Tab_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_handlers_sendHotkeyV2Tab_pre[hIndex].func;
			preHookFunc(&json, &hotkeys);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.handlers.sendHotkeyV2Tab(json, hotkeys);
	}
	if (HPMHooks.count.HP_handlers_sendHotkeyV2Tab_post > 0) {
		void (*postHookFunc) (JsonP *json, struct userconfig_userhotkeys_v2 *hotkeys);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_sendHotkeyV2Tab_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_handlers_sendHotkeyV2Tab_post[hIndex].func;
			postHookFunc(json, hotkeys);
		}
	}
	return;
}
const char* HP_handlers_hotkeyTabIdToName(int tab_id) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if (HPMHooks.count.HP_handlers_hotkeyTabIdToName_pre > 0) {
		const char* (*preHookFunc) (int *tab_id);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_hotkeyTabIdToName_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_handlers_hotkeyTabIdToName_pre[hIndex].func;
			retVal___ = preHookFunc(&tab_id);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.handlers.hotkeyTabIdToName(tab_id);
	}
	if (HPMHooks.count.HP_handlers_hotkeyTabIdToName_post > 0) {
		const char* (*postHookFunc) (const char* retVal___, int tab_id);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_hotkeyTabIdToName_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_handlers_hotkeyTabIdToName_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, tab_id);
		}
	}
	return retVal___;
}
bool HP_handlers_parse_userconfig_load(int fd, struct api_session_data *sd) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_handlers_parse_userconfig_load_pre > 0) {
		bool (*preHookFunc) (int *fd, struct api_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_parse_userconfig_load_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_handlers_parse_userconfig_load_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.handlers.parse_userconfig_load(fd, sd);
	}
	if (HPMHooks.count.HP_handlers_parse_userconfig_load_post > 0) {
		bool (*postHookFunc) (bool retVal___, int fd, struct api_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_parse_userconfig_load_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_handlers_parse_userconfig_load_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, sd);
		}
	}
	return retVal___;
}
void HP_handlers_userconfig_load(int fd, struct api_session_data *sd, const void *data, size_t data_size) {
	int hIndex = 0;
	if (HPMHooks.count.HP_handlers_userconfig_load_pre > 0) {
		void (*preHookFunc) (int *fd, struct api_session_data **sd, const void **data, size_t *data_size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_userconfig_load_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_handlers_userconfig_load_pre[hIndex].func;
			preHookFunc(&fd, &sd, &data, &data_size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.handlers.userconfig_load(fd, sd, data, data_size);
	}
	if (HPMHooks.count.HP_handlers_userconfig_load_post > 0) {
		void (*postHookFunc) (int fd, struct api_session_data *sd, const void *data, size_t data_size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_userconfig_load_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_handlers_userconfig_load_post[hIndex].func;
			postHookFunc(fd, sd, data, data_size);
		}
	}
	return;
}
bool HP_handlers_parse_userconfig_save(int fd, struct api_session_data *sd) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_handlers_parse_userconfig_save_pre > 0) {
		bool (*preHookFunc) (int *fd, struct api_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_parse_userconfig_save_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_handlers_parse_userconfig_save_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.handlers.parse_userconfig_save(fd, sd);
	}
	if (HPMHooks.count.HP_handlers_parse_userconfig_save_post > 0) {
		bool (*postHookFunc) (bool retVal___, int fd, struct api_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_parse_userconfig_save_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_handlers_parse_userconfig_save_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, sd);
		}
	}
	return retVal___;
}
void HP_handlers_userconfig_save(int fd, struct api_session_data *sd, const void *data, size_t data_size) {
	int hIndex = 0;
	if (HPMHooks.count.HP_handlers_userconfig_save_pre > 0) {
		void (*preHookFunc) (int *fd, struct api_session_data **sd, const void **data, size_t *data_size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_userconfig_save_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_handlers_userconfig_save_pre[hIndex].func;
			preHookFunc(&fd, &sd, &data, &data_size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.handlers.userconfig_save(fd, sd, data, data_size);
	}
	if (HPMHooks.count.HP_handlers_userconfig_save_post > 0) {
		void (*postHookFunc) (int fd, struct api_session_data *sd, const void *data, size_t data_size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_userconfig_save_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_handlers_userconfig_save_post[hIndex].func;
			postHookFunc(fd, sd, data, data_size);
		}
	}
	return;
}
bool HP_handlers_parse_charconfig_load(int fd, struct api_session_data *sd) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_handlers_parse_charconfig_load_pre > 0) {
		bool (*preHookFunc) (int *fd, struct api_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_parse_charconfig_load_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_handlers_parse_charconfig_load_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.handlers.parse_charconfig_load(fd, sd);
	}
	if (HPMHooks.count.HP_handlers_parse_charconfig_load_post > 0) {
		bool (*postHookFunc) (bool retVal___, int fd, struct api_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_parse_charconfig_load_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_handlers_parse_charconfig_load_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, sd);
		}
	}
	return retVal___;
}
void HP_handlers_charconfig_load(int fd, struct api_session_data *sd, const void *data, size_t data_size) {
	int hIndex = 0;
	if (HPMHooks.count.HP_handlers_charconfig_load_pre > 0) {
		void (*preHookFunc) (int *fd, struct api_session_data **sd, const void **data, size_t *data_size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_charconfig_load_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_handlers_charconfig_load_pre[hIndex].func;
			preHookFunc(&fd, &sd, &data, &data_size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.handlers.charconfig_load(fd, sd, data, data_size);
	}
	if (HPMHooks.count.HP_handlers_charconfig_load_post > 0) {
		void (*postHookFunc) (int fd, struct api_session_data *sd, const void *data, size_t data_size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_charconfig_load_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_handlers_charconfig_load_post[hIndex].func;
			postHookFunc(fd, sd, data, data_size);
		}
	}
	return;
}
bool HP_handlers_parse_emblem_upload(int fd, struct api_session_data *sd) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_handlers_parse_emblem_upload_pre > 0) {
		bool (*preHookFunc) (int *fd, struct api_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_parse_emblem_upload_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_handlers_parse_emblem_upload_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.handlers.parse_emblem_upload(fd, sd);
	}
	if (HPMHooks.count.HP_handlers_parse_emblem_upload_post > 0) {
		bool (*postHookFunc) (bool retVal___, int fd, struct api_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_parse_emblem_upload_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_handlers_parse_emblem_upload_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, sd);
		}
	}
	return retVal___;
}
void HP_handlers_emblem_upload(int fd, struct api_session_data *sd, const void *data, size_t data_size) {
	int hIndex = 0;
	if (HPMHooks.count.HP_handlers_emblem_upload_pre > 0) {
		void (*preHookFunc) (int *fd, struct api_session_data **sd, const void **data, size_t *data_size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_emblem_upload_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_handlers_emblem_upload_pre[hIndex].func;
			preHookFunc(&fd, &sd, &data, &data_size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.handlers.emblem_upload(fd, sd, data, data_size);
	}
	if (HPMHooks.count.HP_handlers_emblem_upload_post > 0) {
		void (*postHookFunc) (int fd, struct api_session_data *sd, const void *data, size_t data_size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_emblem_upload_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_handlers_emblem_upload_post[hIndex].func;
			postHookFunc(fd, sd, data, data_size);
		}
	}
	return;
}
bool HP_handlers_parse_emblem_download(int fd, struct api_session_data *sd) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_handlers_parse_emblem_download_pre > 0) {
		bool (*preHookFunc) (int *fd, struct api_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_parse_emblem_download_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_handlers_parse_emblem_download_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.handlers.parse_emblem_download(fd, sd);
	}
	if (HPMHooks.count.HP_handlers_parse_emblem_download_post > 0) {
		bool (*postHookFunc) (bool retVal___, int fd, struct api_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_parse_emblem_download_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_handlers_parse_emblem_download_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, sd);
		}
	}
	return retVal___;
}
void HP_handlers_emblem_download(int fd, struct api_session_data *sd, const void *data, size_t data_size) {
	int hIndex = 0;
	if (HPMHooks.count.HP_handlers_emblem_download_pre > 0) {
		void (*preHookFunc) (int *fd, struct api_session_data **sd, const void **data, size_t *data_size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_emblem_download_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_handlers_emblem_download_pre[hIndex].func;
			preHookFunc(&fd, &sd, &data, &data_size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.handlers.emblem_download(fd, sd, data, data_size);
	}
	if (HPMHooks.count.HP_handlers_emblem_download_post > 0) {
		void (*postHookFunc) (int fd, struct api_session_data *sd, const void *data, size_t data_size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_emblem_download_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_handlers_emblem_download_post[hIndex].func;
			postHookFunc(fd, sd, data, data_size);
		}
	}
	return;
}
bool HP_handlers_parse_test_url(int fd, struct api_session_data *sd) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_handlers_parse_test_url_pre > 0) {
		bool (*preHookFunc) (int *fd, struct api_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_parse_test_url_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_handlers_parse_test_url_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.handlers.parse_test_url(fd, sd);
	}
	if (HPMHooks.count.HP_handlers_parse_test_url_post > 0) {
		bool (*postHookFunc) (bool retVal___, int fd, struct api_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_parse_test_url_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_handlers_parse_test_url_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, sd);
		}
	}
	return retVal___;
}
void HP_handlers_userconfig_load_emotes(int fd, struct api_session_data *sd, const void *data, size_t data_size) {
	int hIndex = 0;
	if (HPMHooks.count.HP_handlers_userconfig_load_emotes_pre > 0) {
		void (*preHookFunc) (int *fd, struct api_session_data **sd, const void **data, size_t *data_size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_userconfig_load_emotes_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_handlers_userconfig_load_emotes_pre[hIndex].func;
			preHookFunc(&fd, &sd, &data, &data_size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.handlers.userconfig_load_emotes(fd, sd, data, data_size);
	}
	if (HPMHooks.count.HP_handlers_userconfig_load_emotes_post > 0) {
		void (*postHookFunc) (int fd, struct api_session_data *sd, const void *data, size_t data_size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_userconfig_load_emotes_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_handlers_userconfig_load_emotes_post[hIndex].func;
			postHookFunc(fd, sd, data, data_size);
		}
	}
	return;
}
void HP_handlers_userconfig_load_hotkeys(int fd, struct api_session_data *sd, const void *data, size_t data_size) {
	int hIndex = 0;
	if (HPMHooks.count.HP_handlers_userconfig_load_hotkeys_pre > 0) {
		void (*preHookFunc) (int *fd, struct api_session_data **sd, const void **data, size_t *data_size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_userconfig_load_hotkeys_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_handlers_userconfig_load_hotkeys_pre[hIndex].func;
			preHookFunc(&fd, &sd, &data, &data_size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.handlers.userconfig_load_hotkeys(fd, sd, data, data_size);
	}
	if (HPMHooks.count.HP_handlers_userconfig_load_hotkeys_post > 0) {
		void (*postHookFunc) (int fd, struct api_session_data *sd, const void *data, size_t data_size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_handlers_userconfig_load_hotkeys_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_handlers_userconfig_load_hotkeys_post[hIndex].func;
			postHookFunc(fd, sd, data, data_size);
		}
	}
	return;
}
/* httpparser_interface */
int HP_httpparser_init(bool minimal) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_httpparser_init_pre > 0) {
		int (*preHookFunc) (bool *minimal);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_init_pre[hIndex].func;
			retVal___ = preHookFunc(&minimal);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.init(minimal);
	}
	if (HPMHooks.count.HP_httpparser_init_post > 0) {
		int (*postHookFunc) (int retVal___, bool minimal);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, minimal);
		}
	}
	return retVal___;
}
void HP_httpparser_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_httpparser_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.httpparser.final();
	}
	if (HPMHooks.count.HP_httpparser_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
bool HP_httpparser_parse(int fd) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_httpparser_parse_pre > 0) {
		bool (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_parse_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_parse_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.parse(fd);
	}
	if (HPMHooks.count.HP_httpparser_parse_post > 0) {
		bool (*postHookFunc) (bool retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_parse_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_parse_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
bool HP_httpparser_parse_real(int fd, struct api_session_data *sd, const char *data, int data_size) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_httpparser_parse_real_pre > 0) {
		bool (*preHookFunc) (int *fd, struct api_session_data **sd, const char **data, int *data_size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_parse_real_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_parse_real_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &sd, &data, &data_size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.parse_real(fd, sd, data, data_size);
	}
	if (HPMHooks.count.HP_httpparser_parse_real_post > 0) {
		bool (*postHookFunc) (bool retVal___, int fd, struct api_session_data *sd, const char *data, int data_size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_parse_real_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_parse_real_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, sd, data, data_size);
		}
	}
	return retVal___;
}
void HP_httpparser_add_to_temp_request(int fd, struct api_session_data *sd, const char *data, int data_size) {
	int hIndex = 0;
	if (HPMHooks.count.HP_httpparser_add_to_temp_request_pre > 0) {
		void (*preHookFunc) (int *fd, struct api_session_data **sd, const char **data, int *data_size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_add_to_temp_request_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_add_to_temp_request_pre[hIndex].func;
			preHookFunc(&fd, &sd, &data, &data_size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.httpparser.add_to_temp_request(fd, sd, data, data_size);
	}
	if (HPMHooks.count.HP_httpparser_add_to_temp_request_post > 0) {
		void (*postHookFunc) (int fd, struct api_session_data *sd, const char *data, int data_size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_add_to_temp_request_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_add_to_temp_request_post[hIndex].func;
			postHookFunc(fd, sd, data, data_size);
		}
	}
	return;
}
int HP_httpparser_search_request_line_end(struct api_session_data *sd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_httpparser_search_request_line_end_pre > 0) {
		int (*preHookFunc) (struct api_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_search_request_line_end_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_search_request_line_end_pre[hIndex].func;
			retVal___ = preHookFunc(&sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.search_request_line_end(sd);
	}
	if (HPMHooks.count.HP_httpparser_search_request_line_end_post > 0) {
		int (*postHookFunc) (int retVal___, struct api_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_search_request_line_end_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_search_request_line_end_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, sd);
		}
	}
	return retVal___;
}
bool HP_httpparser_multi_parse(int fd) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_httpparser_multi_parse_pre > 0) {
		bool (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_multi_parse_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_multi_parse_pre[hIndex].func;
			retVal___ = preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.multi_parse(fd);
	}
	if (HPMHooks.count.HP_httpparser_multi_parse_post > 0) {
		bool (*postHookFunc) (bool retVal___, int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_multi_parse_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_multi_parse_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd);
		}
	}
	return retVal___;
}
void HP_httpparser_init_parser(int fd, struct api_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_httpparser_init_parser_pre > 0) {
		void (*preHookFunc) (int *fd, struct api_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_init_parser_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_init_parser_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.httpparser.init_parser(fd, sd);
	}
	if (HPMHooks.count.HP_httpparser_init_parser_post > 0) {
		void (*postHookFunc) (int fd, struct api_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_init_parser_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_init_parser_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_httpparser_show_error(int fd, struct api_session_data *sd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_httpparser_show_error_pre > 0) {
		void (*preHookFunc) (int *fd, struct api_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_show_error_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_show_error_pre[hIndex].func;
			preHookFunc(&fd, &sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.httpparser.show_error(fd, sd);
	}
	if (HPMHooks.count.HP_httpparser_show_error_post > 0) {
		void (*postHookFunc) (int fd, struct api_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_show_error_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_show_error_post[hIndex].func;
			postHookFunc(fd, sd);
		}
	}
	return;
}
void HP_httpparser_init_multi_parser(int fd, struct api_session_data *sd, const char *boundary) {
	int hIndex = 0;
	if (HPMHooks.count.HP_httpparser_init_multi_parser_pre > 0) {
		void (*preHookFunc) (int *fd, struct api_session_data **sd, const char **boundary);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_init_multi_parser_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_init_multi_parser_pre[hIndex].func;
			preHookFunc(&fd, &sd, &boundary);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.httpparser.init_multi_parser(fd, sd, boundary);
	}
	if (HPMHooks.count.HP_httpparser_init_multi_parser_post > 0) {
		void (*postHookFunc) (int fd, struct api_session_data *sd, const char *boundary);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_init_multi_parser_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_init_multi_parser_post[hIndex].func;
			postHookFunc(fd, sd, boundary);
		}
	}
	return;
}
void HP_httpparser_delete_parser(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_httpparser_delete_parser_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_delete_parser_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_delete_parser_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.httpparser.delete_parser(fd);
	}
	if (HPMHooks.count.HP_httpparser_delete_parser_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_delete_parser_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_delete_parser_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
}
void HP_httpparser_init_settings(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_httpparser_init_settings_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_init_settings_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_init_settings_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.httpparser.init_settings();
	}
	if (HPMHooks.count.HP_httpparser_init_settings_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_init_settings_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_init_settings_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_httpparser_init_multi_settings(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_httpparser_init_multi_settings_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_init_multi_settings_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_init_multi_settings_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.httpparser.init_multi_settings();
	}
	if (HPMHooks.count.HP_httpparser_init_multi_settings_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_init_multi_settings_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_init_multi_settings_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
const char* HP_httpparser_get_method_str(struct api_session_data *sd) {
	int hIndex = 0;
	const char* retVal___ = NULL;
	if (HPMHooks.count.HP_httpparser_get_method_str_pre > 0) {
		const char* (*preHookFunc) (struct api_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_get_method_str_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_get_method_str_pre[hIndex].func;
			retVal___ = preHookFunc(&sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.get_method_str(sd);
	}
	if (HPMHooks.count.HP_httpparser_get_method_str_post > 0) {
		const char* (*postHookFunc) (const char* retVal___, struct api_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_get_method_str_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_get_method_str_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, sd);
		}
	}
	return retVal___;
}
http_method HP_httpparser_get_method(struct api_session_data *sd) {
	int hIndex = 0;
	http_method retVal___ = HTTP_GET;
	if (HPMHooks.count.HP_httpparser_get_method_pre > 0) {
		http_method (*preHookFunc) (struct api_session_data **sd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_get_method_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_get_method_pre[hIndex].func;
			retVal___ = preHookFunc(&sd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.get_method(sd);
	}
	if (HPMHooks.count.HP_httpparser_get_method_post > 0) {
		http_method (*postHookFunc) (http_method retVal___, struct api_session_data *sd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_get_method_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_get_method_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, sd);
		}
	}
	return retVal___;
}
int HP_httpparser_on_message_begin(HTTP_PARSER *parser) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_httpparser_on_message_begin_pre > 0) {
		int (*preHookFunc) (HTTP_PARSER **parser);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_message_begin_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_on_message_begin_pre[hIndex].func;
			retVal___ = preHookFunc(&parser);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.on_message_begin(parser);
	}
	if (HPMHooks.count.HP_httpparser_on_message_begin_post > 0) {
		int (*postHookFunc) (int retVal___, HTTP_PARSER *parser);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_message_begin_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_on_message_begin_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parser);
		}
	}
	return retVal___;
}
int HP_httpparser_on_headers_complete(HTTP_PARSER *parser) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_httpparser_on_headers_complete_pre > 0) {
		int (*preHookFunc) (HTTP_PARSER **parser);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_headers_complete_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_on_headers_complete_pre[hIndex].func;
			retVal___ = preHookFunc(&parser);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.on_headers_complete(parser);
	}
	if (HPMHooks.count.HP_httpparser_on_headers_complete_post > 0) {
		int (*postHookFunc) (int retVal___, HTTP_PARSER *parser);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_headers_complete_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_on_headers_complete_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parser);
		}
	}
	return retVal___;
}
int HP_httpparser_on_message_complete(HTTP_PARSER *parser) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_httpparser_on_message_complete_pre > 0) {
		int (*preHookFunc) (HTTP_PARSER **parser);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_message_complete_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_on_message_complete_pre[hIndex].func;
			retVal___ = preHookFunc(&parser);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.on_message_complete(parser);
	}
	if (HPMHooks.count.HP_httpparser_on_message_complete_post > 0) {
		int (*postHookFunc) (int retVal___, HTTP_PARSER *parser);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_message_complete_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_on_message_complete_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parser);
		}
	}
	return retVal___;
}
int HP_httpparser_on_chunk_header(HTTP_PARSER *parser) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_httpparser_on_chunk_header_pre > 0) {
		int (*preHookFunc) (HTTP_PARSER **parser);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_chunk_header_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_on_chunk_header_pre[hIndex].func;
			retVal___ = preHookFunc(&parser);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.on_chunk_header(parser);
	}
	if (HPMHooks.count.HP_httpparser_on_chunk_header_post > 0) {
		int (*postHookFunc) (int retVal___, HTTP_PARSER *parser);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_chunk_header_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_on_chunk_header_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parser);
		}
	}
	return retVal___;
}
int HP_httpparser_on_chunk_complete(HTTP_PARSER *parser) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_httpparser_on_chunk_complete_pre > 0) {
		int (*preHookFunc) (HTTP_PARSER **parser);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_chunk_complete_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_on_chunk_complete_pre[hIndex].func;
			retVal___ = preHookFunc(&parser);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.on_chunk_complete(parser);
	}
	if (HPMHooks.count.HP_httpparser_on_chunk_complete_post > 0) {
		int (*postHookFunc) (int retVal___, HTTP_PARSER *parser);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_chunk_complete_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_on_chunk_complete_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parser);
		}
	}
	return retVal___;
}
int HP_httpparser_on_url(HTTP_PARSER *parser, const char *at, size_t length) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_httpparser_on_url_pre > 0) {
		int (*preHookFunc) (HTTP_PARSER **parser, const char **at, size_t *length);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_url_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_on_url_pre[hIndex].func;
			retVal___ = preHookFunc(&parser, &at, &length);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.on_url(parser, at, length);
	}
	if (HPMHooks.count.HP_httpparser_on_url_post > 0) {
		int (*postHookFunc) (int retVal___, HTTP_PARSER *parser, const char *at, size_t length);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_url_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_on_url_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parser, at, length);
		}
	}
	return retVal___;
}
int HP_httpparser_on_status(HTTP_PARSER *parser, const char *at, size_t length) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_httpparser_on_status_pre > 0) {
		int (*preHookFunc) (HTTP_PARSER **parser, const char **at, size_t *length);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_status_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_on_status_pre[hIndex].func;
			retVal___ = preHookFunc(&parser, &at, &length);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.on_status(parser, at, length);
	}
	if (HPMHooks.count.HP_httpparser_on_status_post > 0) {
		int (*postHookFunc) (int retVal___, HTTP_PARSER *parser, const char *at, size_t length);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_status_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_on_status_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parser, at, length);
		}
	}
	return retVal___;
}
int HP_httpparser_on_header_field(HTTP_PARSER *parser, const char *at, size_t length) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_httpparser_on_header_field_pre > 0) {
		int (*preHookFunc) (HTTP_PARSER **parser, const char **at, size_t *length);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_header_field_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_on_header_field_pre[hIndex].func;
			retVal___ = preHookFunc(&parser, &at, &length);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.on_header_field(parser, at, length);
	}
	if (HPMHooks.count.HP_httpparser_on_header_field_post > 0) {
		int (*postHookFunc) (int retVal___, HTTP_PARSER *parser, const char *at, size_t length);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_header_field_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_on_header_field_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parser, at, length);
		}
	}
	return retVal___;
}
int HP_httpparser_on_header_value(HTTP_PARSER *parser, const char *at, size_t length) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_httpparser_on_header_value_pre > 0) {
		int (*preHookFunc) (HTTP_PARSER **parser, const char **at, size_t *length);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_header_value_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_on_header_value_pre[hIndex].func;
			retVal___ = preHookFunc(&parser, &at, &length);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.on_header_value(parser, at, length);
	}
	if (HPMHooks.count.HP_httpparser_on_header_value_post > 0) {
		int (*postHookFunc) (int retVal___, HTTP_PARSER *parser, const char *at, size_t length);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_header_value_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_on_header_value_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parser, at, length);
		}
	}
	return retVal___;
}
int HP_httpparser_on_body(HTTP_PARSER *parser, const char *at, size_t length) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_httpparser_on_body_pre > 0) {
		int (*preHookFunc) (HTTP_PARSER **parser, const char **at, size_t *length);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_body_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_on_body_pre[hIndex].func;
			retVal___ = preHookFunc(&parser, &at, &length);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.on_body(parser, at, length);
	}
	if (HPMHooks.count.HP_httpparser_on_body_post > 0) {
		int (*postHookFunc) (int retVal___, HTTP_PARSER *parser, const char *at, size_t length);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_body_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_on_body_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parser, at, length);
		}
	}
	return retVal___;
}
int HP_httpparser_on_multi_body_begin(struct multipartparser *parser) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_httpparser_on_multi_body_begin_pre > 0) {
		int (*preHookFunc) (struct multipartparser **parser);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_multi_body_begin_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_on_multi_body_begin_pre[hIndex].func;
			retVal___ = preHookFunc(&parser);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.on_multi_body_begin(parser);
	}
	if (HPMHooks.count.HP_httpparser_on_multi_body_begin_post > 0) {
		int (*postHookFunc) (int retVal___, struct multipartparser *parser);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_multi_body_begin_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_on_multi_body_begin_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parser);
		}
	}
	return retVal___;
}
int HP_httpparser_on_multi_part_begin(struct multipartparser *parser) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_httpparser_on_multi_part_begin_pre > 0) {
		int (*preHookFunc) (struct multipartparser **parser);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_multi_part_begin_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_on_multi_part_begin_pre[hIndex].func;
			retVal___ = preHookFunc(&parser);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.on_multi_part_begin(parser);
	}
	if (HPMHooks.count.HP_httpparser_on_multi_part_begin_post > 0) {
		int (*postHookFunc) (int retVal___, struct multipartparser *parser);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_multi_part_begin_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_on_multi_part_begin_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parser);
		}
	}
	return retVal___;
}
int HP_httpparser_on_multi_header_field(struct multipartparser *parser, const char *data, size_t size) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_httpparser_on_multi_header_field_pre > 0) {
		int (*preHookFunc) (struct multipartparser **parser, const char **data, size_t *size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_multi_header_field_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_on_multi_header_field_pre[hIndex].func;
			retVal___ = preHookFunc(&parser, &data, &size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.on_multi_header_field(parser, data, size);
	}
	if (HPMHooks.count.HP_httpparser_on_multi_header_field_post > 0) {
		int (*postHookFunc) (int retVal___, struct multipartparser *parser, const char *data, size_t size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_multi_header_field_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_on_multi_header_field_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parser, data, size);
		}
	}
	return retVal___;
}
int HP_httpparser_on_multi_header_value(struct multipartparser *parser, const char *data, size_t size) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_httpparser_on_multi_header_value_pre > 0) {
		int (*preHookFunc) (struct multipartparser **parser, const char **data, size_t *size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_multi_header_value_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_on_multi_header_value_pre[hIndex].func;
			retVal___ = preHookFunc(&parser, &data, &size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.on_multi_header_value(parser, data, size);
	}
	if (HPMHooks.count.HP_httpparser_on_multi_header_value_post > 0) {
		int (*postHookFunc) (int retVal___, struct multipartparser *parser, const char *data, size_t size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_multi_header_value_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_on_multi_header_value_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parser, data, size);
		}
	}
	return retVal___;
}
int HP_httpparser_on_multi_headers_complete(struct multipartparser *parser) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_httpparser_on_multi_headers_complete_pre > 0) {
		int (*preHookFunc) (struct multipartparser **parser);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_multi_headers_complete_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_on_multi_headers_complete_pre[hIndex].func;
			retVal___ = preHookFunc(&parser);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.on_multi_headers_complete(parser);
	}
	if (HPMHooks.count.HP_httpparser_on_multi_headers_complete_post > 0) {
		int (*postHookFunc) (int retVal___, struct multipartparser *parser);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_multi_headers_complete_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_on_multi_headers_complete_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parser);
		}
	}
	return retVal___;
}
int HP_httpparser_on_multi_data(struct multipartparser *parser, const char *data, size_t size) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_httpparser_on_multi_data_pre > 0) {
		int (*preHookFunc) (struct multipartparser **parser, const char **data, size_t *size);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_multi_data_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_on_multi_data_pre[hIndex].func;
			retVal___ = preHookFunc(&parser, &data, &size);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.on_multi_data(parser, data, size);
	}
	if (HPMHooks.count.HP_httpparser_on_multi_data_post > 0) {
		int (*postHookFunc) (int retVal___, struct multipartparser *parser, const char *data, size_t size);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_multi_data_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_on_multi_data_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parser, data, size);
		}
	}
	return retVal___;
}
int HP_httpparser_on_multi_part_end(struct multipartparser *parser) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_httpparser_on_multi_part_end_pre > 0) {
		int (*preHookFunc) (struct multipartparser **parser);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_multi_part_end_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_on_multi_part_end_pre[hIndex].func;
			retVal___ = preHookFunc(&parser);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.on_multi_part_end(parser);
	}
	if (HPMHooks.count.HP_httpparser_on_multi_part_end_post > 0) {
		int (*postHookFunc) (int retVal___, struct multipartparser *parser);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_multi_part_end_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_on_multi_part_end_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parser);
		}
	}
	return retVal___;
}
int HP_httpparser_on_multi_body_end(struct multipartparser *parser) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_httpparser_on_multi_body_end_pre > 0) {
		int (*preHookFunc) (struct multipartparser **parser);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_multi_body_end_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpparser_on_multi_body_end_pre[hIndex].func;
			retVal___ = preHookFunc(&parser);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpparser.on_multi_body_end(parser);
	}
	if (HPMHooks.count.HP_httpparser_on_multi_body_end_post > 0) {
		int (*postHookFunc) (int retVal___, struct multipartparser *parser);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpparser_on_multi_body_end_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpparser_on_multi_body_end_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parser);
		}
	}
	return retVal___;
}
/* httpsender_interface */
int HP_httpsender_init(bool minimal) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_httpsender_init_pre > 0) {
		int (*preHookFunc) (bool *minimal);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpsender_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpsender_init_pre[hIndex].func;
			retVal___ = preHookFunc(&minimal);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpsender.init(minimal);
	}
	if (HPMHooks.count.HP_httpsender_init_post > 0) {
		int (*postHookFunc) (int retVal___, bool minimal);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpsender_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpsender_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, minimal);
		}
	}
	return retVal___;
}
void HP_httpsender_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_httpsender_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpsender_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpsender_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.httpsender.final();
	}
	if (HPMHooks.count.HP_httpsender_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpsender_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpsender_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
bool HP_httpsender_send_plain(int fd, const char *data) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_httpsender_send_plain_pre > 0) {
		bool (*preHookFunc) (int *fd, const char **data);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpsender_send_plain_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpsender_send_plain_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &data);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpsender.send_plain(fd, data);
	}
	if (HPMHooks.count.HP_httpsender_send_plain_post > 0) {
		bool (*postHookFunc) (bool retVal___, int fd, const char *data);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpsender_send_plain_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpsender_send_plain_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, data);
		}
	}
	return retVal___;
}
bool HP_httpsender_send_html(int fd, const char *data) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_httpsender_send_html_pre > 0) {
		bool (*preHookFunc) (int *fd, const char **data);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpsender_send_html_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpsender_send_html_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &data);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpsender.send_html(fd, data);
	}
	if (HPMHooks.count.HP_httpsender_send_html_post > 0) {
		bool (*postHookFunc) (bool retVal___, int fd, const char *data);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpsender_send_html_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpsender_send_html_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, data);
		}
	}
	return retVal___;
}
bool HP_httpsender_send_json(int fd, const JsonW *json) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_httpsender_send_json_pre > 0) {
		bool (*preHookFunc) (int *fd, const JsonW **json);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpsender_send_json_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpsender_send_json_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &json);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpsender.send_json(fd, json);
	}
	if (HPMHooks.count.HP_httpsender_send_json_post > 0) {
		bool (*postHookFunc) (bool retVal___, int fd, const JsonW *json);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpsender_send_json_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpsender_send_json_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, json);
		}
	}
	return retVal___;
}
bool HP_httpsender_send_binary(int fd, const char *data, const size_t data_len) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_httpsender_send_binary_pre > 0) {
		bool (*preHookFunc) (int *fd, const char **data, const size_t *data_len);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpsender_send_binary_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_httpsender_send_binary_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &data, &data_len);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.httpsender.send_binary(fd, data, data_len);
	}
	if (HPMHooks.count.HP_httpsender_send_binary_post > 0) {
		bool (*postHookFunc) (bool retVal___, int fd, const char *data, const size_t data_len);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_httpsender_send_binary_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_httpsender_send_binary_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, data, data_len);
		}
	}
	return retVal___;
}
/* jsonparser_interface */
int HP_jsonparser_init(bool minimal) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_jsonparser_init_pre > 0) {
		int (*preHookFunc) (bool *minimal);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonparser_init_pre[hIndex].func;
			retVal___ = preHookFunc(&minimal);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonparser.init(minimal);
	}
	if (HPMHooks.count.HP_jsonparser_init_post > 0) {
		int (*postHookFunc) (int retVal___, bool minimal);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonparser_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, minimal);
		}
	}
	return retVal___;
}
void HP_jsonparser_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_jsonparser_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonparser_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.jsonparser.final();
	}
	if (HPMHooks.count.HP_jsonparser_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonparser_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
JsonP* HP_jsonparser_parse(const char *text) {
	int hIndex = 0;
	JsonP* retVal___ = NULL;
	if (HPMHooks.count.HP_jsonparser_parse_pre > 0) {
		JsonP* (*preHookFunc) (const char **text);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_parse_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonparser_parse_pre[hIndex].func;
			retVal___ = preHookFunc(&text);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonparser.parse(text);
	}
	if (HPMHooks.count.HP_jsonparser_parse_post > 0) {
		JsonP* (*postHookFunc) (JsonP* retVal___, const char *text);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_parse_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonparser_parse_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, text);
		}
	}
	return retVal___;
}
void HP_jsonparser_print(const JsonP *parent) {
	int hIndex = 0;
	if (HPMHooks.count.HP_jsonparser_print_pre > 0) {
		void (*preHookFunc) (const JsonP **parent);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_print_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonparser_print_pre[hIndex].func;
			preHookFunc(&parent);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.jsonparser.print(parent);
	}
	if (HPMHooks.count.HP_jsonparser_print_post > 0) {
		void (*postHookFunc) (const JsonP *parent);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_print_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonparser_print_post[hIndex].func;
			postHookFunc(parent);
		}
	}
	return;
}
char* HP_jsonparser_get_string(const JsonP *parent) {
	int hIndex = 0;
	char* retVal___ = NULL;
	if (HPMHooks.count.HP_jsonparser_get_string_pre > 0) {
		char* (*preHookFunc) (const JsonP **parent);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_get_string_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonparser_get_string_pre[hIndex].func;
			retVal___ = preHookFunc(&parent);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonparser.get_string(parent);
	}
	if (HPMHooks.count.HP_jsonparser_get_string_post > 0) {
		char* (*postHookFunc) (char* retVal___, const JsonP *parent);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_get_string_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonparser_get_string_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent);
		}
	}
	return retVal___;
}
char* HP_jsonparser_get_formatted_string(const JsonP *parent) {
	int hIndex = 0;
	char* retVal___ = NULL;
	if (HPMHooks.count.HP_jsonparser_get_formatted_string_pre > 0) {
		char* (*preHookFunc) (const JsonP **parent);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_get_formatted_string_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonparser_get_formatted_string_pre[hIndex].func;
			retVal___ = preHookFunc(&parent);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonparser.get_formatted_string(parent);
	}
	if (HPMHooks.count.HP_jsonparser_get_formatted_string_post > 0) {
		char* (*postHookFunc) (char* retVal___, const JsonP *parent);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_get_formatted_string_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonparser_get_formatted_string_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent);
		}
	}
	return retVal___;
}
bool HP_jsonparser_is_null(const JsonP *parent) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_jsonparser_is_null_pre > 0) {
		bool (*preHookFunc) (const JsonP **parent);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_is_null_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonparser_is_null_pre[hIndex].func;
			retVal___ = preHookFunc(&parent);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonparser.is_null(parent);
	}
	if (HPMHooks.count.HP_jsonparser_is_null_post > 0) {
		bool (*postHookFunc) (bool retVal___, const JsonP *parent);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_is_null_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonparser_is_null_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent);
		}
	}
	return retVal___;
}
bool HP_jsonparser_is_null_or_missing(const JsonP *parent) {
	int hIndex = 0;
	bool retVal___ = false;
	if (HPMHooks.count.HP_jsonparser_is_null_or_missing_pre > 0) {
		bool (*preHookFunc) (const JsonP **parent);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_is_null_or_missing_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonparser_is_null_or_missing_pre[hIndex].func;
			retVal___ = preHookFunc(&parent);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonparser.is_null_or_missing(parent);
	}
	if (HPMHooks.count.HP_jsonparser_is_null_or_missing_post > 0) {
		bool (*postHookFunc) (bool retVal___, const JsonP *parent);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_is_null_or_missing_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonparser_is_null_or_missing_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent);
		}
	}
	return retVal___;
}
JsonP* HP_jsonparser_get(const JsonP *parent, const char *name) {
	int hIndex = 0;
	JsonP* retVal___ = NULL;
	if (HPMHooks.count.HP_jsonparser_get_pre > 0) {
		JsonP* (*preHookFunc) (const JsonP **parent, const char **name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_get_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonparser_get_pre[hIndex].func;
			retVal___ = preHookFunc(&parent, &name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonparser.get(parent, name);
	}
	if (HPMHooks.count.HP_jsonparser_get_post > 0) {
		JsonP* (*postHookFunc) (JsonP* retVal___, const JsonP *parent, const char *name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_get_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonparser_get_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent, name);
		}
	}
	return retVal___;
}
int HP_jsonparser_get_array_size(const JsonP *parent) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_jsonparser_get_array_size_pre > 0) {
		int (*preHookFunc) (const JsonP **parent);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_get_array_size_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonparser_get_array_size_pre[hIndex].func;
			retVal___ = preHookFunc(&parent);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonparser.get_array_size(parent);
	}
	if (HPMHooks.count.HP_jsonparser_get_array_size_post > 0) {
		int (*postHookFunc) (int retVal___, const JsonP *parent);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_get_array_size_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonparser_get_array_size_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent);
		}
	}
	return retVal___;
}
char* HP_jsonparser_get_string_value(const JsonP *parent) {
	int hIndex = 0;
	char* retVal___ = NULL;
	if (HPMHooks.count.HP_jsonparser_get_string_value_pre > 0) {
		char* (*preHookFunc) (const JsonP **parent);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_get_string_value_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonparser_get_string_value_pre[hIndex].func;
			retVal___ = preHookFunc(&parent);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonparser.get_string_value(parent);
	}
	if (HPMHooks.count.HP_jsonparser_get_string_value_post > 0) {
		char* (*postHookFunc) (char* retVal___, const JsonP *parent);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_get_string_value_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonparser_get_string_value_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent);
		}
	}
	return retVal___;
}
double HP_jsonparser_get_number_value(const JsonP *parent) {
	int hIndex = 0;
	double retVal___ = 0.;
	if (HPMHooks.count.HP_jsonparser_get_number_value_pre > 0) {
		double (*preHookFunc) (const JsonP **parent);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_get_number_value_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonparser_get_number_value_pre[hIndex].func;
			retVal___ = preHookFunc(&parent);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonparser.get_number_value(parent);
	}
	if (HPMHooks.count.HP_jsonparser_get_number_value_post > 0) {
		double (*postHookFunc) (double retVal___, const JsonP *parent);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_get_number_value_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonparser_get_number_value_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent);
		}
	}
	return retVal___;
}
int HP_jsonparser_get_int_value(const JsonP *parent) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_jsonparser_get_int_value_pre > 0) {
		int (*preHookFunc) (const JsonP **parent);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_get_int_value_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonparser_get_int_value_pre[hIndex].func;
			retVal___ = preHookFunc(&parent);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonparser.get_int_value(parent);
	}
	if (HPMHooks.count.HP_jsonparser_get_int_value_post > 0) {
		int (*postHookFunc) (int retVal___, const JsonP *parent);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_get_int_value_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonparser_get_int_value_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent);
		}
	}
	return retVal___;
}
char* HP_jsonparser_get_child_string_value(const JsonP *parent, const char *name) {
	int hIndex = 0;
	char* retVal___ = NULL;
	if (HPMHooks.count.HP_jsonparser_get_child_string_value_pre > 0) {
		char* (*preHookFunc) (const JsonP **parent, const char **name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_get_child_string_value_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonparser_get_child_string_value_pre[hIndex].func;
			retVal___ = preHookFunc(&parent, &name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonparser.get_child_string_value(parent, name);
	}
	if (HPMHooks.count.HP_jsonparser_get_child_string_value_post > 0) {
		char* (*postHookFunc) (char* retVal___, const JsonP *parent, const char *name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_get_child_string_value_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonparser_get_child_string_value_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent, name);
		}
	}
	return retVal___;
}
double HP_jsonparser_get_child_number_value(const JsonP *parent, const char *name) {
	int hIndex = 0;
	double retVal___ = 0.;
	if (HPMHooks.count.HP_jsonparser_get_child_number_value_pre > 0) {
		double (*preHookFunc) (const JsonP **parent, const char **name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_get_child_number_value_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonparser_get_child_number_value_pre[hIndex].func;
			retVal___ = preHookFunc(&parent, &name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonparser.get_child_number_value(parent, name);
	}
	if (HPMHooks.count.HP_jsonparser_get_child_number_value_post > 0) {
		double (*postHookFunc) (double retVal___, const JsonP *parent, const char *name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_get_child_number_value_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonparser_get_child_number_value_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent, name);
		}
	}
	return retVal___;
}
int HP_jsonparser_get_child_int_value(const JsonP *parent, const char *name) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_jsonparser_get_child_int_value_pre > 0) {
		int (*preHookFunc) (const JsonP **parent, const char **name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_get_child_int_value_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonparser_get_child_int_value_pre[hIndex].func;
			retVal___ = preHookFunc(&parent, &name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonparser.get_child_int_value(parent, name);
	}
	if (HPMHooks.count.HP_jsonparser_get_child_int_value_post > 0) {
		int (*postHookFunc) (int retVal___, const JsonP *parent, const char *name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_get_child_int_value_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonparser_get_child_int_value_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent, name);
		}
	}
	return retVal___;
}
void HP_jsonparser_free(char *ptr) {
	int hIndex = 0;
	if (HPMHooks.count.HP_jsonparser_free_pre > 0) {
		void (*preHookFunc) (char **ptr);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_free_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonparser_free_pre[hIndex].func;
			preHookFunc(&ptr);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.jsonparser.free(ptr);
	}
	if (HPMHooks.count.HP_jsonparser_free_post > 0) {
		void (*postHookFunc) (char *ptr);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_free_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonparser_free_post[hIndex].func;
			postHookFunc(ptr);
		}
	}
	return;
}
void HP_jsonparser_delete(JsonP *ptr) {
	int hIndex = 0;
	if (HPMHooks.count.HP_jsonparser_delete_pre > 0) {
		void (*preHookFunc) (JsonP **ptr);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_delete_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonparser_delete_pre[hIndex].func;
			preHookFunc(&ptr);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.jsonparser.delete(ptr);
	}
	if (HPMHooks.count.HP_jsonparser_delete_post > 0) {
		void (*postHookFunc) (JsonP *ptr);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonparser_delete_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonparser_delete_post[hIndex].func;
			postHookFunc(ptr);
		}
	}
	return;
}
/* jsonwriter_interface */
int HP_jsonwriter_init(bool minimal) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_jsonwriter_init_pre > 0) {
		int (*preHookFunc) (bool *minimal);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonwriter_init_pre[hIndex].func;
			retVal___ = preHookFunc(&minimal);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonwriter.init(minimal);
	}
	if (HPMHooks.count.HP_jsonwriter_init_post > 0) {
		int (*postHookFunc) (int retVal___, bool minimal);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonwriter_init_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, minimal);
		}
	}
	return retVal___;
}
void HP_jsonwriter_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_jsonwriter_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonwriter_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.jsonwriter.final();
	}
	if (HPMHooks.count.HP_jsonwriter_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonwriter_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
JsonW* HP_jsonwriter_create(const char *text) {
	int hIndex = 0;
	JsonW* retVal___ = NULL;
	if (HPMHooks.count.HP_jsonwriter_create_pre > 0) {
		JsonW* (*preHookFunc) (const char **text);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_create_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonwriter_create_pre[hIndex].func;
			retVal___ = preHookFunc(&text);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonwriter.create(text);
	}
	if (HPMHooks.count.HP_jsonwriter_create_post > 0) {
		JsonW* (*postHookFunc) (JsonW* retVal___, const char *text);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_create_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonwriter_create_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, text);
		}
	}
	return retVal___;
}
JsonW* HP_jsonwriter_new_array(void) {
	int hIndex = 0;
	JsonW* retVal___ = NULL;
	if (HPMHooks.count.HP_jsonwriter_new_array_pre > 0) {
		JsonW* (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_new_array_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonwriter_new_array_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonwriter.new_array();
	}
	if (HPMHooks.count.HP_jsonwriter_new_array_post > 0) {
		JsonW* (*postHookFunc) (JsonW* retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_new_array_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonwriter_new_array_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
JsonW* HP_jsonwriter_new_object(void) {
	int hIndex = 0;
	JsonW* retVal___ = NULL;
	if (HPMHooks.count.HP_jsonwriter_new_object_pre > 0) {
		JsonW* (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_new_object_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonwriter_new_object_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonwriter.new_object();
	}
	if (HPMHooks.count.HP_jsonwriter_new_object_post > 0) {
		JsonW* (*postHookFunc) (JsonW* retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_new_object_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonwriter_new_object_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
JsonW* HP_jsonwriter_new_string(const char *str) {
	int hIndex = 0;
	JsonW* retVal___ = NULL;
	if (HPMHooks.count.HP_jsonwriter_new_string_pre > 0) {
		JsonW* (*preHookFunc) (const char **str);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_new_string_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonwriter_new_string_pre[hIndex].func;
			retVal___ = preHookFunc(&str);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonwriter.new_string(str);
	}
	if (HPMHooks.count.HP_jsonwriter_new_string_post > 0) {
		JsonW* (*postHookFunc) (JsonW* retVal___, const char *str);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_new_string_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonwriter_new_string_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, str);
		}
	}
	return retVal___;
}
JsonW* HP_jsonwriter_new_number(int number) {
	int hIndex = 0;
	JsonW* retVal___ = NULL;
	if (HPMHooks.count.HP_jsonwriter_new_number_pre > 0) {
		JsonW* (*preHookFunc) (int *number);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_new_number_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonwriter_new_number_pre[hIndex].func;
			retVal___ = preHookFunc(&number);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonwriter.new_number(number);
	}
	if (HPMHooks.count.HP_jsonwriter_new_number_post > 0) {
		JsonW* (*postHookFunc) (JsonW* retVal___, int number);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_new_number_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonwriter_new_number_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, number);
		}
	}
	return retVal___;
}
JsonW* HP_jsonwriter_new_null(void) {
	int hIndex = 0;
	JsonW* retVal___ = NULL;
	if (HPMHooks.count.HP_jsonwriter_new_null_pre > 0) {
		JsonW* (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_new_null_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonwriter_new_null_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonwriter.new_null();
	}
	if (HPMHooks.count.HP_jsonwriter_new_null_post > 0) {
		JsonW* (*postHookFunc) (JsonW* retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_new_null_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonwriter_new_null_post[hIndex].func;
			retVal___ = postHookFunc(retVal___);
		}
	}
	return retVal___;
}
JsonWBool HP_jsonwriter_add_node(JsonW *parent, const char *name, JsonW *child) {
	int hIndex = 0;
	JsonWBool retVal___ = cJSON_False;
	if (HPMHooks.count.HP_jsonwriter_add_node_pre > 0) {
		JsonWBool (*preHookFunc) (JsonW **parent, const char **name, JsonW **child);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_add_node_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonwriter_add_node_pre[hIndex].func;
			retVal___ = preHookFunc(&parent, &name, &child);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonwriter.add_node(parent, name, child);
	}
	if (HPMHooks.count.HP_jsonwriter_add_node_post > 0) {
		JsonWBool (*postHookFunc) (JsonWBool retVal___, JsonW *parent, const char *name, JsonW *child);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_add_node_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonwriter_add_node_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent, name, child);
		}
	}
	return retVal___;
}
JsonWBool HP_jsonwriter_add_node_to_array(JsonW *parent, JsonW *child) {
	int hIndex = 0;
	JsonWBool retVal___ = cJSON_False;
	if (HPMHooks.count.HP_jsonwriter_add_node_to_array_pre > 0) {
		JsonWBool (*preHookFunc) (JsonW **parent, JsonW **child);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_add_node_to_array_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonwriter_add_node_to_array_pre[hIndex].func;
			retVal___ = preHookFunc(&parent, &child);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonwriter.add_node_to_array(parent, child);
	}
	if (HPMHooks.count.HP_jsonwriter_add_node_to_array_post > 0) {
		JsonWBool (*postHookFunc) (JsonWBool retVal___, JsonW *parent, JsonW *child);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_add_node_to_array_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonwriter_add_node_to_array_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent, child);
		}
	}
	return retVal___;
}
JsonW* HP_jsonwriter_add_new_array(JsonW *parent, const char *name) {
	int hIndex = 0;
	JsonW* retVal___ = NULL;
	if (HPMHooks.count.HP_jsonwriter_add_new_array_pre > 0) {
		JsonW* (*preHookFunc) (JsonW **parent, const char **name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_add_new_array_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonwriter_add_new_array_pre[hIndex].func;
			retVal___ = preHookFunc(&parent, &name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonwriter.add_new_array(parent, name);
	}
	if (HPMHooks.count.HP_jsonwriter_add_new_array_post > 0) {
		JsonW* (*postHookFunc) (JsonW* retVal___, JsonW *parent, const char *name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_add_new_array_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonwriter_add_new_array_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent, name);
		}
	}
	return retVal___;
}
JsonW* HP_jsonwriter_add_new_object(JsonW *parent, const char *name) {
	int hIndex = 0;
	JsonW* retVal___ = NULL;
	if (HPMHooks.count.HP_jsonwriter_add_new_object_pre > 0) {
		JsonW* (*preHookFunc) (JsonW **parent, const char **name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_add_new_object_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonwriter_add_new_object_pre[hIndex].func;
			retVal___ = preHookFunc(&parent, &name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonwriter.add_new_object(parent, name);
	}
	if (HPMHooks.count.HP_jsonwriter_add_new_object_post > 0) {
		JsonW* (*postHookFunc) (JsonW* retVal___, JsonW *parent, const char *name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_add_new_object_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonwriter_add_new_object_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent, name);
		}
	}
	return retVal___;
}
JsonW* HP_jsonwriter_add_new_null(JsonW *parent, const char *name) {
	int hIndex = 0;
	JsonW* retVal___ = NULL;
	if (HPMHooks.count.HP_jsonwriter_add_new_null_pre > 0) {
		JsonW* (*preHookFunc) (JsonW **parent, const char **name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_add_new_null_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonwriter_add_new_null_pre[hIndex].func;
			retVal___ = preHookFunc(&parent, &name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonwriter.add_new_null(parent, name);
	}
	if (HPMHooks.count.HP_jsonwriter_add_new_null_post > 0) {
		JsonW* (*postHookFunc) (JsonW* retVal___, JsonW *parent, const char *name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_add_new_null_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonwriter_add_new_null_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent, name);
		}
	}
	return retVal___;
}
JsonW* HP_jsonwriter_add_new_number(JsonW *parent, const char *name, int number) {
	int hIndex = 0;
	JsonW* retVal___ = NULL;
	if (HPMHooks.count.HP_jsonwriter_add_new_number_pre > 0) {
		JsonW* (*preHookFunc) (JsonW **parent, const char **name, int *number);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_add_new_number_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonwriter_add_new_number_pre[hIndex].func;
			retVal___ = preHookFunc(&parent, &name, &number);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonwriter.add_new_number(parent, name, number);
	}
	if (HPMHooks.count.HP_jsonwriter_add_new_number_post > 0) {
		JsonW* (*postHookFunc) (JsonW* retVal___, JsonW *parent, const char *name, int number);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_add_new_number_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonwriter_add_new_number_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent, name, number);
		}
	}
	return retVal___;
}
JsonW* HP_jsonwriter_add_new_string(JsonW *parent, const char *name, const char *str) {
	int hIndex = 0;
	JsonW* retVal___ = NULL;
	if (HPMHooks.count.HP_jsonwriter_add_new_string_pre > 0) {
		JsonW* (*preHookFunc) (JsonW **parent, const char **name, const char **str);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_add_new_string_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonwriter_add_new_string_pre[hIndex].func;
			retVal___ = preHookFunc(&parent, &name, &str);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonwriter.add_new_string(parent, name, str);
	}
	if (HPMHooks.count.HP_jsonwriter_add_new_string_post > 0) {
		JsonW* (*postHookFunc) (JsonW* retVal___, JsonW *parent, const char *name, const char *str);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_add_new_string_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonwriter_add_new_string_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent, name, str);
		}
	}
	return retVal___;
}
JsonW* HP_jsonwriter_add_string_to_array(JsonW *parent, const char *str) {
	int hIndex = 0;
	JsonW* retVal___ = NULL;
	if (HPMHooks.count.HP_jsonwriter_add_string_to_array_pre > 0) {
		JsonW* (*preHookFunc) (JsonW **parent, const char **str);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_add_string_to_array_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonwriter_add_string_to_array_pre[hIndex].func;
			retVal___ = preHookFunc(&parent, &str);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonwriter.add_string_to_array(parent, str);
	}
	if (HPMHooks.count.HP_jsonwriter_add_string_to_array_post > 0) {
		JsonW* (*postHookFunc) (JsonW* retVal___, JsonW *parent, const char *str);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_add_string_to_array_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonwriter_add_string_to_array_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent, str);
		}
	}
	return retVal___;
}
void HP_jsonwriter_print(const JsonW *parent) {
	int hIndex = 0;
	if (HPMHooks.count.HP_jsonwriter_print_pre > 0) {
		void (*preHookFunc) (const JsonW **parent);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_print_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonwriter_print_pre[hIndex].func;
			preHookFunc(&parent);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.jsonwriter.print(parent);
	}
	if (HPMHooks.count.HP_jsonwriter_print_post > 0) {
		void (*postHookFunc) (const JsonW *parent);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_print_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonwriter_print_post[hIndex].func;
			postHookFunc(parent);
		}
	}
	return;
}
JsonW* HP_jsonwriter_get(const JsonW *parent, const char *name) {
	int hIndex = 0;
	JsonW* retVal___ = NULL;
	if (HPMHooks.count.HP_jsonwriter_get_pre > 0) {
		JsonW* (*preHookFunc) (const JsonW **parent, const char **name);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_get_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonwriter_get_pre[hIndex].func;
			retVal___ = preHookFunc(&parent, &name);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonwriter.get(parent, name);
	}
	if (HPMHooks.count.HP_jsonwriter_get_post > 0) {
		JsonW* (*postHookFunc) (JsonW* retVal___, const JsonW *parent, const char *name);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_get_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonwriter_get_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent, name);
		}
	}
	return retVal___;
}
char* HP_jsonwriter_get_string(const JsonW *parent) {
	int hIndex = 0;
	char* retVal___ = NULL;
	if (HPMHooks.count.HP_jsonwriter_get_string_pre > 0) {
		char* (*preHookFunc) (const JsonW **parent);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_get_string_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonwriter_get_string_pre[hIndex].func;
			retVal___ = preHookFunc(&parent);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonwriter.get_string(parent);
	}
	if (HPMHooks.count.HP_jsonwriter_get_string_post > 0) {
		char* (*postHookFunc) (char* retVal___, const JsonW *parent);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_get_string_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonwriter_get_string_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent);
		}
	}
	return retVal___;
}
char* HP_jsonwriter_get_formatted_string(const JsonW *parent) {
	int hIndex = 0;
	char* retVal___ = NULL;
	if (HPMHooks.count.HP_jsonwriter_get_formatted_string_pre > 0) {
		char* (*preHookFunc) (const JsonW **parent);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_get_formatted_string_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonwriter_get_formatted_string_pre[hIndex].func;
			retVal___ = preHookFunc(&parent);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.jsonwriter.get_formatted_string(parent);
	}
	if (HPMHooks.count.HP_jsonwriter_get_formatted_string_post > 0) {
		char* (*postHookFunc) (char* retVal___, const JsonW *parent);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_get_formatted_string_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonwriter_get_formatted_string_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, parent);
		}
	}
	return retVal___;
}
void HP_jsonwriter_free(char *ptr) {
	int hIndex = 0;
	if (HPMHooks.count.HP_jsonwriter_free_pre > 0) {
		void (*preHookFunc) (char **ptr);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_free_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonwriter_free_pre[hIndex].func;
			preHookFunc(&ptr);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.jsonwriter.free(ptr);
	}
	if (HPMHooks.count.HP_jsonwriter_free_post > 0) {
		void (*postHookFunc) (char *ptr);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_free_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonwriter_free_post[hIndex].func;
			postHookFunc(ptr);
		}
	}
	return;
}
void HP_jsonwriter_delete(JsonW *ptr) {
	int hIndex = 0;
	if (HPMHooks.count.HP_jsonwriter_delete_pre > 0) {
		void (*preHookFunc) (JsonW **ptr);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_delete_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_jsonwriter_delete_pre[hIndex].func;
			preHookFunc(&ptr);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.jsonwriter.delete(ptr);
	}
	if (HPMHooks.count.HP_jsonwriter_delete_post > 0) {
		void (*postHookFunc) (JsonW *ptr);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_jsonwriter_delete_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_jsonwriter_delete_post[hIndex].func;
			postHookFunc(ptr);
		}
	}
	return;
}
/* libconfig_interface */
void HP_libconfig_set_db_path(const char *db_path) {
	int hIndex = 0;
	if (HPMHooks.count.HP_libconfig_set_db_path_pre > 0) {
		void (*preHookFunc) (const char **db_path);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_set_db_path_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_set_db_path_pre[hIndex].func;
			preHookFunc(&db_path);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.libconfig.set_db_path(db_path);
	}
	if (HPMHooks.count.HP_libconfig_set_db_path_post > 0) {
		void (*postHookFunc) (const char *db_path);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_set_db_path_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_set_db_path_post[hIndex].func;
			postHookFunc(db_path);
		}
	}
	return;
}
void HP_libconfig_format_db_path(const char *filename, char *path_buf, int buffer_len) {
	int hIndex = 0;
	if (HPMHooks.count.HP_libconfig_format_db_path_pre > 0) {
		void (*preHookFunc) (const char **filename, char **path_buf, int *buffer_len);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_format_db_path_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_libconfig_format_db_path_pre[hIndex].func;
			preHookFunc(&filename, &path_buf, &buffer_len);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.libconfig.format_db_path(filename, path_buf, buffer_len);
	}
	if (HPMHooks.count.HP_libconfig_format_db_path_post > 0) {
		void (*postHookFunc) (const char *filename, char *path_buf, int buffer_len);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_libconfig_format_db_path_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_libconfig_format_db_path_post[hIndex].func;
			postHookFunc(filename, path_buf, buffer_len);
		}
	}
	return;
}
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
void HP_nullpo_init(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_nullpo_init_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_nullpo_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_nullpo_init_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.nullpo.init();
	}
	if (HPMHooks.count.HP_nullpo_init_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_nullpo_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_nullpo_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_nullpo_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_nullpo_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_nullpo_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_nullpo_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.nullpo.final();
	}
	if (HPMHooks.count.HP_nullpo_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_nullpo_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_nullpo_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
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
/* packets_interface */
void HP_packets_init(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_packets_init_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_packets_init_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_packets_init_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.packets.init();
	}
	if (HPMHooks.count.HP_packets_init_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_packets_init_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_packets_init_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_packets_final(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_packets_final_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_packets_final_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_packets_final_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.packets.final();
	}
	if (HPMHooks.count.HP_packets_final_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_packets_final_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_packets_final_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_packets_addLens(void) {
	int hIndex = 0;
	if (HPMHooks.count.HP_packets_addLens_pre > 0) {
		void (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_packets_addLens_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_packets_addLens_pre[hIndex].func;
			preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.packets.addLens();
	}
	if (HPMHooks.count.HP_packets_addLens_post > 0) {
		void (*postHookFunc) (void);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_packets_addLens_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_packets_addLens_post[hIndex].func;
			postHookFunc();
		}
	}
	return;
}
void HP_packets_addLen(int id, int len) {
	int hIndex = 0;
	if (HPMHooks.count.HP_packets_addLen_pre > 0) {
		void (*preHookFunc) (int *id, int *len);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_packets_addLen_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_packets_addLen_pre[hIndex].func;
			preHookFunc(&id, &len);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.packets.addLen(id, len);
	}
	if (HPMHooks.count.HP_packets_addLen_post > 0) {
		void (*postHookFunc) (int id, int len);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_packets_addLen_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_packets_addLen_post[hIndex].func;
			postHookFunc(id, len);
		}
	}
	return;
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
int HP_sockt_wfifoset(int fd, size_t len, bool validate) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_sockt_wfifoset_pre > 0) {
		int (*preHookFunc) (int *fd, size_t *len, bool *validate);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_wfifoset_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_wfifoset_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &len, &validate);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.wfifoset(fd, len, validate);
	}
	if (HPMHooks.count.HP_sockt_wfifoset_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, size_t len, bool validate);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_wfifoset_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_wfifoset_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, len, validate);
		}
	}
	return retVal___;
}
void HP_sockt_wfifohead(int fd, size_t len) {
	int hIndex = 0;
	if (HPMHooks.count.HP_sockt_wfifohead_pre > 0) {
		void (*preHookFunc) (int *fd, size_t *len);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_wfifohead_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_wfifohead_pre[hIndex].func;
			preHookFunc(&fd, &len);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.wfifohead(fd, len);
	}
	if (HPMHooks.count.HP_sockt_wfifohead_post > 0) {
		void (*postHookFunc) (int fd, size_t len);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_wfifohead_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_wfifohead_post[hIndex].func;
			postHookFunc(fd, len);
		}
	}
	return;
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
void HP_sockt_validateWfifo(int fd, size_t len) {
	int hIndex = 0;
	if (HPMHooks.count.HP_sockt_validateWfifo_pre > 0) {
		void (*preHookFunc) (int *fd, size_t *len);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_validateWfifo_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_validateWfifo_pre[hIndex].func;
			preHookFunc(&fd, &len);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.validateWfifo(fd, len);
	}
	if (HPMHooks.count.HP_sockt_validateWfifo_post > 0) {
		void (*postHookFunc) (int fd, size_t len);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_validateWfifo_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_validateWfifo_post[hIndex].func;
			postHookFunc(fd, len);
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
int HP_sockt_create_session(int fd, RecvFunc func_recv, SendFunc func_send, ParseFunc func_parse, ConnectedFunc func_client_connected, DeleteFunc func_delete) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_sockt_create_session_pre > 0) {
		int (*preHookFunc) (int *fd, RecvFunc *func_recv, SendFunc *func_send, ParseFunc *func_parse, ConnectedFunc *func_client_connected, DeleteFunc *func_delete);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_create_session_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_create_session_pre[hIndex].func;
			retVal___ = preHookFunc(&fd, &func_recv, &func_send, &func_parse, &func_client_connected, &func_delete);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.create_session(fd, func_recv, func_send, func_parse, func_client_connected, func_delete);
	}
	if (HPMHooks.count.HP_sockt_create_session_post > 0) {
		int (*postHookFunc) (int retVal___, int fd, RecvFunc func_recv, SendFunc func_send, ParseFunc func_parse, ConnectedFunc func_client_connected, DeleteFunc func_delete);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_create_session_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_create_session_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, fd, func_recv, func_send, func_parse, func_client_connected, func_delete);
		}
	}
	return retVal___;
}
void HP_sockt_delete_session(int fd) {
	int hIndex = 0;
	if (HPMHooks.count.HP_sockt_delete_session_pre > 0) {
		void (*preHookFunc) (int *fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_delete_session_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_delete_session_pre[hIndex].func;
			preHookFunc(&fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.delete_session(fd);
	}
	if (HPMHooks.count.HP_sockt_delete_session_post > 0) {
		void (*postHookFunc) (int fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_delete_session_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_delete_session_post[hIndex].func;
			postHookFunc(fd);
		}
	}
	return;
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
int HP_sockt_connect_client(int listen_fd) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_sockt_connect_client_pre > 0) {
		int (*preHookFunc) (int *listen_fd);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_connect_client_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_connect_client_pre[hIndex].func;
			retVal___ = preHookFunc(&listen_fd);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sockt.connect_client(listen_fd);
	}
	if (HPMHooks.count.HP_sockt_connect_client_post > 0) {
		int (*postHookFunc) (int retVal___, int listen_fd);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_connect_client_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_connect_client_post[hIndex].func;
			retVal___ = postHookFunc(retVal___, listen_fd);
		}
	}
	return retVal___;
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
void HP_sockt_set_default_client_connected(ConnectedFunc defaultparse) {
	int hIndex = 0;
	if (HPMHooks.count.HP_sockt_set_default_client_connected_pre > 0) {
		void (*preHookFunc) (ConnectedFunc *defaultparse);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_set_default_client_connected_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_set_default_client_connected_pre[hIndex].func;
			preHookFunc(&defaultparse);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.set_default_client_connected(defaultparse);
	}
	if (HPMHooks.count.HP_sockt_set_default_client_connected_post > 0) {
		void (*postHookFunc) (ConnectedFunc defaultparse);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_set_default_client_connected_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_set_default_client_connected_post[hIndex].func;
			postHookFunc(defaultparse);
		}
	}
	return;
}
void HP_sockt_set_default_delete(DeleteFunc defaultdelete) {
	int hIndex = 0;
	if (HPMHooks.count.HP_sockt_set_default_delete_pre > 0) {
		void (*preHookFunc) (DeleteFunc *defaultdelete);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_set_default_delete_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sockt_set_default_delete_pre[hIndex].func;
			preHookFunc(&defaultdelete);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return;
		}
	}
	{
		HPMHooks.source.sockt.set_default_delete(defaultdelete);
	}
	if (HPMHooks.count.HP_sockt_set_default_delete_post > 0) {
		void (*postHookFunc) (DeleteFunc defaultdelete);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sockt_set_default_delete_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sockt_set_default_delete_post[hIndex].func;
			postHookFunc(defaultdelete);
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
int HP_SQL_QueryStrFetch(struct Sql *self, const char *query) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_SQL_QueryStrFetch_pre > 0) {
		int (*preHookFunc) (struct Sql **self, const char **query);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_QueryStrFetch_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_SQL_QueryStrFetch_pre[hIndex].func;
			retVal___ = preHookFunc(&self, &query);
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.SQL.QueryStrFetch(self, query);
	}
	if (HPMHooks.count.HP_SQL_QueryStrFetch_post > 0) {
		int (*postHookFunc) (int retVal___, struct Sql *self, const char *query);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_SQL_QueryStrFetch_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_SQL_QueryStrFetch_post[hIndex].func;
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
long HP_sysinfo_getpagesize(void) {
	int hIndex = 0;
	long retVal___ = 0;
	if (HPMHooks.count.HP_sysinfo_getpagesize_pre > 0) {
		long (*preHookFunc) (void);
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
		long (*postHookFunc) (long retVal___);
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
int HP_sysinfo_build_revision(void) {
	int hIndex = 0;
	int retVal___ = 0;
	if (HPMHooks.count.HP_sysinfo_build_revision_pre > 0) {
		int (*preHookFunc) (void);
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_build_revision_pre; hIndex++) {
			preHookFunc = HPMHooks.list.HP_sysinfo_build_revision_pre[hIndex].func;
			retVal___ = preHookFunc();
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return retVal___;
		}
	}
	{
		retVal___ = HPMHooks.source.sysinfo.build_revision();
	}
	if (HPMHooks.count.HP_sysinfo_build_revision_post > 0) {
		int (*postHookFunc) (int retVal___);
		for (hIndex = 0; hIndex < HPMHooks.count.HP_sysinfo_build_revision_post; hIndex++) {
			postHookFunc = HPMHooks.list.HP_sysinfo_build_revision_post[hIndex].func;
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
