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

#include "common/nullpo.h"
#include "common/showmsg.h"

#include <array>
#include <deque>
#include <forward_list>
#include <memory>
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

constexpr size_t ers_chunk_size = 2048;

enum ERSOptions {
	ERS_OPT_NONE        = 0x00,
	ERS_OPT_CLEAR       = 0x01,/* silently clears any entries left in the manager upon destruction */
	ERS_OPT_WAIT        = 0x02,/* wait for entries to come in order to list! */
	// ERS_OPT_FREE_NAME   = 0x04,/* name is dynamic memory, and should be freed */ // UNUSED
	ERS_OPT_CLEAN       = 0x08,/* clears used memory upon ers_free so that its all new to be reused on the next alloc */
	// ERS_OPT_FLEX_CHUNK  = 0x10,/* signs that it should look for its own cache given it'll have a dynamic chunk size, so that it doesn't affect the other ERS it'd otherwise be sharing */ // UNUSED
};

/**
 * A common interface for ERS
 */
class ERI
{
  public:
	explicit ERI() {
		m_ers_instance_list.push_front(this);
	}

	virtual ~ERI() {
		m_ers_instance_list.remove(this);
	}

	/**
	 * Reports debug information for instance cache
	 * @return used blocks, total blocks, memory used, total memory
	 */
	[[nodiscard]] virtual std::tuple<size_t, size_t, size_t, size_t>
	        print_report_cache() const noexcept = 0;

#ifdef DEBUG
	/**
	 * Reports debug information for current instance.
	 * @return true if reports is displayed, false otherwise.
	 */
	[[nodiscard]] virtual bool print_report() const noexcept = 0;
#endif

	/**
	 * Print a report about the current state of the Entry Reusage System.
	 * Shows information about the global system and each entry manager.
	 * The number of entries are checked and a warning is shown if extra reusable
	 * entries are found.
	 * The extra entries are included in the count of reusable entries.
	 */
	static void print_full_report() noexcept;

	/**
	 * Destructs all allocated instances that remained in memory
	 */
	static void final() noexcept;

  private:
	static std::forward_list<ERI *> m_ers_instance_list; //< A list holding all allocations of ERI interface
};

/**
 * Manages an ERS chunk
 */
template<typename T, size_t chunk_size>
class ers_chunk
{
	static_assert(chunk_size > 0, "chunk size has to be greater than 0");

  public:
	ers_chunk() = default;
	ers_chunk(ers_chunk &&other)                 = default;
	ers_chunk &operator=(ers_chunk &&other)      = default;
	ers_chunk(const ers_chunk &other)            = delete;
	ers_chunk &operator=(const ers_chunk &other) = delete;

	/**
	 * Allocates a new object in chunk
	 * @return a pointer of the type T or nullptr if chunk is full
	 */
	template<typename... Args>
	[[nodiscard]] T *alloc(Args &&...args) noexcept
	{
		if (used_memory() >= m_chunk.size())
			return nullptr;

		T *ptr = reinterpret_cast<T *>(&m_chunk[used_memory()]);
		new (ptr) T(std::forward<Args>(args)...);

		++m_used_objects;

		return ptr;
	}

	/**
	 * @return Gives current maximum capacity
	 */
	[[nodiscard]] size_t capacity() const noexcept
	{
		return m_chunk.size();
	}

	/**
	 * @return Gives currently used objects
	 */
	[[nodiscard]] size_t used_objects() const noexcept
	{
		return m_used_objects;
	}

	/**
	 * @return Used memory
	 */
	[[nodiscard]] size_t used_memory() const noexcept
	{
		return used_objects() * sizeof(T);
	}

	/**
	 * @return Unused blocks
	 */
	[[nodiscard]] size_t unused_blocks() const noexcept
	{
		return (capacity() - used_memory()) / sizeof(T);
	}

  private:
	std::array<std::byte, sizeof(T) * chunk_size> m_chunk{}; // The memory chunk used
	size_t m_used_objects{0}; // Used objects in chunk
};

/**
 * Public interface of the entry manager.
 */
template<typename T, size_t chunk_size = ers_chunk_size>
class ERS final : public ERI
{
  public:
	/**
	 * Get a new instance of the manager that handles the specified entry type.
	 * @param name the name of this ERS manager instance
	 * @param options a bitmask options of this instance manager
	 */
	ERS(const std::string &name, enum ERSOptions options) noexcept
	        : ERI(), m_name(name), m_options(options) {}; // FIXME: change this to a flag type

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
	}

	/**
	 * Allocate an entry from this entry manager.
	 * If there are reusable entries available, it reuses one instead.
	 * terminates the process if OOM happens
	 * @param args arguments passed to ctor
	 * @return An entry
	 */
	template<typename... Args>
	[[nodiscard]] T *alloc(Args &&...args) noexcept
	{
		T *ret = alloc_sub(std::forward<Args>(args)...);

		if (Assert_chk(ret != nullptr))
			std::terminate();

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
	}

	/**
	 * Reports debug information for instance cache
	 * @return used blocks, total blocks, memory used, total memory
	 */
	[[nodiscard]] std::tuple<size_t, size_t, size_t, size_t>
	        print_report_cache() const noexcept override
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
	[[nodiscard]] bool print_report() const noexcept override
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
	[[nodiscard]] size_t unused_blocks() const noexcept
	{
		if (m_cache.chunks.empty())
			return 0;

		return m_cache.chunks.back()->unused_blocks();
	}

	/**
	 * Returns currently allocated objects
	 * @return count of objects that are actually allocated for us
	 */
	[[nodiscard]] size_t used_objects() const noexcept
	{
		if (m_cache.chunks.empty())
			return 0;

		return (m_cache.chunks.size() * chunk_size) - unused_blocks() - m_cache.reuse_list.size();
	}

	/**
	 * Tries to allocate an object from already allocated memory then create a new chunk
	 * @param args arguments passed to ctor
	 * @return a pointer if successful otherwise nullptr
	 */
	template<typename... Args>
	[[nodiscard]] T *alloc_sub(Args &&...args) noexcept
	{
		if (m_cache.reuse_list.empty() == false) {
			auto *ret = m_cache.reuse_list.front();
			m_cache.reuse_list.pop_front();

			new (ret) T(std::forward<Args>(args)...);
			return ret;
		}

		if (m_cache.chunks.empty() == false) {
			auto chunk_ptr = m_cache.chunks.back()->alloc(std::forward<Args>(args)...);
			if (chunk_ptr != nullptr)
				return chunk_ptr;
		}

		m_cache.chunks.push_back(std::make_unique<ers_chunk<T, chunk_size>>());
		return m_cache.chunks.back()->alloc(std::forward<Args>(args)...); // Should never fail unless OOM
	}

	std::string m_name; //< Name, used for debugging purposes

	ERSOptions m_options {
		ERS_OPT_NONE
	}; //< Misc options

#ifdef DEBUG
	/* for data analysis [Ind/Hercules] */
	size_t m_peak{0};
#endif

	struct {
		std::deque<T *> reuse_list; //< Reuse linked list
		std::vector<std::unique_ptr<ers_chunk<T, chunk_size>>> chunks; //< Memory blocks array
	} m_cache;
};

#ifdef DISABLE_ERS
// Use memory manager to allocate/free and disable other interface functions
#	define ers_alloc(obj,type) ((void)(obj), (type *)aMalloc(sizeof(type)))
#	define ers_free(obj,entry) ((void)(obj), aFree(entry))
#	define ers_entry_size(obj) ((void)(obj), (size_t)0)
#	define ers_destroy(obj) ((void)(obj), (void)0)
// Disable the public functions
#	define ers_new(type,name,options) nullptr
#	define ers_report() (void)0
#	define ers_final() (void)0
#else /* not DISABLE_ERS */
// These defines should be used to allow the code to keep working whenever
// the system is disabled
#	define ers_new(type,name,options) (new ERS<type, ers_chunk_size>((name), (options)))
#	define ers_new2(type,name,options,chunk_size) (new ERS<type, (chunk_size)>((name), (options)))
#	define ers_alloc(obj, ...) ((obj)->alloc(##__VA_ARGS__))
#	define ers_free(obj,entry) ((obj)->free((entry)))
#	define ers_entry_size(obj) ((obj)->entry_size())
#	define ers_destroy(obj)    (delete (obj))

#endif /* DISABLE_ERS / not DISABLE_ERS */

#endif /* COMMON_ERS_H */
