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
 *                                                                           *
 * @version 0.1 - Initial version                                            *
 * @author Flavio @ Amazon Project                                           *
 * @encoding US-ASCII                                                        *
\*****************************************************************************/
#ifndef COMMON_ERS_H
#define COMMON_ERS_H

#include "common/showmsg.h"
#include "common/memmgr.h"

#include <forward_list>
#include <string>
#include <tuple>
#include <vector>

/*****************************************************************************\
 *  (1) All public parts of the Entry Reusage System.                        *
 *  DISABLE_ERS           - Define to disable this system.                   *
 *  ERS_ALIGNED           - Alignment of the entries in the blocks.          *
 *  ERS                   - Entry manager.                                   *
 *  ers_new               - Allocate an instance of an entry manager.        *
 *  ers_report            - Print a report about the current state.          *
 *  ers_final             - Clears the remainder of the managers.           *
\*****************************************************************************/

/**
 * Define this to disable the Entry Reusage System.
 * All code except the typedef of ERInterface will be disabled.
 * To allow a smooth transition,
 */
//#define DISABLE_ERS

constexpr unsigned int ers_chunk_size = 2048;

enum ERSOptions {
	ERS_OPT_NONE        = 0x00,
	ERS_OPT_CLEAR       = 0x01,/* silently clears any entries left in the manager upon destruction */
	ERS_OPT_WAIT        = 0x02,/* wait for entries to come in order to list! */
	// ERS_OPT_FREE_NAME   = 0x04,/* name is dynamic memory, and should be freed */ // UNUSED
	ERS_OPT_CLEAN       = 0x08,/* clears used memory upon ers_free so that its all new to be reused on the next alloc */
	ERS_OPT_FLEX_CHUNK  = 0x10,/* signs that it should look for its own cache given it'll have a dynamic chunk size, so that it doesn't affect the other ERS it'd otherwise be sharing */

	/* Compound, is used to determine whether it should be looking for a cache of matching options */
	ERS_CACHE_OPTIONS   = ERS_OPT_CLEAN|ERS_OPT_FLEX_CHUNK,
};

/**
 * A common interface for ERS
 */
class ERI
{
  public:
	virtual ~ERI() = default;
	/**
	 * Reports debug information for instance cache
	 * @return used blocks, total blocks, memory used, total memory
	 */
	[[nodiscard]] virtual std::tuple<size_t, size_t, size_t, size_t>
	        report_cache(void) const noexcept = 0;

#ifdef DEBUG
	/**
	 * Reports debug information for current instance.
	 * @return true if reports is displayed, false otherwise.
	 */
	[[nodiscard]] virtual bool report(void) const noexcept = 0;
#endif

  protected:
	/**
	 * Adds ERS instance to the global list of instances
	 */
	void add_to_global_list(void) noexcept;
	/**
	 * Removes ERS instance from global list of instances
	 */
	void remove_from_global_list(void) noexcept;
};

/**
 * Public interface of the entry manager.
 */
template<typename T>
class ERS : public ERI
{
  public:
	/**
	 * Get a new instance of the manager that handles the specified entry type.
	 * It's also aligned to ERS_ALIGNED bytes, so the smallest multiple of
	 * ERS_ALIGNED that is greater or equal to size is what's actually used.
	 * @param name the name of this ERS manager instance
	 * @param options a bitmask options of this instance manager
	 */
	ERS(const std::string &name, enum ERSOptions options) noexcept
	        : m_name(name), m_options(options) // FIXME: change this to a flag type
	{
		add_to_global_list();
	};

	/**
	 * Destroy this instance of the manager.
	 * The manager is actually only destroyed when all the instances are destroyed.
	 * When destroying the manager a warning is shown if the manager has
	 * missing/extra entries.
	 */
	~ERS() noexcept override
	{
		if (used_objects() > 0) {
			if ((m_options & ERS_OPT_CLEAR) == 0) {
				ShowWarning("Memory leak detected at ERS '%s', %" PRIuS " objects not freed.\n",
				            m_name.c_str(),
				            used_objects());
			}
		}

		remove_from_global_list();
	}

	/**
	 * Allocate an entry from this entry manager.
	 * If there are reusable entries available, it reuses one instead.
	 * @return An entry
	 */
	template<typename... Args>
	[[nodiscard]] T *alloc(Args &&...args) noexcept
	{
		T *ret;

		if (m_cache.reuse_list.empty() == false) {
			ret = m_cache.reuse_list.front();
			m_cache.reuse_list.pop_front();
			m_cache.reuse_size--;

			new (ret) T(std::forward<Args>(args)...);
		} else if (m_cache.blocks.size() > 0 && m_cache.blocks.back().size() < m_cache.blocks.back().capacity()) {
			m_cache.blocks.back().emplace_back(std::forward<Args>(args)...);
			ret = &m_cache.blocks.back().back();
		} else {
			m_cache.blocks.emplace_back();

			auto &chunk = m_cache.blocks.back();
			chunk.reserve(m_cache.chunk_size);
			chunk.emplace_back(std::forward<Args>(args)...);
			ret = &chunk.back();
		}

#ifdef DEBUG
		if (m_peak < used_objects())
			m_peak = used_objects();
#endif

		return ret;
	}

	/**
	 * Free an entry allocated from this manager.
	 * WARNING: Does not check if the entry was allocated by this manager.
	 * Freeing such an entry can lead to unexpected behavior.
	 * @param entry Entry to be freed
	 */
	void free(T *entry) noexcept
	{
		if (entry == nullptr) {
			ShowError("ERS::free: NULL entry, nothing to free.\n");
			return;
		}

		entry->~T();

		if ((m_options & ERS_OPT_CLEAN) != 0)
			memset(entry, 0, sizeof(T));

		m_cache.reuse_list.push_front(entry);
		m_cache.reuse_size++;
	}

	/**
	 * Return the size of the entries allocated from this manager.
	 * @param self Interface of the entry manager
	 * @return Size of the entries of this manager in bytes
	 */
	[[nodiscard]] size_t entry_size(void) const noexcept
	{
		return sizeof(T);
	}

	/**
	 * Adjusts chunk size of the ers cache requires ERS_OPT_FLEX_CHUNK option
	 * otherwise it throws a warning
	 * @param new_size the new chunk size
	 */
	void chunk_size(unsigned int new_size) noexcept
	{
		if ((m_options & ERS_OPT_FLEX_CHUNK) == 0) {
			ShowWarning(
			        "ers_cache_size: '%s' has adjusted its chunk size to '%u', however ERS_OPT_FLEX_CHUNK "
			        "is missing!\n",
			        m_name.c_str(),
			        new_size);
		}

		m_cache.chunk_size = new_size;
	}

	/**
	 * Reports debug information for instance cache
	 * @return used blocks, total blocks, memory used, total memory
	 */
	[[nodiscard]] std::tuple<size_t, size_t, size_t, size_t>
	        report_cache(void) const noexcept override
	{
		ShowMessage(CL_BOLD "[ERS Cache of size '" CL_NORMAL CL_WHITE "%" PRIuS CL_NORMAL CL_BOLD
		                    "' report]\n" CL_NORMAL,
		            sizeof(T));
		ShowMessage("\tblocks in use      : %" PRIuS "/%" PRIuS "\n", used_objects(), used_objects() + unused_blocks());
		ShowMessage("\tblocks unused      : %" PRIuS "\n", unused_blocks());
		ShowMessage("\tmemory in use      : %.2f MB\n",
		            used_objects() == 0 ? 0.
		                                   : (double)((used_objects() * sizeof(T)) / 1024) / 1024);
		ShowMessage("\tmemory allocated   : %.2f MB\n",
		            (unused_blocks() + used_objects()) == 0
		                    ? 0.
		                    : (double)(((used_objects() + unused_blocks()) * sizeof(T)) / 1024)
		                              / 1024);

		return {used_objects(),
		        used_objects() + unused_blocks(),
		        used_objects() * sizeof(T),
		        (used_objects() + unused_blocks()) * sizeof(T)};
	}

#ifdef DEBUG
	/**
	 * Reports debug information for current instance.
	 * @return true if reports is displayed, false otherwise.
	 */
	[[nodiscard]] bool report(void) const noexcept override
	{
		if ((m_options & ERS_OPT_WAIT) != 0 && used_objects() == 0)
			return false;

		ShowMessage(CL_BOLD "[ERS Instance " CL_NORMAL CL_WHITE "%s" CL_NORMAL CL_BOLD " report]\n" CL_NORMAL,
		            m_name.c_str());
		ShowMessage("\tblock size        : %" PRIuS "\n", sizeof(T));
		ShowMessage("\tblocks being used : %" PRIuS "\n", used_objects());
		ShowMessage("\tpeak blocks       : %" PRIuS "\n", m_peak);
		ShowMessage("\tmemory in use     : %.2f MB\n",
		            used_objects() == 0 ? 0. : (double)((used_objects() * sizeof(T)) / 1024) / 1024);

		return true;
	}
#endif

  private:
  	/**
	 * Returns never used blocks
	 * @return allocated blocks that aren't used
	 */
	[[nodiscard]] size_t unused_blocks(void) const noexcept
	{
		if (m_cache.blocks.size() < 1)
			return 0;

		return m_cache.blocks.back().capacity() - m_cache.blocks.back().size();
	}

	/**
	 * Returns currently allocated objects
	 * @return count of objects that are actually allocated for us
	 */
	[[nodiscard]] size_t used_objects(void) const noexcept
	{
		if (m_cache.blocks.size() < 1)
			return 0;

		return (m_cache.blocks.size() * m_cache.chunk_size) - unused_blocks() - m_cache.reuse_size;
	}

	std::string m_name; //< Name, used for debugging purposes

	enum ERSOptions m_options {
		ERS_OPT_NONE
	}; //< Misc options

#ifdef DEBUG
	/* for data analysis [Ind/Hercules] */
	size_t m_peak{0};
#endif

	struct {
		std::forward_list<T *> reuse_list;  //< Reuse linked list
		size_t reuse_size{0};
		std::vector<std::vector<T>> blocks; //< Memory blocks array
		unsigned int chunk_size{
		        ers_chunk_size}; //< Default = ERS_BLOCK_ENTRIES, can be adjusted for performance for individual
		                         // cache sizes.
	} m_cache;
};

#ifdef DISABLE_ERS
// Use memory manager to allocate/free and disable other interface functions
#	define ers_alloc(obj,type) ((void)(obj), (type *)aMalloc(sizeof(type)))
#	define ers_free(obj,entry) ((void)(obj), aFree(entry))
#	define ers_entry_size(obj) ((void)(obj), (size_t)0)
#	define ers_destroy(obj) ((void)(obj), (void)0)
#	define ers_chunk_size(obj,size) ((void)(obj), (void)(size), (size_t)0)
// Disable the public functions
#	define ers_new(type,name,options) nullptr
#	define ers_report() (void)0
#	define ers_final() (void)0
#else /* not DISABLE_ERS */
// These defines should be used to allow the code to keep working whenever
// the system is disabled
#	define ers_new(type,name,options) (new ERS<type>((name), (options)))
#	define ers_alloc(obj, ...) ((obj)->alloc(##__VA_ARGS__))
#	define ers_free(obj,entry) ((obj)->free((entry)))
#	define ers_entry_size(obj) ((obj)->entry_size())
#	define ers_destroy(obj)    (delete (obj))
#	define ers_chunk_size(obj,size) ((obj)->chunk_size((size)))

#ifdef HERCULES_CORE
/**
 * Print a report about the current state of the Entry Reusage System.
 * Shows information about the global system and each entry manager.
 * The number of entries are checked and a warning is shown if extra reusable
 * entries are found.
 * The extra entries are included in the count of reusable entries.
 */
void ers_report(void);

/**
 * Clears the remainder of the managers
 **/
void ers_final(void);
#endif // HERCULES_CORE

#endif /* DISABLE_ERS / not DISABLE_ERS */

#endif /* COMMON_ERS_H */
