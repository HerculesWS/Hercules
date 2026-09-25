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

#include <forward_list>
#include <stdlib.h>
#include <string.h>

#ifndef DISABLE_ERS

struct ers_list
{
	struct ers_list *Next;
};

static std::forward_list<ERS *> ers_instance_list;

void *ERS::alloc() noexcept
{
	void *ret;

	if (m_cache.reuse_list != nullptr) {
		ret = (void *)((unsigned char *)m_cache.reuse_list + sizeof(struct ers_list));
		m_cache.reuse_list = m_cache.reuse_list->Next;
	} else if (m_cache.free > 0) {
		m_cache.free--;
		ret = &m_cache.blocks[m_cache.used - 1][m_cache.free * (size_t)m_cache.object_size + sizeof(struct ers_list)];
	} else {
		if (m_cache.used == m_cache.max) {
			m_cache.max = (m_cache.max * 4) + 3;
			RECREATE(m_cache.blocks, unsigned char *, m_cache.max);
		}

		CREATE(m_cache.blocks[m_cache.used], unsigned char, m_cache.object_size * m_cache.chunk_size);
		m_cache.used++;

		m_cache.free = m_cache.chunk_size -1;
		ret = &m_cache.blocks[m_cache.used - 1][m_cache.free * (size_t)m_cache.object_size + sizeof(struct ers_list)];
	}

	m_count++;
	m_cache.used_objs++;

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

	if ((m_options & ERS_OPT_CLEAN) != 0)
		memset((unsigned char*)reuse + sizeof(struct ers_list), 0, m_cache.object_size - sizeof(struct ers_list));

	reuse->Next = m_cache.reuse_list;
	m_cache.reuse_list = reuse;
	m_count--;
	m_cache.used_objs--;
}

size_t ERS::entry_size() const noexcept
{
	return m_cache.object_size;
}

ERS::~ERS() noexcept
{
	if (m_count > 0) {
		if ((m_options & ERS_OPT_CLEAR) == 0) {
			ShowWarning("Memory leak detected at ERS '%s', %u objects not freed.\n", m_name.c_str(), m_count);
		}
	}

	for (unsigned int i = 0; i < m_cache.used; i++)
		aFree(m_cache.blocks[i]);

	aFree(m_cache.blocks);

	ers_instance_list.remove(this);
}

void ERS::chunk_size(unsigned int new_size) noexcept
{
	if ((m_options&ERS_OPT_FLEX_CHUNK) == 0) {
		ShowWarning("ers_cache_size: '%s' has adjusted its chunk size to '%u', however ERS_OPT_FLEX_CHUNK is missing!\n", m_name.c_str(), new_size);
	}

	m_cache.chunk_size = new_size;
}

ERS::ERS(uint32 size, const std::string &name, enum ERSOptions options) noexcept : m_name(name), m_options(options) // FIXME: change this to a flag type
{
	size += sizeof(struct ers_list);

#if ERS_ALIGNED > 1 // If it's aligned to 1-byte boundaries, no need to bother.
	if (size % ERS_ALIGNED)
		size += ERS_ALIGNED - size % ERS_ALIGNED;
#endif

	m_cache.object_size = size;

	ers_instance_list.push_front(this);
}

std::tuple<unsigned int, unsigned int, unsigned int, unsigned int> ERS::report_cache(void) const noexcept
{
	ShowMessage(CL_BOLD "[ERS Cache of size '" CL_NORMAL CL_WHITE "%u" CL_NORMAL CL_BOLD "' report]\n" CL_NORMAL, m_cache.object_size);
	ShowMessage("\tblocks in use      : %u/%u\n", m_cache.used_objs, m_cache.used_objs + m_cache.free);
	ShowMessage("\tblocks unused      : %u\n", m_cache.free);
	ShowMessage("\tmemory in use      : %.2f MB\n", m_cache.used_objs == 0 ? 0. : (double)((m_cache.used_objs * m_cache.object_size)/1024)/1024);
	ShowMessage("\tmemory allocated   : %.2f MB\n", (m_cache.free + m_cache.used_objs) == 0 ? 0. : (double)(((m_cache.used_objs + m_cache.free) * m_cache.object_size)/1024)/1024);

	return {m_cache.used_objs, m_cache.used_objs + m_cache.free, m_cache.used_objs * m_cache.object_size, (m_cache.used_objs + m_cache.free) * m_cache.object_size};
}

#ifdef DEBUG
bool ERS::report(void) const noexcept
{
	if ((m_options & ERS_OPT_WAIT) != 0 && m_count == 0)
		return false;

	ShowMessage(CL_BOLD "[ERS Instance " CL_NORMAL CL_WHITE "%s" CL_NORMAL CL_BOLD " report]\n" CL_NORMAL, m_name.c_str());
	ShowMessage("\tblock size        : %u\n", m_cache.object_size);
	ShowMessage("\tblocks being used : %u\n", m_count);
	ShowMessage("\tpeak blocks       : %u\n", m_peak);
	ShowMessage("\tmemory in use     : %.2f MB\n", m_count == 0 ? 0. : (double)((m_count * m_cache.object_size)/1024)/1024);

	return true;
}
#endif

void ers_report(void)
{
	unsigned int blocks_u = 0, blocks_a = 0, memory_b = 0, memory_t = 0;
#ifdef DEBUG
	unsigned int instance_c = 0, instance_c_d = 0;

	for (const auto instance : ers_instance_list) {
		instance_c++;
		if (instance->report() == false)
			continue;
		instance_c_d++;
	}
#endif

	for (const auto instance : ers_instance_list) {
		const auto [blocks_used, blocks_total, memory_used, memory_total] = instance->report_cache();

		blocks_u += blocks_used;
		blocks_a += blocks_total;
		memory_b += memory_used;
		memory_t += memory_total;
	}
#ifdef DEBUG
	ShowInfo("ers_report: '" CL_WHITE "%u" CL_NORMAL "' instances in use, '" CL_WHITE "%u" CL_NORMAL "' displayed\n", instance_c, instance_c_d);
#endif
	ShowInfo("ers_report: '" CL_WHITE "%u" CL_NORMAL "' blocks in use, consuming '" CL_WHITE "%.2f MB" CL_NORMAL "'\n",blocks_u,(double)((memory_b)/1024)/1024);
	ShowInfo("ers_report: '" CL_WHITE "%u" CL_NORMAL "' blocks total, consuming '" CL_WHITE "%.2f MB" CL_NORMAL "' \n",blocks_a,(double)((memory_t)/1024)/1024);
}

/**
 * Call on shutdown to clear remaining entries
 **/
void ers_final(void)
{
	for (auto instance : ers_instance_list) {
		delete instance;
	}

	ers_instance_list.clear();
}

#endif
