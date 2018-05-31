/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2016  Hercules Dev Team
 * Copyright (C)  Athena Dev Teams
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

/*****************************************************************************\
 *  This file is separated in two sections:                                  *
 *  (1) public typedefs, enums, unions, structures and defines               *
 *  (2) public functions                                                     *
 *                                                                           *
 *  <B>Notes on the release system:</B>                                      *
 *  Whenever an entry is removed from the database both the key and the      *
 *  data are requested to be released.                                       *
 *  At least one entry is removed when replacing an entry, removing an       *
 *  entry, clearing the database or destroying the database.                 *
 *  What is actually released is defined by the release function, the        *
 *  functions of the database only ask for the key and/or data to be         *
 *  released.                                                                *
 *                                                                           *
 *  TODO:                                                                    *
 *  - create a custom database allocator                                     *
 *  - see what functions need or should be added to the database interface   *
 *                                                                           *
 *  HISTORY:                                                                 *
 *    2013/08/25 - Added int64/uint64 support for keys                       *
 *    2012/03/09 - Added enum for data types (int, uint, void*)              *
 *    2007/11/09 - Added an iterator to the database.                        *
 *    2.1 (Athena build #???#) - Portability fix                             *
 *      - Fixed the portability of casting to union and added the functions  *
 *        struct DBMap#ensure() and struct DBMap#clear().                    *
 *    2.0 (Athena build 4859) - Transition version                           *
 *      - Almost everything recoded with a strategy similar to objects,      *
 *        database structure is maintained.                                  *
 *    1.0 (up to Athena build 4706)                                          *
 *      - Previous database system.                                          *
 *                                                                           *
 * @version 2.1 (Athena build #???#) - Portability fix                       *
 * @author (Athena build 4859) Flavio @ Amazon Project                       *
 * @author (up to Athena build 4706) Athena Dev Teams                        *
 * @encoding US-ASCII                                                        *
 * @see common#db.c                                                          *
\*****************************************************************************/
#ifndef COMMON_DB_H
#define COMMON_DB_H

#include "common/hercules.h"

#include <stdarg.h>

/*****************************************************************************
 *  (1) Section with public typedefs, enums, unions, structures and defines. *
 *  enum DBReleaseOption - Enumeration of release options.                   *
 *  enum DBType          - Enumeration of database types.                    *
 *  enum DBOptions       - Bitfield enumeration of database options.         *
 *  union DBKey          - Union of used key types.                          *
 *  enum DBDataType      - Enumeration of data types.                        *
 *  struct DBData        - Struct for used data types.                       *
 *  DBApply              - Format of functions applied to the databases.     *
 *  DBMatcher            - Format of matchers used in struct DBMap#getall(). *
 *  DBComparator         - Format of the comparators used by the databases.  *
 *  DBHasher             - Format of the hashers used by the databases.      *
 *  DBReleaser           - Format of the releasers used by the databases.    *
 *  struct DBIterator    - Database iterator.                                *
 *  struct DBMap         - Database interface.                               *
 *****************************************************************************/

/**
 * Bitfield with what should be released by the releaser function (if the
 * function supports it).
 * @public
 * @see #DBReleaser
 * @see #db_custom_release()
 */
enum DBReleaseOption {
	DB_RELEASE_NOTHING = 0x0,
	DB_RELEASE_KEY     = 0x1,
	DB_RELEASE_DATA    = 0x2,
	DB_RELEASE_BOTH    = DB_RELEASE_KEY|DB_RELEASE_DATA,
};

/**
 * Supported types of database.
 *
 * See #db_fix_options() for restrictions of the types of databases.
 *
 * @param DB_INT Uses int's for keys
 * @param DB_UINT Uses unsigned int's for keys
 * @param DB_STRING Uses strings for keys.
 * @param DB_ISTRING Uses case insensitive strings for keys.
 * @param DB_INT64 Uses int64's for keys
 * @param DB_UINT64 Uses uint64's for keys
 * @public
 * @see enum DBOptions
 * @see union DBKey
 * @see #db_fix_options()
 * @see #db_default_cmp()
 * @see #db_default_hash()
 * @see #db_default_release()
 * @see #db_alloc()
 */
enum DBType {
	DB_INT,
	DB_UINT,
	DB_STRING,
	DB_ISTRING,
	DB_INT64,
	DB_UINT64,
};

/**
 * Bitfield of options that define the behavior of the database.
 *
 * See #db_fix_options() for restrictions of the types of databases.
 *
 * @param DB_OPT_BASE Base options: does not duplicate keys, releases nothing
 *          and does not allow NULL keys or NULL data.
 * @param DB_OPT_DUP_KEY Duplicates the keys internally. If DB_OPT_RELEASE_KEY
 *          is defined, the real key is freed as soon as the entry is added.
 * @param DB_OPT_RELEASE_KEY Releases the key.
 * @param DB_OPT_RELEASE_DATA Releases the data whenever an entry is removed
 *          from the database.
 *          WARNING: for functions that return the data (like struct DBMap#remove()),
 *          a dangling pointer will be returned.
 * @param DB_OPT_RELEASE_BOTH Releases both key and data.
 * @param DB_OPT_ALLOW_NULL_KEY Allow NULL keys in the database.
 * @param DB_OPT_ALLOW_NULL_DATA Allow NULL data in the database.
 * @public
 * @see #db_fix_options()
 * @see #db_default_release()
 * @see #db_alloc()
 */
enum DBOptions {
	DB_OPT_BASE            = 0x00,
	DB_OPT_DUP_KEY         = 0x01,
	DB_OPT_RELEASE_KEY     = 0x02,
	DB_OPT_RELEASE_DATA    = 0x04,
	DB_OPT_RELEASE_BOTH    = DB_OPT_RELEASE_KEY|DB_OPT_RELEASE_DATA,
	DB_OPT_ALLOW_NULL_KEY  = 0x08,
	DB_OPT_ALLOW_NULL_DATA = 0x10,
};

/**
 * Union of key types used by the database.
 * @param i Type of key for DB_INT databases
 * @param ui Type of key for DB_UINT databases
 * @param str Type of key for DB_STRING and DB_ISTRING databases
 * @public
 * @see enum DBType
 * @see struct DBMap#get()
 * @see struct DBMap#put()
 * @see struct DBMap#remove()
 */
union DBKey {
	int i;
	unsigned int ui;
	const char *str;
	char *mutstr;
	int64 i64;
	uint64 ui64;
};

/**
 * Supported types of database data.
 * @param DB_DATA_INT Uses ints for data.
 * @param DB_DATA_UINT Uses unsigned ints for data.
 * @param DB_DATA_PTR Uses void pointers for data.
 * @public
 * @see struct DBData
 */
enum DBDataType {
	DB_DATA_INT,
	DB_DATA_UINT,
	DB_DATA_PTR,
};

/**
 * Struct for data types used by the database.
 * @param type Type of data
 * @param u Union of available data types
 * @param u.i Data of int type
 * @param u.ui Data of unsigned int type
 * @param u.ptr Data of void* type
 * @public
 */
struct DBData {
	enum DBDataType type;
	union {
		int i;
		unsigned int ui;
		void *ptr;
	} u;
};

/**
 * Format of functions that create the data for the key when the entry doesn't
 * exist in the database yet.
 * @param key Key of the database entry
 * @param args Extra arguments of the function
 * @return Data identified by the key to be put in the database
 * @public
 * @see struct DBMap#vensure()
 * @see struct DBMap#ensure()
 */
typedef struct DBData (*DBCreateData)(union DBKey key, va_list args);

/**
 * Format of functions to be applied to an unspecified quantity of entries of
 * a database.
 * Any function that applies this function to the database will return the sum
 * of values returned by this function.
 * @param key Key of the database entry
 * @param data Data of the database entry
 * @param args Extra arguments of the function
 * @return Value to be added up by the function that is applying this
 * @public
 * @see struct DBMap#vforeach()
 * @see struct DBMap#foreach()
 * @see struct DBMap#vdestroy()
 * @see struct DBMap#destroy()
 */
typedef int (*DBApply)(union DBKey key, struct DBData *data, va_list args);

/**
 * Format of functions that match database entries.
 * The purpose of the match depends on the function that is calling the matcher.
 * Returns 0 if it is a match, another number otherwise.
 * @param key Key of the database entry
 * @param data Data of the database entry
 * @param args Extra arguments of the function
 * @return 0 if a match, another number otherwise
 * @public
 * @see struct DBMap#getall()
 */
typedef int (*DBMatcher)(union DBKey key, struct DBData data, va_list args);

/**
 * Format of the comparators used internally by the database system.
 * Compares key1 to key2.
 * Returns 0 is equal, negative if lower and positive is higher.
 * @param key1 Key being compared
 * @param key2 Key we are comparing to
 * @param maxlen Maximum number of characters used in DB_STRING and DB_ISTRING
 *          databases.
 * @return 0 if equal, negative if lower and positive if higher
 * @public
 * @see #db_default_cmp()
 */
typedef int (*DBComparator)(union DBKey key1, union DBKey key2, unsigned short maxlen);

/**
 * Format of the hashers used internally by the database system.
 * Creates the hash of the key.
 * @param key Key being hashed
 * @param maxlen Maximum number of characters used in DB_STRING and DB_ISTRING
 *          databases.
 * @return Hash of the key
 * @public
 * @see #db_default_hash()
 */
typedef uint64 (*DBHasher)(union DBKey key, unsigned short maxlen);

/**
 * Format of the releaser used by the database system.
 * Releases nothing, the key, the data or both.
 * All standard releasers use aFree to release.
 * @param key Key of the database entry
 * @param data Data of the database entry
 * @param which What is being requested to be released
 * @public
 * @see enum DBReleaseOption
 * @see #db_default_releaser()
 * @see #db_custom_release()
 */
typedef void (*DBReleaser)(union DBKey key, struct DBData data, enum DBReleaseOption which);

/**
 * Database iterator.
 *
 * Supports forward iteration, backward iteration and removing entries from the database.
 * The iterator is initially positioned before the first entry of the database.
 *
 * While the iterator exists the database is locked internally, so invoke
 * struct DBIterator#destroy() as soon as possible.
 *
 * @public
 * @see struct DBMap
 */
struct DBIterator {
	/**
	 * Fetches the first entry in the database.
	 * Returns the data of the entry.
	 * Puts the key in out_key, if out_key is not NULL.
	 * @param self Iterator
	 * @param out_key Key of the entry
	 * @return Data of the entry
	 * @protected
	 */
	struct DBData *(*first)(struct DBIterator *self, union DBKey *out_key);

	/**
	 * Fetches the last entry in the database.
	 * Returns the data of the entry.
	 * Puts the key in out_key, if out_key is not NULL.
	 * @param self Iterator
	 * @param out_key Key of the entry
	 * @return Data of the entry
	 * @protected
	 */
	struct DBData *(*last)(struct DBIterator *self, union DBKey *out_key);

	/**
	 * Fetches the next entry in the database.
	 * Returns the data of the entry.
	 * Puts the key in out_key, if out_key is not NULL.
	 * @param self Iterator
	 * @param out_key Key of the entry
	 * @return Data of the entry
	 * @protected
	 */
	struct DBData *(*next)(struct DBIterator *self, union DBKey *out_key);

	/**
	 * Fetches the previous entry in the database.
	 * Returns the data of the entry.
	 * Puts the key in out_key, if out_key is not NULL.
	 * @param self Iterator
	 * @param out_key Key of the entry
	 * @return Data of the entry
	 * @protected
	 */
	struct DBData *(*prev)(struct DBIterator *self, union DBKey *out_key);

	/**
	 * Returns true if the fetched entry exists.
	 * The databases entries might have NULL data, so use this to to test if
	 * the iterator is done.
	 * @param self Iterator
	 * @return true is the entry exists
	 * @protected
	 */
	bool (*exists)(struct DBIterator *self);

	/**
	 * Removes the current entry from the database.
	 *
	 * NOTE: struct DBIterator#exists() will return false until another
	 * entry is fetched.
	 *
	 * Puts data of the removed entry in out_data, if out_data is not NULL.
	 * @param self Iterator
	 * @param out_data Data of the removed entry.
	 * @return 1 if entry was removed, 0 otherwise
	 * @protected
	 * @see struct DBMap#remove()
	 */
	int (*remove)(struct DBIterator *self, struct DBData *out_data);

	/**
	 * Destroys this iterator and unlocks the database.
	 * @param self Iterator
	 * @protected
	 */
	void (*destroy)(struct DBIterator *self);

};

/**
 * Public interface of a database. Only contains functions.
 * All the functions take the interface as the first argument.
 * @public
 * @see #db_alloc()
 */
struct DBMap {

	/**
	 * Returns a new iterator for this database.
	 * The iterator keeps the database locked until it is destroyed.
	 * The database will keep functioning normally but will only free internal
	 * memory when unlocked, so destroy the iterator as soon as possible.
	 * @param self Database
	 * @return New iterator
	 * @protected
	 */
	struct DBIterator *(*iterator)(struct DBMap *self);

	/**
	 * Returns true if the entry exists.
	 * @param self Database
	 * @param key Key that identifies the entry
	 * @return true is the entry exists
	 * @protected
	 */
	bool (*exists)(struct DBMap *self, union DBKey key);

	/**
	 * Get the data of the entry identified by the key.
	 * @param self Database
	 * @param key Key that identifies the entry
	 * @return Data of the entry or NULL if not found
	 * @protected
	 */
	struct DBData *(*get)(struct DBMap *self, union DBKey key);

	/**
	 * Just calls struct DBMap#vgetall().
	 *
	 * Get the data of the entries matched by <code>match</code>.
	 * It puts a maximum of <code>max</code> entries into <code>buf</code>.
	 * If <code>buf</code> is NULL, it only counts the matches.
	 * Returns the number of entries that matched.
	 * NOTE: if the value returned is greater than <code>max</code>, only the
	 * first <code>max</code> entries found are put into the buffer.
	 * @param self Database
	 * @param buf Buffer to put the data of the matched entries
	 * @param max Maximum number of data entries to be put into buf
	 * @param match Function that matches the database entries
	 * @param ... Extra arguments for match
	 * @return The number of entries that matched
	 * @protected
	 * @see struct DBMap#vgetall()
	 */
	unsigned int (*getall)(struct DBMap *self, struct DBData **buf, unsigned int max, DBMatcher match, ...);

	/**
	 * Get the data of the entries matched by <code>match</code>.
	 * It puts a maximum of <code>max</code> entries into <code>buf</code>.
	 * If <code>buf</code> is NULL, it only counts the matches.
	 * Returns the number of entries that matched.
	 * NOTE: if the value returned is greater than <code>max</code>, only the
	 * first <code>max</code> entries found are put into the buffer.
	 * @param self Database
	 * @param buf Buffer to put the data of the matched entries
	 * @param max Maximum number of data entries to be put into buf
	 * @param match Function that matches the database entries
	 * @param ... Extra arguments for match
	 * @return The number of entries that matched
	 * @protected
	 * @see struct DBMap#getall()
	 */
	unsigned int (*vgetall)(struct DBMap *self, struct DBData **buf, unsigned int max, DBMatcher match, va_list args);

	/**
	 * Just calls struct DBMap#vensure().
	 *
	 * Get the data of the entry identified by the key.  If the entry does
	 * not exist, an entry is added with the data returned by `create`.
	 *
	 * @param self Database
	 * @param key Key that identifies the entry
	 * @param create Function used to create the data if the entry doesn't exist
	 * @param ... Extra arguments for create
	 * @return Data of the entry
	 * @protected
	 * @see struct DBMap#vensure()
	 */
	struct DBData *(*ensure)(struct DBMap *self, union DBKey key, DBCreateData create, ...);

	/**
	 * Get the data of the entry identified by the key.
	 * If the entry does not exist, an entry is added with the data returned by
	 * <code>create</code>.
	 * @param self Database
	 * @param key Key that identifies the entry
	 * @param create Function used to create the data if the entry doesn't exist
	 * @param args Extra arguments for create
	 * @return Data of the entry
	 * @protected
	 * @see struct DBMap#ensure()
	 */
	struct DBData *(*vensure)(struct DBMap *self, union DBKey key, DBCreateData create, va_list args);

	/**
	 * Put the data identified by the key in the database.
	 * Puts the previous data in out_data, if out_data is not NULL.
	 * NOTE: Uses the new key, the old one is released.
	 * @param self Database
	 * @param key Key that identifies the data
	 * @param data Data to be put in the database
	 * @param out_data Previous data if the entry exists
	 * @return 1 if if the entry already exists, 0 otherwise
	 * @protected
	 */
	int (*put)(struct DBMap *self, union DBKey key, struct DBData data, struct DBData *out_data);

	/**
	 * Remove an entry from the database.
	 * Puts the previous data in out_data, if out_data is not NULL.
	 * NOTE: The key (of the database) is released.
	 * @param self Database
	 * @param key Key that identifies the entry
	 * @param out_data Previous data if the entry exists
	 * @return 1 if if the entry already exists, 0 otherwise
	 * @protected
	 */
	int (*remove)(struct DBMap *self, union DBKey key, struct DBData *out_data);

	/**
	 * Just calls struct DBMap#vforeach().
	 *
	 * Apply <code>func</code> to every entry in the database.
	 * Returns the sum of values returned by func.
	 * @param self Database
	 * @param func Function to be applied
	 * @param ... Extra arguments for func
	 * @return Sum of the values returned by func
	 * @protected
	 * @see struct DBMap#vforeach()
	 */
	int (*foreach)(struct DBMap *self, DBApply func, ...);

	/**
	 * Apply <code>func</code> to every entry in the database.
	 * Returns the sum of values returned by func.
	 * @param self Database
	 * @param func Function to be applied
	 * @param args Extra arguments for func
	 * @return Sum of the values returned by func
	 * @protected
	 * @see struct DBMap#foreach()
	 */
	int (*vforeach)(struct DBMap *self, DBApply func, va_list args);

	/**
	 * Just calls struct DBMap#vclear().
	 *
	 * Removes all entries from the database.
	 * Before deleting an entry, func is applied to it.
	 * Releases the key and the data.
	 * Returns the sum of values returned by func, if it exists.
	 * @param self Database
	 * @param func Function to be applied to every entry before deleting
	 * @param ... Extra arguments for func
	 * @return Sum of values returned by func
	 * @protected
	 * @see struct DBMap#vclear()
	 */
	int (*clear)(struct DBMap *self, DBApply func, ...);

	/**
	 * Removes all entries from the database.
	 * Before deleting an entry, func is applied to it.
	 * Releases the key and the data.
	 * Returns the sum of values returned by func, if it exists.
	 * @param self Database
	 * @param func Function to be applied to every entry before deleting
	 * @param args Extra arguments for func
	 * @return Sum of values returned by func
	 * @protected
	 * @see struct DBMap#clear()
	 */
	int (*vclear)(struct DBMap *self, DBApply func, va_list args);

	/**
	 * Just calls DBMap#vdestroy().
	 * Finalize the database, feeing all the memory it uses.
	 * Before deleting an entry, func is applied to it.
	 * Releases the key and the data.
	 * Returns the sum of values returned by func, if it exists.
	 * NOTE: This locks the database globally. Any attempt to insert or remove
	 * a database entry will give an error and be aborted (except for clearing).
	 * @param self Database
	 * @param func Function to be applied to every entry before deleting
	 * @param ... Extra arguments for func
	 * @return Sum of values returned by func
	 * @protected
	 * @see struct DBMap#vdestroy()
	 */
	int (*destroy)(struct DBMap *self, DBApply func, ...);

	/**
	 * Finalize the database, feeing all the memory it uses.
	 * Before deleting an entry, func is applied to it.
	 * Returns the sum of values returned by func, if it exists.
	 * NOTE: This locks the database globally. Any attempt to insert or remove
	 * a database entry will give an error and be aborted (except for clearing).
	 * @param self Database
	 * @param func Function to be applied to every entry before deleting
	 * @param args Extra arguments for func
	 * @return Sum of values returned by func
	 * @protected
	 * @see struct DBMap#destroy()
	 */
	int (*vdestroy)(struct DBMap *self, DBApply func, va_list args);

	/**
	 * Return the size of the database (number of items in the database).
	 * @param self Database
	 * @return Size of the database
	 * @protected
	 */
	unsigned int (*size)(struct DBMap *self);

	/**
	 * Return the type of the database.
	 * @param self Database
	 * @return Type of the database
	 * @protected
	 */
	enum DBType (*type)(struct DBMap *self);

	/**
	 * Return the options of the database.
	 * @param self Database
	 * @return Options of the database
	 * @protected
	 */
	enum DBOptions (*options)(struct DBMap *self);

};

// For easy access to the common functions.

#define db_exists(db,k)     ( (db)->exists((db),(k)) )
#define idb_exists(db,k)    ( (db)->exists((db),DB->i2key(k)) )
#define uidb_exists(db,k)   ( (db)->exists((db),DB->ui2key(k)) )
#define strdb_exists(db,k)  ( (db)->exists((db),DB->str2key(k)) )
#define i64db_exists(db,k)  ( (db)->exists((db),DB->i642key(k)) )
#define ui64db_exists(db,k) ( (db)->exists((db),DB->ui642key(k)) )

// Get pointer-type data from DBMaps of various key types
#define db_get(db,k)     ( DB->data2ptr((db)->get((db),(k))) )
#define idb_get(db,k)    ( DB->data2ptr((db)->get((db),DB->i2key(k))) )
#define uidb_get(db,k)   ( DB->data2ptr((db)->get((db),DB->ui2key(k))) )
#define strdb_get(db,k)  ( DB->data2ptr((db)->get((db),DB->str2key(k))) )
#define i64db_get(db,k)  ( DB->data2ptr((db)->get((db),DB->i642key(k))) )
#define ui64db_get(db,k) ( DB->data2ptr((db)->get((db),DB->ui642key(k))) )

// Get int-type data from DBMaps of various key types
#define db_iget(db,k)     ( DB->data2i((db)->get((db),(k))) )
#define idb_iget(db,k)    ( DB->data2i((db)->get((db),DB->i2key(k))) )
#define uidb_iget(db,k)   ( DB->data2i((db)->get((db),DB->ui2key(k))) )
#define strdb_iget(db,k)  ( DB->data2i((db)->get((db),DB->str2key(k))) )
#define i64db_iget(db,k)  ( DB->data2i((db)->get((db),DB->i642key(k))) )
#define ui64db_iget(db,k) ( DB->data2i((db)->get((db),DB->ui642key(k))) )

// Get uint-type data from DBMaps of various key types
#define db_uiget(db,k)     ( DB->data2ui((db)->get((db),(k))) )
#define idb_uiget(db,k)    ( DB->data2ui((db)->get((db),DB->i2key(k))) )
#define uidb_uiget(db,k)   ( DB->data2ui((db)->get((db),DB->ui2key(k))) )
#define strdb_uiget(db,k)  ( DB->data2ui((db)->get((db),DB->str2key(k))) )
#define i64db_uiget(db,k)  ( DB->data2ui((db)->get((db),DB->i642key(k))) )
#define ui64db_uiget(db,k) ( DB->data2ui((db)->get((db),DB->ui642key(k))) )

// Put pointer-type data into DBMaps of various key types
#define db_put(db,k,d)     ( (db)->put((db),(k),DB->ptr2data(d),NULL) )
#define idb_put(db,k,d)    ( (db)->put((db),DB->i2key(k),DB->ptr2data(d),NULL) )
#define uidb_put(db,k,d)   ( (db)->put((db),DB->ui2key(k),DB->ptr2data(d),NULL) )
#define strdb_put(db,k,d)  ( (db)->put((db),DB->str2key(k),DB->ptr2data(d),NULL) )
#define i64db_put(db,k,d)  ( (db)->put((db),DB->i642key(k),DB->ptr2data(d),NULL) )
#define ui64db_put(db,k,d) ( (db)->put((db),DB->ui642key(k),DB->ptr2data(d),NULL) )

// Put int-type data into DBMaps of various key types
#define db_iput(db,k,d)     ( (db)->put((db),(k),DB->i2data(d),NULL) )
#define idb_iput(db,k,d)    ( (db)->put((db),DB->i2key(k),DB->i2data(d),NULL) )
#define uidb_iput(db,k,d)   ( (db)->put((db),DB->ui2key(k),DB->i2data(d),NULL) )
#define strdb_iput(db,k,d)  ( (db)->put((db),DB->str2key(k),DB->i2data(d),NULL) )
#define i64db_iput(db,k,d)  ( (db)->put((db),DB->i642key(k),DB->i2data(d),NULL) )
#define ui64db_iput(db,k,d) ( (db)->put((db),DB->ui642key(k),DB->i2data(d),NULL) )

// Put uint-type data into DBMaps of various key types
#define db_uiput(db,k,d)     ( (db)->put((db),(k),DB->ui2data(d),NULL) )
#define idb_uiput(db,k,d)    ( (db)->put((db),DB->i2key(k),DB->ui2data(d),NULL) )
#define uidb_uiput(db,k,d)   ( (db)->put((db),DB->ui2key(k),DB->ui2data(d),NULL) )
#define strdb_uiput(db,k,d)  ( (db)->put((db),DB->str2key(k),DB->ui2data(d),NULL) )
#define i64db_uiput(db,k,d)  ( (db)->put((db),DB->i642key(k),DB->ui2data(d),NULL) )
#define ui64db_uiput(db,k,d) ( (db)->put((db),DB->ui642key(k),DB->ui2data(d),NULL) )

// Remove entry from DBMaps of various key types
#define db_remove(db,k)     ( (db)->remove((db),(k),NULL) )
#define idb_remove(db,k)    ( (db)->remove((db),DB->i2key(k),NULL) )
#define uidb_remove(db,k)   ( (db)->remove((db),DB->ui2key(k),NULL) )
#define strdb_remove(db,k)  ( (db)->remove((db),DB->str2key(k),NULL) )
#define i64db_remove(db,k)  ( (db)->remove((db),DB->i642key(k),NULL) )
#define ui64db_remove(db,k) ( (db)->remove((db),DB->ui642key(k),NULL) )

//These are discarding the possible vargs you could send to the function, so those
//that require vargs must not use these defines.
#define db_ensure(db,k,f)     ( DB->data2ptr((db)->ensure((db),(k),(f))) )
#define idb_ensure(db,k,f)    ( DB->data2ptr((db)->ensure((db),DB->i2key(k),(f))) )
#define uidb_ensure(db,k,f)   ( DB->data2ptr((db)->ensure((db),DB->ui2key(k),(f))) )
#define strdb_ensure(db,k,f)  ( DB->data2ptr((db)->ensure((db),DB->str2key(k),(f))) )
#define i64db_ensure(db,k,f)  ( DB->data2ptr((db)->ensure((db),DB->i642key(k),(f))) )
#define ui64db_ensure(db,k,f) ( DB->data2ptr((db)->ensure((db),DB->ui642key(k),(f))) )

// Database creation and destruction macros
#define idb_alloc(opt)            DB->alloc(__FILE__,__func__,__LINE__,DB_INT,(opt),sizeof(int))
#define uidb_alloc(opt)           DB->alloc(__FILE__,__func__,__LINE__,DB_UINT,(opt),sizeof(unsigned int))
#define strdb_alloc(opt,maxlen)   DB->alloc(__FILE__,__func__,__LINE__,DB_STRING,(opt),(maxlen))
#define stridb_alloc(opt,maxlen)  DB->alloc(__FILE__,__func__,__LINE__,DB_ISTRING,(opt),(maxlen))
#define i64db_alloc(opt)          DB->alloc(__FILE__,__func__,__LINE__,DB_INT64,(opt),sizeof(int64))
#define ui64db_alloc(opt)         DB->alloc(__FILE__,__func__,__LINE__,DB_UINT64,(opt),sizeof(uint64))
#define db_destroy(db)            ( (db)->destroy((db),NULL) )
// Other macros
#define db_clear(db)        ( (db)->clear((db),NULL) )
#define db_size(db)         ( (db)->size(db) )
#define db_iterator(db)     ( (db)->iterator(db) )
#define dbi_first(dbi)      ( DB->data2ptr((dbi)->first((dbi),NULL)) )
#define dbi_last(dbi)       ( DB->data2ptr((dbi)->last((dbi),NULL)) )
#define dbi_next(dbi)       ( DB->data2ptr((dbi)->next((dbi),NULL)) )
#define dbi_prev(dbi)       ( DB->data2ptr((dbi)->prev((dbi),NULL)) )
#define dbi_remove(dbi)     ( (dbi)->remove((dbi),NULL) )
#define dbi_exists(dbi)     ( (dbi)->exists(dbi) )
#define dbi_destroy(dbi)    ( (dbi)->destroy(dbi) )

/*****************************************************************************
 *  (2) Section with public functions.                                       *
 *  db_fix_options     - Fix the options for a type of database.             *
 *  db_default_cmp     - Get the default comparator for a type of database.  *
 *  db_default_hash    - Get the default hasher for a type of database.      *
 *  db_default_release - Get the default releaser for a type of database     *
 *           with the fixed options.                                         *
 *  db_custom_release  - Get the releaser that behaves as specified.         *
 *  db_alloc           - Allocate a new database.                            *
 *  db_i2key           - Manual cast from `int` to `union DBKey`.            *
 *  db_ui2key          - Manual cast from `unsigned int` to `union DBKey`.   *
 *  db_str2key         - Manual cast from `unsigned char *` to `union DBKey`.*
 *  db_i642key         - Manual cast from `int64` to `union DBKey`.          *
 *  db_ui642key        - Manual cast from `uint64` to `union DBKey`.         *
 *  db_i2data          - Manual cast from `int` to `struct DBData`.          *
 *  db_ui2data         - Manual cast from `unsigned int` to `struct DBData`. *
 *  db_ptr2data        - Manual cast from `void*` to `struct DBData`.        *
 *  db_data2i          - Gets `int` value from `struct DBData`.              *
 *  db_data2ui         - Gets `unsigned int` value from `struct DBData`.     *
 *  db_data2ptr        - Gets `void*` value from `struct DBData`.            *
 *  db_init            - Initializes the database system.                    *
 *  db_final           - Finalizes the database system.                      *
 *****************************************************************************/

struct db_interface {
/**
 * Returns the fixed options according to the database type.
 * Sets required options and unsets unsupported options.
 * For numeric databases DB_OPT_DUP_KEY and DB_OPT_RELEASE_KEY are unset.
 * @param type Type of the database
 * @param options Original options of the database
 * @return Fixed options of the database
 * @private
 * @see enum DBType
 * @see enum DBOptions
 * @see #db_default_release()
 */
enum DBOptions (*fix_options) (enum DBType type, enum DBOptions options);

/**
 * Returns the default comparator for the type of database.
 * @param type Type of database
 * @return Comparator for the type of database or NULL if unknown database
 * @public
 * @see enum DBType
 * @see #DBComparator
 */
DBComparator (*default_cmp) (enum DBType type);

/**
 * Returns the default hasher for the specified type of database.
 * @param type Type of database
 * @return Hasher of the type of database or NULL if unknown database
 * @public
 * @see enum DBType
 * @see #DBHasher
 */
DBHasher (*default_hash) (enum DBType type);

/**
 * Returns the default releaser for the specified type of database with the
 * specified options.
 *
 * NOTE: the options are fixed by #db_fix_options() before choosing the
 * releaser.
 *
 * @param type Type of database
 * @param options Options of the database
 * @return Default releaser for the type of database with the fixed options
 * @public
 * @see enum DBType
 * @see enum DBOptions
 * @see #DBReleaser
 * @see #db_fix_options()
 * @see #db_custom_release()
 */
DBReleaser (*default_release) (enum DBType type, enum DBOptions options);

/**
 * Returns the releaser that behaves as <code>which</code> specifies.
 * @param which Defines what the releaser releases
 * @return Releaser for the specified release options
 * @public
 * @see enum DBReleaseOption
 * @see #DBReleaser
 * @see #db_default_release()
 */
DBReleaser (*custom_release)  (enum DBReleaseOption which);

/**
 * Allocate a new database of the specified type.
 *
 * It uses the default comparator, hasher and releaser of the specified
 * database type and fixed options.
 *
 * NOTE: the options are fixed by #db_fix_options() before creating the
 * database.
 *
 * @param file File where the database is being allocated
 * @param line Line of the file where the database is being allocated
 * @param type Type of database
 * @param options Options of the database
 * @param maxlen Maximum length of the string to be used as key in string
 *          databases. If 0, the maximum number of maxlen is used (64K).
 * @return The interface of the database
 * @public
 * @see enum DBType
 * @see struct DBMap
 * @see #db_default_cmp()
 * @see #db_default_hash()
 * @see #db_default_release()
 * @see #db_fix_options()
 */
struct DBMap *(*alloc) (const char *file, const char *func, int line, enum DBType type, enum DBOptions options, unsigned short maxlen);

/**
 * Manual cast from 'int' to the union DBKey.
 * @param key Key to be casted
 * @return The key as a DBKey union
 * @public
 */
union DBKey (*i2key) (int key);

/**
 * Manual cast from 'unsigned int' to the union DBKey.
 * @param key Key to be casted
 * @return The key as a DBKey union
 * @public
 */
union DBKey (*ui2key) (unsigned int key);

/**
 * Manual cast from 'unsigned char *' to the union DBKey.
 * @param key Key to be casted
 * @return The key as a DBKey union
 * @public
 */
union DBKey (*str2key) (const char *key);

/**
 * Manual cast from 'int64' to the union DBKey.
 * @param key Key to be casted
 * @return The key as a DBKey union
 * @public
 */
union DBKey (*i642key) (int64 key);

/**
 * Manual cast from 'uint64' to the union DBKey.
 * @param key Key to be casted
 * @return The key as a DBKey union
 * @public
 */
union DBKey (*ui642key) (uint64 key);

/**
 * Manual cast from 'int' to the struct DBData.
 * @param data Data to be casted
 * @return The data as a DBData struct
 * @public
 */
struct DBData (*i2data) (int data);

/**
 * Manual cast from 'unsigned int' to the struct DBData.
 * @param data Data to be casted
 * @return The data as a DBData struct
 * @public
 */
struct DBData (*ui2data) (unsigned int data);

/**
 * Manual cast from 'void *' to the struct DBData.
 * @param data Data to be casted
 * @return The data as a DBData struct
 * @public
 */
struct DBData (*ptr2data) (void *data);

/**
 * Gets int type data from struct DBData.
 * If data is not int type, returns 0.
 * @param data Data
 * @return Integer value of the data.
 * @public
 */
int (*data2i) (struct DBData *data);

/**
 * Gets unsigned int type data from struct DBData.
 * If data is not unsigned int type, returns 0.
 * @param data Data
 * @return Unsigned int value of the data.
 * @public
 */
unsigned int (*data2ui) (struct DBData *data);

/**
 * Gets void* type data from struct DBData.
 * If data is not void* type, returns NULL.
 * @param data Data
 * @return Void* value of the data.
 * @public
 */
void* (*data2ptr) (struct DBData *data);

/**
 * Initialize the database system.
 * @public
 * @see #db_final(void)
 */
void (*init) (void);

/**
 * Finalize the database system.
 * Frees the memory used by the block reusage system.
 * @public
 * @see #db_init(void)
 */
void (*final) (void);
};

// Link DB System - From jAthena
struct linkdb_node {
	struct linkdb_node *next;
	struct linkdb_node *prev;
	void               *key;
	void               *data;
};

typedef void (*LinkDBFunc)(void* key, void* data, va_list args);

#ifdef HERCULES_CORE
void  linkdb_insert  (struct linkdb_node** head, void *key, void* data); // Doesn't take into account duplicate keys
void  linkdb_replace (struct linkdb_node** head, void *key, void* data); // Takes into account duplicate keys
void* linkdb_search  (struct linkdb_node** head, void *key);
void* linkdb_erase   (struct linkdb_node** head, void *key);
void  linkdb_final   (struct linkdb_node** head);
void  linkdb_vforeach(struct linkdb_node** head, LinkDBFunc func, va_list ap);
void  linkdb_foreach (struct linkdb_node** head, LinkDBFunc func, ...);

void db_defaults(void);
#endif // HERCULES_CORE

HPShared struct db_interface *DB;

/**
 * Array Helper macros
 */

/**
 * Finds an entry in an array.
 *
 * @code
 *    ARR_FIND(0, size, i, list[i] == target);
 * @endcode
 *
 * To differentiate between the found and not found cases, the caller code can
 * compare _end and _var after this macro returns.
 *
 * @param _start Starting index (ex: 0).
 * @param _end   End index (ex: size of the array).
 * @param _var   Index variable.
 * @param _cmp   Search expression (should return true when the target entry is found).
 */
#define ARR_FIND(_start, _end, _var, _cmp) \
	do { \
		for ((_var) = (_start); (_var) < (_end); ++(_var)) \
			if (_cmp) \
				break; \
	} while(false)

/**
 * Moves an entry of the array.
 *
 * @code
 *    ARR_MOVE(i, 0, list, int); // move index i to index 0
 * @endcode
 *
 * @remark
 *    Use ARR_MOVERIGHT/ARR_MOVELEFT if _from and _to are direct numbers.
 *
 * @param _from Initial index of the entry.
 * @param _to   Target index of the entry.
 * @param _arr  Array.
 * @param _type Type of entry.
 */
#define ARR_MOVE(_from, _to, _arr, _type) \
	do { \
		if ((_from) != (_to)) { \
			_type _backup_; \
			memmove(&_backup_, (_arr)+(_from), sizeof(_type)); \
			if ((_from) < (_to)) \
				memmove((_arr)+(_from), (_arr)+(_from)+1, ((_to)-(_from))*sizeof(_type)); \
			else if ((_from) > (_to)) \
				memmove((_arr)+(_to)+1, (_arr)+(_to), ((_from)-(_to))*sizeof(_type)); \
			memmove((_arr)+(_to), &_backup_, sizeof(_type)); \
		} \
	} while(false)

/**
 * Moves an entry of the array to the right.
 *
 * @code
 *    ARR_MOVERIGHT(1, 4, list, int); // move index 1 to index 4
 * @endcode
 *
 * @param _from Initial index of the entry.
 * @param _to   Target index of the entry.
 * @param _arr  Array.
 * @param _type Type of entry.
 */
#define ARR_MOVERIGHT(_from, _to, _arr, _type) \
	do { \
		_type _backup_; \
		memmove(&_backup_, (_arr)+(_from), sizeof(_type)); \
		memmove((_arr)+(_from), (_arr)+(_from)+1, ((_to)-(_from))*sizeof(_type)); \
		memmove((_arr)+(_to), &_backup_, sizeof(_type)); \
	} while(false)

/**
 * Moves an entry of the array to the left.
 *
 * @code
 *    ARR_MOVELEFT(3, 0, list, int); // move index 3 to index 0
 * @endcode
 *
 * @param _from Initial index of the entry.
 * @param _end  Target index of the entry.
 * @param _arr  Array.
 * @param _type Type of entry.
 */
#define ARR_MOVELEFT(_from, _to, _arr, _type) \
	do { \
		_type _backup_; \
		memmove(&_backup_, (_arr)+(_from), sizeof(_type)); \
		memmove((_arr)+(_to)+1, (_arr)+(_to), ((_from)-(_to))*sizeof(_type)); \
		memmove((_arr)+(_to), &_backup_, sizeof(_type)); \
	} while(false)

/**
 * Vector library based on defines (dynamic array).
 *
 * @remark
 *    This library uses aMalloc, aRealloc, aFree.
 */

/**
 * Declares an anonymous vector struct.
 *
 * @param _type Type of data to be contained.
 */
#define VECTOR_DECL(_type) \
	struct { \
		int _max_; \
		int _len_; \
		_type *_data_; \
	}

/**
 * Declares a named vector struct.
 *
 * @param _name Structure name.
 * @param _type Type of data to be contained.
 */
#define VECTOR_STRUCT_DECL(_name, _type) \
	struct _name { \
		int _max_; \
		int _len_; \
		_type *_data_; \
	}

/**
 * Declares and initializes an anonymous vector variable.
 *
 * @param _type Type of data to be contained.
 * @param _var  Variable name.
 */
#define VECTOR_VAR(_type, _var) \
	VECTOR_DECL(_type) _var = {0, 0, NULL}

/**
 * Declares and initializes a named vector variable.
 *
 * @param _name Structure name.
 * @param _var  Variable name.
 */
#define VECTOR_STRUCT_VAR(_name, _var) \
	struct _name _var = {0, 0, NULL}

/**
 * Initializes a vector.
 *
 * @param _vec Vector.
 */
#define VECTOR_INIT(_vec) \
	do { \
		VECTOR_DATA(_vec) = NULL; \
		VECTOR_CAPACITY(_vec) = 0; \
		VECTOR_LENGTH(_vec) = 0; \
	} while(false)

/**
 * Returns the internal array of values.
 *
 * @param _vec Vector.
 * @return Internal array of values.
 */
#define VECTOR_DATA(_vec) \
	( (_vec)._data_ )

/**
 * Returns the length of the vector (number of elements in use).
 *
 * @param _vec Vector
 * @return Length
 */
#define VECTOR_LENGTH(_vec) \
	( (_vec)._len_ )

/**
 * Returns the capacity of the vector (number of elements allocated).
 *
 * @param _vec Vector.
 * @return Capacity.
 */
#define VECTOR_CAPACITY(_vec) \
	( (_vec)._max_ )

/**
 * Returns the value at the target index.
 *
 * Assumes the index exists.
 *
 * @param _vec Vector.
 * @param _idx Index.
 * @return Value.
 */
#define VECTOR_INDEX(_vec, _idx) \
	( VECTOR_DATA(_vec)[_idx] )

/**
 * Returns the first value of the vector.
 *
 * Assumes the array is not empty.
 *
 * @param _vec Vector.
 * @return First value.
 */
#define VECTOR_FIRST(_vec) \
	( VECTOR_INDEX(_vec, 0) )

/**
 * Returns the last value of the vector.
 *
 * Assumes the array is not empty.
 *
 * @param _vec Vector.
 * @return Last value.
 */
#define VECTOR_LAST(_vec) \
	( VECTOR_INDEX(_vec, VECTOR_LENGTH(_vec)-1) )

/**
 * Resizes the vector.
 *
 * Excess values are discarded, new positions are zeroed.
 *
 * @param _vec Vector.
 * @param _n   New size.
 */
#define VECTOR_RESIZE(_vec, _n) \
	do { \
		if ((_n) > VECTOR_CAPACITY(_vec)) { \
			/* increase size */ \
			if (VECTOR_CAPACITY(_vec) == 0) \
				VECTOR_DATA(_vec) = aMalloc((_n)*sizeof(VECTOR_FIRST(_vec))); /* allocate new */ \
			else \
				VECTOR_DATA(_vec) = aRealloc(VECTOR_DATA(_vec), (_n)*sizeof(VECTOR_FIRST(_vec))); /* reallocate */ \
			memset(VECTOR_DATA(_vec)+VECTOR_LENGTH(_vec), 0, (VECTOR_CAPACITY(_vec)-VECTOR_LENGTH(_vec))*sizeof(VECTOR_FIRST(_vec))); /* clear new data */ \
			VECTOR_CAPACITY(_vec) = (_n); /* update capacity */ \
		} else if ((_n) == 0 && VECTOR_CAPACITY(_vec) > 0) { \
			/* clear vector */ \
			aFree(VECTOR_DATA(_vec)); VECTOR_DATA(_vec) = NULL; /* free data */ \
			VECTOR_CAPACITY(_vec) = 0; /* clear capacity */ \
			VECTOR_LENGTH(_vec) = 0; /* clear length */ \
		} else if ((_n) < VECTOR_CAPACITY(_vec)) { \
			/* reduce size */ \
			VECTOR_DATA(_vec) = aRealloc(VECTOR_DATA(_vec), (_n)*sizeof(VECTOR_FIRST(_vec))); /* reallocate */ \
			VECTOR_CAPACITY(_vec) = (_n); /* update capacity */ \
			if ((_n) - VECTOR_LENGTH(_vec) > 0) \
				VECTOR_LENGTH(_vec) = (_n); /* update length */ \
		} \
	} while(false)

/**
 * Ensures that the array has the target number of empty positions.
 *
 * Increases the capacity in multiples of _step.
 *
 * @param _vec  Vector.
 * @param _n    Desired empty positions.
 * @param _step Increase.
 */
#define VECTOR_ENSURE(_vec, _n, _step) \
	do { \
		int _newcapacity_ = VECTOR_CAPACITY(_vec); \
		while ((_n) + VECTOR_LENGTH(_vec) > _newcapacity_) \
			_newcapacity_ += (_step); \
		if (_newcapacity_ > VECTOR_CAPACITY(_vec)) \
			VECTOR_RESIZE(_vec, _newcapacity_); \
	} while(false)

/**
 * Inserts a zeroed value in the target index.
 *
 * Assumes the index is valid and there is enough capacity.
 *
 * @param _vec Vector.
 * @param _idx Index.
 */
#define VECTOR_INSERTZEROED(_vec, _idx) \
	do { \
		if ((_idx) < VECTOR_LENGTH(_vec)) /* move data */ \
			memmove(&VECTOR_INDEX(_vec, (_idx)+1), &VECTOR_INDEX(_vec, _idx), (VECTOR_LENGTH(_vec)-(_idx))*sizeof(VECTOR_FIRST(_vec))); \
		memset(&VECTOR_INDEX(_vec, _idx), 0, sizeof(VECTOR_INDEX(_vec, _idx))); /* set zeroed value */ \
		++VECTOR_LENGTH(_vec); /* increase length */ \
	} while(false)

/**
 * Inserts a value in the target index (using the '=' operator).
 *
 * Assumes the index is valid and there is enough capacity.
 *
 * @param _vec Vector.
 * @param _idx Index.
 * @param _val Value.
 */
#define VECTOR_INSERT(_vec, _idx, _val) \
	do { \
		if ((_idx) < VECTOR_LENGTH(_vec)) /* move data */ \
			memmove(&VECTOR_INDEX(_vec, (_idx)+1), &VECTOR_INDEX(_vec, _idx), (VECTOR_LENGTH(_vec)-(_idx))*sizeof(VECTOR_FIRST(_vec))); \
		VECTOR_INDEX(_vec, _idx) = (_val); /* set value */ \
		++VECTOR_LENGTH(_vec); /* increase length */ \
	} while(false)

/**
 * Inserts a value in the target index (using memcpy).
 *
 * Assumes the index is valid and there is enough capacity.
 *
 * @param _vec Vector.
 * @param _idx Index.
 * @param _val Value.
 */
#define VECTOR_INSERTCOPY(_vec, _idx, _val) \
	VECTOR_INSERTARRAY(_vec, _idx, &(_val), 1)

/**
 * Inserts the values of the array in the target index (using memcpy).
 *
 * Assumes the index is valid and there is enough capacity.
 *
 * @param _vec  Vector.
 * @param _idx  Index.
 * @param _pval Array of values.
 * @param _n    Number of values.
 */
#define VECTOR_INSERTARRAY(_vec, _idx, _pval, _n) \
	do { \
		if ((_idx) < VECTOR_LENGTH(_vec)) /* move data */ \
			memmove(&VECTOR_INDEX(_vec, (_idx)+(_n)), &VECTOR_INDEX(_vec, _idx), (VECTOR_LENGTH(_vec)-(_idx))*sizeof(VECTOR_FIRST(_vec))); \
		memcpy(&VECTOR_INDEX(_vec, _idx), (_pval), (_n)*sizeof(VECTOR_FIRST(_vec))); /* set values */ \
		VECTOR_LENGTH(_vec) += (_n); /* increase length */ \
	} while(false)

/**
 * Inserts a zeroed value in the end of the vector.
 *
 * Assumes there is enough capacity.
 *
 * @param _vec Vector.
 */
#define VECTOR_PUSHZEROED(_vec) \
	do { \
		memset(&VECTOR_INDEX(_vec, VECTOR_LENGTH(_vec)), 0, sizeof(VECTOR_INDEX(_vec, VECTOR_LENGTH(_vec)))); /* set zeroed value */ \
		++VECTOR_LENGTH(_vec); /* increase length */ \
	} while(false)

/**
 * Appends a value at the end of the vector (using the '=' operator).
 *
 * Assumes there is enough capacity.
 *
 * @param _vec Vector.
 * @param _val Value.
 */
#define VECTOR_PUSH(_vec, _val) \
	do { \
		VECTOR_INDEX(_vec, VECTOR_LENGTH(_vec)) = (_val); /* set value */ \
		++VECTOR_LENGTH(_vec); /* increase length */ \
	}while(false)

/**
 * Appends a value at the end of the vector (using memcpy).
 *
 * Assumes there is enough capacity.
 *
 * @param _vec Vector.
 * @param _val Value.
 */
#define VECTOR_PUSHCOPY(_vec, _val) \
	VECTOR_PUSHARRAY(_vec, &(_val), 1)

/**
 * Appends the values of the array at the end of the vector (using memcpy).
 *
 * Assumes there is enough capacity.
 *
 * @param _vec  Vector.
 * @param _pval Array of values.
 * @param _n    Number of values.
 */
#define VECTOR_PUSHARRAY(_vec, _pval, _n) \
	do { \
		memcpy(&VECTOR_INDEX(_vec, VECTOR_LENGTH(_vec)), (_pval), (_n)*sizeof(VECTOR_FIRST(_vec))); /* set values */ \
		VECTOR_LENGTH(_vec) += (_n); /* increase length */ \
	} while(false)

/**
 * Removes and returns the last value of the vector.
 *
 * Assumes the array is not empty.
 *
 * @param _vec Vector.
 * @return Removed value.
 */
#define VECTOR_POP(_vec) \
	( VECTOR_INDEX(_vec, --VECTOR_LENGTH(_vec)) )

/**
 * Removes the last N values of the vector and returns the value of the last pop.
 *
 * Assumes there are enough values.
 *
 * @param _vec Vector.
 * @param _n   Number of pops.
 * @return Last removed value.
 */
#define VECTOR_POPN(_vec, _n) \
	( VECTOR_INDEX(_vec, (VECTOR_LENGTH(_vec) -= (_n))) )

/**
 * Removes the target index from the vector.
 *
 * Assumes the index is valid and there are enough values.
 *
 * @param _vec Vector.
 * @param _idx Index.
 */
#define VECTOR_ERASE(_vec, _idx) \
	VECTOR_ERASEN(_vec, _idx, 1)

/**
 * Removes N values from the target index of the vector.
 *
 * Assumes the index is valid and there are enough values.
 *
 * @param _vec Vector.
 * @param _idx Index.
 * @param _n   Number of values to remove.
 */
#define VECTOR_ERASEN(_vec, _idx, _n) \
	do { \
		if ((_idx) < VECTOR_LENGTH(_vec)-(_n) ) /* move data */ \
			memmove(&VECTOR_INDEX(_vec, _idx), &VECTOR_INDEX(_vec, (_idx)+(_n)), (VECTOR_LENGTH(_vec)-((_idx)+(_n)))*sizeof(VECTOR_FIRST(_vec))); \
		VECTOR_LENGTH(_vec) -= (_n); /* decrease length */ \
	} while(false)

/**
 * Removes all values from the vector.
 *
 * Does not free the allocated data.
 */
#define VECTOR_TRUNCATE(_vec) \
	do { \
		VECTOR_LENGTH(_vec) = 0; \
	} while (false)

/**
 * Clears the vector, freeing allocated data.
 *
 * @param _vec Vector.
 */
#define VECTOR_CLEAR(_vec) \
	do { \
		if (VECTOR_CAPACITY(_vec) > 0) { \
			aFree(VECTOR_DATA(_vec)); VECTOR_DATA(_vec) = NULL; /* clear allocated array */ \
			VECTOR_CAPACITY(_vec) = 0; /* clear capacity */ \
			VECTOR_LENGTH(_vec) = 0; /* clear length */ \
		} \
	} while(false)

/**
 * Binary heap library based on defines.
 *
 * Uses the VECTOR defines above.
 * Uses aMalloc, aRealloc, aFree.
 *
 * @warning
 *    BHEAP implementation details affect behaviour of A* pathfinding.
 */

/**
 * Declares an anonymous binary heap struct.
 *
 * @param _type Type of data.
 */
#define BHEAP_DECL(_type) \
	VECTOR_DECL(_type)

/**
 * Declares a named binary heap struct.
 *
 * @param _name Structure name.
 * @param _type Type of data.
 */
#define BHEAP_STRUCT_DECL(_name, _type) \
	VECTOR_STRUCT_DECL(_name, _type)

/**
 * Declares and initializes an anonymous binary heap variable.
 *
 * @param _type Type of data.
 * @param _var  Variable name.
 */
#define BHEAP_VAR(_type, _var) \
	VECTOR_VAR(_type, _var)

/**
 * Declares and initializes a named binary heap variable.
 *
 * @param _name Structure name.
 * @param _var  Variable name.
 */
#define BHEAP_STRUCT_VAR(_name, _var) \
	VECTOR_STRUCT_VAR(_name, _var)

/**
 * Initializes a heap.
 *
 * @param _heap Binary heap.
 */
#define BHEAP_INIT(_heap) \
	VECTOR_INIT(_heap)

/**
 * Returns the internal array of values.
 *
 * @param _heap Binary heap.
 * @return Internal array of values.
 */
#define BHEAP_DATA(_heap) \
	VECTOR_DATA(_heap)

/**
 * Returns the length of the heap.
 *
 * @param _heap Binary heap.
 * @return Length.
 */
#define BHEAP_LENGTH(_heap) \
	VECTOR_LENGTH(_heap)

/**
 * Returns the capacity of the heap.
 *
 * @param _heap Binary heap.
 * @return Capacity.
 */
#define BHEAP_CAPACITY(_heap) \
	VECTOR_CAPACITY(_heap)

/**
 * Ensures that the heap has the target number of empty positions.
 *
 * Increases the capacity in multiples of _step.
 *
 * @param _heap Binary heap.
 * @param _n    Required empty positions.
 * @param _step Increase.
 */
#define BHEAP_ENSURE(_heap, _n, _step) \
	VECTOR_ENSURE(_heap, _n, _step)

/**
 * Returns the top value of the heap.
 *
 * Assumes the heap is not empty.
 *
 * @param _heap Binary heap.
 * @return Value at the top.
 */
#define BHEAP_PEEK(_heap) \
	VECTOR_INDEX(_heap, 0)

/**
 * Inserts a value in the heap (using the '=' operator).
 *
 * Assumes there is enough capacity.
 *
 * The comparator takes two values as arguments, returns:
 *   - negative if the first value is on the top
 *   - positive if the second value is on the top
 *   - 0 if they are equal
 *
 * @param _heap   Binary heap.
 * @param _val    Value.
 * @param _topcmp Comparator.
 * @param _swp    Swapper.
 */
#define BHEAP_PUSH(_heap, _val, _topcmp, _swp) \
	do { \
		int _i_ = VECTOR_LENGTH(_heap); \
		VECTOR_PUSH(_heap, _val); /* insert at end */ \
		while (_i_ > 0) { \
			/* restore heap property in parents */ \
			int _parent_ = (_i_-1)/2; \
			if (_topcmp(VECTOR_INDEX(_heap, _parent_), VECTOR_INDEX(_heap, _i_)) < 0) \
				break; /* done */ \
			_swp(VECTOR_INDEX(_heap, _parent_), VECTOR_INDEX(_heap, _i_)); \
			_i_ = _parent_; \
		} \
	} while(false)

/**
 * Variant of BHEAP_PUSH used by A* implementation, matching client bheap.
 *
 * @see BHEAP_PUSH.
 *
 * @param _heap   Binary heap.
 * @param _val    Value.
 * @param _topcmp Comparator.
 * @param _swp    Swapper.
 */
#define BHEAP_PUSH2(_heap, _val, _topcmp, _swp) \
	do { \
		int _i_ = VECTOR_LENGTH(_heap); \
		VECTOR_PUSH(_heap, _val); /* insert at end */ \
		BHEAP_SIFTDOWN(_heap, 0, _i_, _topcmp, _swp); \
	} while(false)

/**
 * Removes the top value of the heap (using the '=' operator).
 *
 * Assumes the heap is not empty.
 *
 * The comparator takes two values as arguments, returns:
 *   - negative if the first value is on the top
 *   - positive if the second value is on the top
 *   - 0 if they are equal
 *
 * @param _heap   Binary heap.
 * @param _topcmp Comparator.
 * @param _swp Swapper.
 */
#define BHEAP_POP(_heap, _topcmp, _swp) \
	BHEAP_POPINDEX(_heap, 0, _topcmp, _swp)

/**
 * Variant of BHEAP_POP used by A* implementation, matching client bheap.
 *
 * @see BHEAP_POP.
 *
 * @param _heap   Binary heap.
 * @param _topcmp Comparator.
 * @param _swp    Swapper.
 */
#define BHEAP_POP2(_heap, _topcmp, _swp) \
	do { \
		VECTOR_INDEX(_heap, 0) = VECTOR_POP(_heap); /* put last at index */ \
		if (VECTOR_LENGTH(_heap) == 0) /* removed last, nothing to do */ \
			break; \
		BHEAP_SIFTUP(_heap, 0, _topcmp, _swp); \
	} while(false)

/**
 * Removes the target value of the heap (using the '=' operator).
 *
 * Assumes the index exists.
 *
 * The comparator takes two values as arguments, returns:
 *   - negative if the first value is on the top
 *   - positive if the second value is on the top
 *   - 0 if they are equal
 *
 * @param _heap   Binary heap.
 * @param _idx    Index.
 * @param _topcmp Comparator.
 * @param _swp    Swapper.
 */
#define BHEAP_POPINDEX(_heap, _idx, _topcmp, _swp) \
	do { \
		int _i_ = _idx; \
		VECTOR_INDEX(_heap, _idx) = VECTOR_POP(_heap); /* put last at index */ \
		if (_i_ >= VECTOR_LENGTH(_heap)) /* removed last, nothing to do */ \
			break; \
		while (_i_ > 0) { \
			/* restore heap property in parents */ \
			int _parent_ = (_i_-1)/2; \
			if (_topcmp(VECTOR_INDEX(_heap, _parent_), VECTOR_INDEX(_heap, _i_)) < 0) \
				break; /* done */ \
			_swp(VECTOR_INDEX(_heap, _parent_), VECTOR_INDEX(_heap, _i_)); \
			_i_ = _parent_; \
		} \
		while (_i_ < VECTOR_LENGTH(_heap)) { \
			/* restore heap property in children */ \
			int _lchild_ = _i_*2 + 1; \
			int _rchild_ = _i_*2 + 2; \
			if ((_lchild_ >= VECTOR_LENGTH(_heap) || _topcmp(VECTOR_INDEX(_heap, _i_), VECTOR_INDEX(_heap, _lchild_)) <= 0) \
			 && (_rchild_ >= VECTOR_LENGTH(_heap) || _topcmp(VECTOR_INDEX(_heap, _i_), VECTOR_INDEX(_heap, _rchild_)) <= 0)) { \
				break; /* done */ \
			} else if (_rchild_ >= VECTOR_LENGTH(_heap) || _topcmp(VECTOR_INDEX(_heap, _lchild_), VECTOR_INDEX(_heap, _rchild_)) <= 0) { \
				/* left child */ \
				_swp(VECTOR_INDEX(_heap, _i_), VECTOR_INDEX(_heap, _lchild_)); \
				_i_ = _lchild_; \
			} else { \
				/* right child */ \
				_swp(VECTOR_INDEX(_heap, _i_), VECTOR_INDEX(_heap, _rchild_)); \
				_i_ = _rchild_; \
			} \
		} \
	} while(false)

/**
 * Follow path up towards (but not all the way to) the root, swapping nodes
 * until finding a place where the new item that was placed at _idx fits.
 *
 * Only goes as high as _startidx (usually 0).
 *
 * @param _heap     Binary heap.
 * @param _startidx Index of an ancestor of _idx.
 * @param _idx      Index of an inserted element.
 * @param _topcmp   Comparator.
 * @param _swp      Swapper.
 */
#define BHEAP_SIFTDOWN(_heap, _startidx, _idx, _topcmp, _swp) \
	do { \
		int _i2_ = _idx; \
		while (_i2_ > _startidx) { \
			/* restore heap property in parents */ \
			int _parent_ = (_i2_-1)/2; \
			if (_topcmp(VECTOR_INDEX(_heap, _parent_), VECTOR_INDEX(_heap, _i2_)) <= 0) \
				break; /* done */ \
			_swp(VECTOR_INDEX(_heap, _parent_), VECTOR_INDEX(_heap, _i2_)); \
			_i2_ = _parent_; \
		} \
	} while(false)

/**
 * Repeatedly swap the smaller child with parent, after placing a new item at _idx.
 *
 * @param _heap   Binary heap.
 * @param _idx    Index of an inserted element.
 * @param _topcmp Comparator.
 * @param _swp    Swapper.
 */
#define BHEAP_SIFTUP(_heap, _idx, _topcmp, _swp) \
	do { \
		int _i_ = _idx; \
		int _lchild_ = _i_*2 + 1; \
		while (_lchild_ < VECTOR_LENGTH(_heap)) { \
			/* restore heap property in children */ \
			int _rchild_ = _i_*2 + 2; \
			if (_rchild_ >= VECTOR_LENGTH(_heap) || _topcmp(VECTOR_INDEX(_heap, _lchild_), VECTOR_INDEX(_heap, _rchild_)) < 0) { \
				/* left child */ \
				_swp(VECTOR_INDEX(_heap, _i_), VECTOR_INDEX(_heap, _lchild_)); \
				_i_ = _lchild_; \
			} else { \
				/* right child */ \
				_swp(VECTOR_INDEX(_heap, _i_), VECTOR_INDEX(_heap, _rchild_)); \
				_i_ = _rchild_; \
			} \
			_lchild_ = _i_*2 + 1; \
		} \
		BHEAP_SIFTDOWN(_heap, _idx, _i_, _topcmp, _swp); \
	} while(false)

/**
 * Restores a heap (after modifying the item at _idx).
 *
 * @param _heap   Binary heap.
 * @param _idx    Index.
 * @param _topcmp Comparator.
 * @param _swp    Swapper.
 */
#define BHEAP_UPDATE(_heap, _idx, _topcmp, _swp) \
	do { \
		BHEAP_SIFTDOWN(_heap, 0, _idx, _topcmp, _swp); \
		BHEAP_SIFTUP(_heap, _idx, _topcmp, _swp); \
	} while(false)

/**
 * Clears the binary heap, freeing allocated data.
 *
 * @param _heap Binary heap.
 */
#define BHEAP_CLEAR(_heap) \
	VECTOR_CLEAR(_heap)

/**
 * Generic comparator for a min-heap (minimum value at top).
 *
 * Returns -1 if v1 is smaller, 1 if v2 is smaller, 0 if equal.
 *
 * @warning
 *    Arguments may be evaluted more than once.
 *
 * @param v1 First value.
 * @param v2 Second value.
 * @return negative if v1 is top, positive if v2 is top, 0 if equal.
 */
#define BHEAP_MINTOPCMP(v1, v2) \
	( (v1) == (v2) ? 0 : (v1) < (v2) ? -1 : 1 )

/**
 * Generic comparator for a max-heap (maximum value at top).
 *
 * Returns -1 if v1 is bigger, 1 if v2 is bigger, 0 if equal.
 *
 * @warning
 *    Arguments may be evaluted more than once.
 *
 * @param v1 First value.
 * @param v2 Second value.
 * @return negative if v1 is top, positive if v2 is top, 0 if equal.
 */
#define BHEAP_MAXTOPCMP(v1, v2) \
	( (v1) == (v2) ? 0 : (v1) > (v2) ? -1 : 1 )

#endif /* COMMON_DB_H */
