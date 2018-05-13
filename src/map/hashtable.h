/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2015 - 2016 Evol Online
 * Copyright (C) 2017 - 2018 The Mana World
 * Copyright (C) 2018 Hercules Dev Team
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

#ifndef MAP_HASHTABLE_H
#define MAP_HASHTABLE_H

#include "common/hercules.h"
#include "common/db.h"

#define HT_MAX_KEY_LEN 32

struct htreg_interface
{
	int64 last_id;
	int64 last_iterator_id;
	struct DBMap *htables;
	struct DBMap *iterators;

	void (*init) (void);
	void (*final) (void);

	int64 (*new_hashtable) (void);
	bool (*destroy_hashtable) (int64 id);
	bool (*hashtable_exists) (int64 id);
	int64 (*hashtable_size) (int64 id);
	bool (*clear_hashtable) (int64 id);
	const struct DBData *(*hashtable_getvalue) (int64 id, const char *key);
	bool (*hashtable_setvalue) (int64 id, const char *key, const struct DBData value);

	int64 (*create_iterator) (int64 id);
	bool (*destroy_iterator) (int64 id);
	bool (*iterator_check) (int64 id);
	bool (*iterator_exists) (int64 id);
	const char *(*iterator_firstkey) (int64 id);
	const char *(*iterator_lastkey) (int64 id);
	const char *(*iterator_nextkey) (int64 id);
	const char *(*iterator_prevkey) (int64 id);
};

#ifdef HERCULES_CORE
void htreg_defaults(void);
#endif

HPShared struct htreg_interface *htreg;

#endif  // MAP_HASHTABLE_H
