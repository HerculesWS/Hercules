/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
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
#ifndef CHAR_INT_USERCONFIG_H
#define CHAR_INT_USERCONFIG_H

#include "common/hercules.h"

struct userconfig_emotes;
struct userconfig_userhotkeys_v2;

/**
 * inter_userconfig_interface interface
 **/
struct inter_userconfig_interface {
	int (*load_emotes) (int account_id, struct userconfig_emotes *emotes);
	int (*save_emotes) (int account_id, const struct userconfig_emotes *emotes);
	void (*use_default_emotes) (int account_id, struct userconfig_emotes *emotes);
	bool (*emotes_from_sql) (int account_id, struct userconfig_emotes *emotes);
	bool (*emotes_to_sql) (int account_id, const struct userconfig_emotes *emotes);
	void (*hotkey_tab_tosql) (int account_id, const struct userconfig_userhotkeys_v2 *hotkeys);
	void (*hotkey_tab_clear) (int account_id, int tab_id);
	void (*hotkey_tab_fromsql) (int account_id, struct userconfig_userhotkeys_v2 *hotkeys, int tab_id);
};

#ifdef HERCULES_CORE
void inter_userconfig_defaults(void);
#endif // HERCULES_CORE

HPShared struct inter_userconfig_interface *inter_userconfig;

#endif /* CHAR_INT_USERCONFIG_H */
