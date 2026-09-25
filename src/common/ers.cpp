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

#include "common/memmgr.h" // CREATE, RECREATE, aMalloc, aFree

#include <stdlib.h>

#ifndef DISABLE_ERS

static std::forward_list<ERI *> ers_instance_list;

void ERI::add_to_global_list(void) noexcept
{
	ers_instance_list.push_front(this);
}

void ERI::remove_from_global_list(void) noexcept
{
	ers_instance_list.remove(this);
}

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
