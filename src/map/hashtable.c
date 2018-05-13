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
#define HERCULES_CORE

#include "common/hercules.h"

#include "common/db.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "map/hashtable.h"

struct htreg_interface htreg_s;
struct htreg_interface *htreg;

int64 htreg_new_hashtable(void)
{
	int64 id = htreg->last_id++;
	struct DBMap *ht = strdb_alloc(DB_OPT_DUP_KEY | DB_OPT_RELEASE_DATA, HT_MAX_KEY_LEN);
	i64db_put(htreg->htables, id, ht);
	return id;
}

bool htreg_destroy_hashtable(int64 id)
{
	struct DBMap *ht = i64db_get(htreg->htables, id);
	nullpo_retr(false, ht);

	db_destroy(ht);
	i64db_remove(htreg->htables, id);
	return true;
}

bool htreg_hashtable_exists(int64 id)
{
	return i64db_exists(htreg->htables, id);
}

int64 htreg_hashtable_size(int64 id)
{
	struct DBMap *ht = i64db_get(htreg->htables, id);
	nullpo_ret(ht);

	return db_size(ht);
}

bool htreg_clear_hashtable(int64 id)
{
	struct DBMap *ht = i64db_get(htreg->htables, id);
	nullpo_retr(false, ht);

	db_clear(ht);
	return true;
}

const struct DBData *htreg_hashtable_getvalue(int64 id, const char *key)
{
	struct DBMap *ht = i64db_get(htreg->htables, id);
	nullpo_retr(NULL, ht);

	return ht->get(ht, DB->str2key(key));
}

bool htreg_hashtable_setvalue(int64 id, const char *key, struct DBData value)
{
	struct DBMap *ht = i64db_get(htreg->htables, id);
	bool keep = false;
	nullpo_retr(false, ht);

	switch(value.type) {
	case DB_DATA_INT:
	case DB_DATA_UINT:
		keep = value.u.i != 0;
		break;
	case DB_DATA_PTR:
		keep = value.u.ptr != NULL && ((char *)value.u.ptr)[0] != '\0';
		break;
	}

	if (keep) {
		ht->put(ht, DB->str2key(key), value, NULL);
	} else {
		strdb_remove(ht, key);
	}
	return true;
}

/**
 *   Iterators
 */
int64 htreg_create_iterator(int64 htId)
{
	struct DBMap *ht = i64db_get(htreg->htables, htId);
	nullpo_ret(ht);

	int64 id = htreg->last_iterator_id++;
	struct DBIterator *it = db_iterator(ht);
	i64db_put(htreg->iterators, id, it);
	return id;
}

bool htreg_destroy_iterator(int64 id)
{
	struct DBIterator *it = i64db_get(htreg->iterators, id);
	nullpo_retr(false, it);

	dbi_destroy(it);
	i64db_remove(htreg->iterators, id);
	return true;
}

bool htreg_iterator_check(int64 id)
{
	struct DBIterator *it = i64db_get(htreg->iterators, id);
	nullpo_retr(false, it);

	return dbi_exists(it);
}

bool htreg_iterator_exists(int64 id)
{
	return i64db_exists(htreg->iterators, id);
}

const char *htreg_iterator_firstkey(int64 id)
{
	struct DBIterator *it = i64db_get(htreg->iterators, id);
	nullpo_retr(NULL, it);

	union DBKey key;
	it->first(it, &key);
	return dbi_exists(it) ? key.str : NULL;
}

const char *htreg_iterator_lastkey(int64 id)
{
	struct DBIterator *it = i64db_get(htreg->iterators, id);
	nullpo_retr(NULL, it);

	union DBKey key;
	it->last(it, &key);
	return dbi_exists(it) ? key.str : NULL;
}

const char *htreg_iterator_nextkey(int64 id)
{
	struct DBIterator *it = i64db_get(htreg->iterators, id);
	nullpo_retr(NULL, it);

	union DBKey key;
	it->next(it, &key);
	return dbi_exists(it) ? key.str : NULL;
}

const char *htreg_iterator_prevkey(int64 id)
{
	struct DBIterator *it = i64db_get(htreg->iterators, id);
	nullpo_retr(NULL, it);

	union DBKey key;
	it->prev(it, &key);
	return dbi_exists(it) ? key.str : NULL;
}

/**
 * Initializer.
 */
void htreg_init(void)
{
	htreg->htables = i64db_alloc(DB_OPT_BASE);
	htreg->iterators = i64db_alloc(DB_OPT_BASE);
}

/**
 * Finalizer.
 */
void htreg_final(void)
{

	if (htreg->iterators != NULL) {
		struct DBIterator *it;
		struct DBIterator *iter = db_iterator(htreg->iterators);

		for (it = dbi_first(iter); dbi_exists(iter); it = dbi_next(iter)) {
			dbi_destroy(it);
		}

		dbi_destroy(iter);
	}

	if (htreg->htables != NULL) {
		struct DBMap *ht;
		struct DBIterator *iter = db_iterator(htreg->htables);

		for (ht = dbi_first(iter); dbi_exists(iter); ht = dbi_next(iter)) {
			db_destroy(ht);
		}

		dbi_destroy(iter);
	}

	db_destroy(htreg->htables);
	db_destroy(htreg->iterators);
}

/**
 * Interface defaults initializer.
 */
void htreg_defaults(void)
{
	htreg = &htreg_s;

	htreg->last_id = 1;
	htreg->htables = NULL;
	htreg->init = htreg_init;
	htreg->final = htreg_final;
	htreg->new_hashtable = htreg_new_hashtable;
	htreg->destroy_hashtable = htreg_destroy_hashtable;
	htreg->hashtable_exists = htreg_hashtable_exists;
	htreg->hashtable_size = htreg_hashtable_size;
	htreg->clear_hashtable = htreg_clear_hashtable;
	htreg->hashtable_getvalue = htreg_hashtable_getvalue;
	htreg->hashtable_setvalue = htreg_hashtable_setvalue;

	htreg->last_iterator_id = 1;
	htreg->iterators = NULL;
	htreg->create_iterator = htreg_create_iterator;
	htreg->destroy_iterator = htreg_destroy_iterator;
	htreg->iterator_check = htreg_iterator_check;
	htreg->iterator_exists = htreg_iterator_exists;
	htreg->iterator_firstkey = htreg_iterator_firstkey;
	htreg->iterator_lastkey = htreg_iterator_lastkey;
	htreg->iterator_nextkey = htreg_iterator_nextkey;
	htreg->iterator_prevkey = htreg_iterator_prevkey;
}
