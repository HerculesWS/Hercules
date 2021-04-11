/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
 * Copyright (C) 2020-2021 Andrei Karas (4144)
 * Copyright (C) Athena Dev Teams
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
#ifndef CHAR_CAPIIF_H
#define CHAR_CAPIIF_H

#include "common/hercules.h"

struct online_char_data;
struct PACKET_API_PROXY;

/**
 * capiif interface
 **/
struct capiif_interface {
	void (*init) (void);
	void (*final) (void);
	struct online_char_data* (*get_online_character) (const struct PACKET_API_PROXY *p);
	void (*emblem_download) (int fd, int guild_id, int emblem_id);
	void (*parse_userconfig_load_emotes) (int fd);
	void (*parse_userconfig_save_emotes) (int fd);
	void (*parse_charconfig_load) (int fd);
	void (*parse_emblem_upload) (int fd);
	void (*parse_emblem_upload_guild_id) (int fd);
	void (*parse_emblem_download) (int fd);
	void (*parse_userconfig_save_userhotkey_v2) (int fd);
	void (*parse_userconfig_load_hotkeys) (int fd);
	void (*parse_party_add) (int fd);
	void (*parse_party_list) (int fd);
	int (*parse_fromlogin_api_proxy) (int fd);
};

#ifdef HERCULES_CORE
void capiif_defaults(void);
#endif // HERCULES_CORE

HPShared struct capiif_interface *capiif;

#endif /* CHAR_CAPIIF_H */
