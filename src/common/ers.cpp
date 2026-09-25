/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2026 Hercules Dev Team
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

/*****************************************************************************\
 *  <H1>Entry Reusage System</H1>                                            *
 *                                                                           *
 *  There are several root entry managers, each with a different entry size. *
 *  Each manager will keep track of how many instances have been 'created'.  *
 *  They will only automatically destroy themselves after the last instance  *
 *  is destroyed.                                                            *
 *                                                                           *
 *  Entries can be allocated from the managers.                              *
 *  If it has reusable entries (freed entry), it uses one.                   *
 *  So no assumption should be made about the data of the entry.             *
 *  Entries should be freed in the manager they where allocated from.        *
 *  Failure to do so can lead to unexpected behaviors.                      *
 *                                                                           *
 *  <H2>Advantages:</H2>                                                     *
 *  - The same manager is used for entries of the same size.                 *
 *    So entries freed in one instance of the manager can be used by other   *
 *    instances of the manager.                                              *
 *  - Much less memory allocation/deallocation - program will be faster.     *
 *  - Avoids memory fragmentation - program will run better for longer.       *
 *                                                                           *
 *  <H2>Disadvantages:</H2>                                                   *
 *  - Unused entries are almost inevitable - memory being wasted.            *
 *  - A  manager will only auto-destroy when all of its instances are        *
 *    destroyed so memory will usually only be recovered near the end.       *
 *  - Always wastes space for entries smaller than a pointer.                *
 *                                                                           *
 *  WARNING: The system is not thread-safe at the moment.                    *
 *                                                                           *
 *  HISTORY:                                                                 *
 *    0.1 - Initial version                                                  *
 *    1.0 - ERS Rework                                                       *
 *                                                                           *
 * @version 1.0 - ERS Rework                                                 *
 * @author GreenBox @ rAthena Project                                        *
 * @encoding US-ASCII                                                        *
 * @see common#ers.h                                                         *
\*****************************************************************************/

#define HERCULES_CORE

#include "ers.h"

#include "common/cbasetypes.h"
#include "common/memmgr.h" // CREATE, RECREATE, aMalloc, aFree
#include "common/nullpo.h"
#include "common/showmsg.h" // ShowMessage, ShowError, ShowFatalError, CL_BOLD, CL_NORMAL

#include <stdlib.h>
#include <string.h>

#ifndef DISABLE_ERS

#define ERS_BLOCK_ENTRIES 2048


struct ers_list
{
	struct ers_list *Next;
};

typedef struct ers_cache
{
	// Allocated object size, including ers_list size
	unsigned int ObjectSize;

	// Number of ers_instances referencing this
	int ReferenceCount;

	// Reuse linked list
	struct ers_list *ReuseList;

	// Memory blocks array
	unsigned char **Blocks;

	// Max number of blocks
	unsigned int Max;

	// Free objects count
	unsigned int Free;

	// Used blocks count
	unsigned int Used;

	// Objects in-use count
	unsigned int UsedObjs;

	// Default = ERS_BLOCK_ENTRIES, can be adjusted for performance for individual cache sizes.
	unsigned int ChunkSize;

	// Misc options, some options are shared from the instance
	enum ERSOptions Options;

	// Linked list
	struct ers_cache *Next, *Prev;
} ers_cache_t;

// Array containing a pointer for all ers_cache structures
static ers_cache_t *CacheList = NULL;
static ERS *InstanceList = NULL;

/**
 * @param Options the options from the instance seeking a cache, we use it to give it a cache with matching configuration
 **/
static ers_cache_t *ers_find_cache(unsigned int size, enum ERSOptions Options)
{
	ers_cache_t *cache;

	for (cache = CacheList; cache; cache = cache->Next)
		if ( cache->ObjectSize == size && cache->Options == ( Options & ERS_CACHE_OPTIONS ) )
			return cache;

	CREATE(cache, ers_cache_t, 1);
	cache->ObjectSize = size;
	cache->ReferenceCount = 0;
	cache->ReuseList = NULL;
	cache->Blocks = NULL;
	cache->Free = 0;
	cache->Used = 0;
	cache->UsedObjs = 0;
	cache->Max = 0;
	cache->ChunkSize = ERS_BLOCK_ENTRIES;
	cache->Options = (enum ERSOptions)(Options & ERS_CACHE_OPTIONS);

	if (CacheList == NULL)
	{
		CacheList = cache;
	}
	else
	{
		cache->Next = CacheList;
		cache->Next->Prev = cache;
		CacheList = cache;
		CacheList->Prev = NULL;
	}

	return cache;
}

static void ers_free_cache(ers_cache_t *cache, bool remove)
{
	unsigned int i;

	nullpo_retv(cache);
	for (i = 0; i < cache->Used; i++)
		aFree(cache->Blocks[i]);

	if (cache->Next)
		cache->Next->Prev = cache->Prev;

	if (cache->Prev)
		cache->Prev->Next = cache->Next;
	else
		CacheList = cache->Next;

	aFree(cache->Blocks);

	aFree(cache);
}

void *ERS::alloc() noexcept
{
	void *ret;

	if (m_cache->ReuseList != nullptr) {
		ret = (void *)((unsigned char *)m_cache->ReuseList + sizeof(struct ers_list));
		m_cache->ReuseList = m_cache->ReuseList->Next;
	} else if (m_cache->Free > 0) {
		m_cache->Free--;
		ret = &m_cache->Blocks[m_cache->Used - 1][m_cache->Free * (size_t)m_cache->ObjectSize + sizeof(struct ers_list)];
	} else {
		if (m_cache->Used == m_cache->Max) {
			m_cache->Max = (m_cache->Max * 4) + 3;
			RECREATE(m_cache->Blocks, unsigned char *, m_cache->Max);
		}

		CREATE(m_cache->Blocks[m_cache->Used], unsigned char, m_cache->ObjectSize * m_cache->ChunkSize);
		m_cache->Used++;

		m_cache->Free = m_cache->ChunkSize -1;
		ret = &m_cache->Blocks[m_cache->Used - 1][m_cache->Free * (size_t)m_cache->ObjectSize + sizeof(struct ers_list)];
	}

	m_count++;
	m_cache->UsedObjs++;

#ifdef DEBUG
	if (m_count > m_peak)
		m_peak = m_count;
#endif

	return ret;
}

void ERS::free(void *entry) noexcept
{
	struct ers_list *reuse = (struct ers_list *)((unsigned char *)entry - sizeof(struct ers_list));

	if (entry == nullptr) {
		ShowError("ERS::free: NULL entry, nothing to free.\n");
		return;
	}

	if ((m_cache->Options & ERS_OPT_CLEAN) != 0)
		memset((unsigned char*)reuse + sizeof(struct ers_list), 0, m_cache->ObjectSize - sizeof(struct ers_list));

	reuse->Next = m_cache->ReuseList;
	m_cache->ReuseList = reuse;
	m_count--;
	m_cache->UsedObjs--;
}

size_t ERS::entry_size() const noexcept
{
	return m_cache->ObjectSize;
}

ERS::~ERS() noexcept
{
	if (m_count > 0) {
		if ((m_options & ERS_OPT_CLEAR) == 0) {
			ShowWarning("Memory leak detected at ERS '%s', %u objects not freed.\n", m_name.c_str(), m_count);
		}
	}

	if (--m_cache->ReferenceCount <= 0)
		ers_free_cache(m_cache, true);

	if (m_next)
		m_next->m_prev = m_prev;

	if (m_prev)
		m_prev->m_next = m_next;
	else
		InstanceList = m_next;
}

void ERS::chunk_size(unsigned int new_size) noexcept
{
	if ((m_cache->Options&ERS_OPT_FLEX_CHUNK) == 0) {
		ShowWarning("ers_cache_size: '%s' has adjusted its chunk size to '%u', however ERS_OPT_FLEX_CHUNK is missing!\n", m_name.c_str(), new_size);
	}

	m_cache->ChunkSize = new_size;
}

ERS::ERS(uint32 size, const std::string &name, enum ERSOptions options) noexcept : m_name(name), m_options(options) // FIXME: change this to a flag type
{
	size += sizeof(struct ers_list);

#if ERS_ALIGNED > 1 // If it's aligned to 1-byte boundaries, no need to bother.
	if (size % ERS_ALIGNED)
		size += ERS_ALIGNED - size % ERS_ALIGNED;
#endif

	m_cache = ers_find_cache(size,m_options);
	m_cache->ReferenceCount++;

	if (InstanceList == NULL) {
		InstanceList = this;
	} else {
		m_next = InstanceList;
		m_next->m_prev = this;
		InstanceList = this;
		InstanceList->m_prev = nullptr;
	}
}

#ifdef DEBUG
bool ERS::report(void) const noexcept
{
	if ((m_options & ERS_OPT_WAIT) != 0 && m_count == 0)
		return false;

	ShowMessage(CL_BOLD "[ERS Instance " CL_NORMAL CL_WHITE "%s" CL_NORMAL CL_BOLD " report]\n" CL_NORMAL, m_name.c_str());
	ShowMessage("\tblock size        : %u\n", m_cache->ObjectSize);
	ShowMessage("\tblocks being used : %u\n", m_count);
	ShowMessage("\tpeak blocks       : %u\n", m_peak);
	ShowMessage("\tmemory in use     : %.2f MB\n", m_count == 0 ? 0. : (double)((m_count * m_cache->ObjectSize)/1024)/1024);

	return true;
}
#endif

void ers_report(void)
{
	ers_cache_t *cache;
	unsigned int cache_c = 0, blocks_u = 0, blocks_a = 0, memory_b = 0, memory_t = 0;
#ifdef DEBUG
	unsigned int instance_c = 0, instance_c_d = 0;

	for (const ERS *instance = InstanceList; instance; instance = instance->m_next) {
		instance_c++;
		if (instance->report() == false)
			continue;
		instance_c_d++;
	}
#endif

	for (cache = CacheList; cache; cache = cache->Next) {
		cache_c++;
		ShowMessage(CL_BOLD "[ERS Cache of size '" CL_NORMAL CL_WHITE "%u" CL_NORMAL CL_BOLD "' report]\n" CL_NORMAL, cache->ObjectSize);
		ShowMessage("\tinstances          : %d\n", cache->ReferenceCount);
		ShowMessage("\tblocks in use      : %u/%u\n", cache->UsedObjs, cache->UsedObjs+cache->Free);
		ShowMessage("\tblocks unused      : %u\n", cache->Free);
		ShowMessage("\tmemory in use      : %.2f MB\n", cache->UsedObjs == 0 ? 0. : (double)((cache->UsedObjs * cache->ObjectSize)/1024)/1024);
		ShowMessage("\tmemory allocated   : %.2f MB\n", (cache->Free+cache->UsedObjs) == 0 ? 0. : (double)(((cache->UsedObjs+cache->Free) * cache->ObjectSize)/1024)/1024);
		blocks_u += cache->UsedObjs;
		blocks_a += cache->UsedObjs + cache->Free;
		memory_b += cache->UsedObjs * cache->ObjectSize;
		memory_t += (cache->UsedObjs+cache->Free) * cache->ObjectSize;
	}
#ifdef DEBUG
	ShowInfo("ers_report: '" CL_WHITE "%u" CL_NORMAL "' instances in use, '" CL_WHITE "%u" CL_NORMAL "' displayed\n",instance_c,instance_c_d);
#endif
	ShowInfo("ers_report: '" CL_WHITE "%u" CL_NORMAL "' caches in use\n",cache_c);
	ShowInfo("ers_report: '" CL_WHITE "%u" CL_NORMAL "' blocks in use, consuming '" CL_WHITE "%.2f MB" CL_NORMAL "'\n",blocks_u,(double)((memory_b)/1024)/1024);
	ShowInfo("ers_report: '" CL_WHITE "%u" CL_NORMAL "' blocks total, consuming '" CL_WHITE "%.2f MB" CL_NORMAL "' \n",blocks_a,(double)((memory_t)/1024)/1024);
}

/**
 * Call on shutdown to clear remaining entries
 **/
void ers_final(void)
{
	ERS *instance = InstanceList, *next;

	while (instance != nullptr) {
		next = instance->m_next;
		delete instance;
		instance = next;
	}
}

#endif
