/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2025 Hercules Dev Team
 * Copyright (C) 2020-2022 Andrei Karas (4144)
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
#ifndef COMMON_API_H
#define COMMON_API_H

#define MAX_CUSTOM_API_MSG 100

enum API_MSG { API_MSG_userconfig_load = 1, API_MSG_userconfig_save = 2, API_MSG_charconfig_load = 3, API_MSG_userconfig_save_emotes = 4, API_MSG_userconfig_save_hotkeys_emotion = 5, API_MSG_userconfig_save_hotkeys_interface = 6, API_MSG_userconfig_save_hotkeys_skill_bar1 = 7, API_MSG_userconfig_save_hotkeys_skill_bar2 = 8, API_MSG_emblem_upload = 9, API_MSG_emblem_upload_guild_id = 10, API_MSG_emblem_download = 11, API_MSG_userconfig_save_userhotkey_v2 = 12, API_MSG_userconfig_load_emotes = 13, API_MSG_userconfig_load_hotkeys = 14, API_MSG_party_list = 15, API_MSG_party_get = 16, API_MSG_party_add = 17, API_MSG_party_del = 18, API_MSG_party_info = 19, API_MSG_CUSTOM, API_MSG_MAX = API_MSG_CUSTOM + MAX_CUSTOM_API_MSG };

#endif /* COMMON_API_H */
