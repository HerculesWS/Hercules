/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2026 Hercules Dev Team
 * Copyright (C) 2026 hemagx
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

#ifndef COMMON_FIFO_BUFFER_H
#define COMMON_FIFO_BUFFER_H

#include <cstdint>
#include <vector>

/**
 * A class that manages a FIFO buffer, specifically designed for the use
 * with our sockets library
 */
class fifo_buffer
{
  public:
	fifo_buffer()  = default;
	~fifo_buffer() = default;

	/**
	 * Constructs fifo_buffer
	 * @param base_size the base size that will be allocated for the internal buffer
	 */
	explicit fifo_buffer(size_t base_size) noexcept;

	/**
	 * Allocates a new fifo buffer class
	 * To be used temporarily until everything stored in smart pointers
	 */
	template<typename... Args>
	[[nodiscard]] static fifo_buffer *create(Args &&...args)
	{
		return new fifo_buffer(std::forward<Args>(args)...);
	}

	/**
	 * Destroys a fifo buffer class
	 * To be used temporarily until everything stored in smart pointers
	 */
	static void destroy(fifo_buffer *self)
	{
		delete self;
	}

	/**
	 * @return Current maximum capacity
	 */
	[[nodiscard]] size_t capacity() const noexcept;

	/**
	 * @return Current free space in the buffer
	 */
	[[nodiscard]] size_t free_space() const noexcept;

	/**
	 * @return Current read cursor position
	 */
	[[nodiscard]] size_t read_cursor() const noexcept;

	/**
	 * @return Current write cursor position
	 */
	[[nodiscard]] size_t write_cursor() const noexcept;

	/**
	 * Returns the empty status of the buffer, it's empty if no buffer was
	 * allocated and/or write cursor is at 0
	 * @return empty status
	 */
	[[nodiscard]] bool empty() const noexcept;

    /**
     * Gives you how much left in the buffer to read
     * @return the amount of bytes available to read
     */
    [[nodiscard]] size_t readable() const noexcept;

	/**
	 * Advances the write position cursor
	 * @param n the amount by which the cursor is advanced
	 */
	void advance_write(size_t n) noexcept;

	/**
	 * Advances the read position cursor
	 * @param n the amount by which the cursor is advanced
	 */
	void advance_read(size_t n) noexcept;

	/**
	 * Resizes the current buffer maximum size
	 * if the new size is bigger than either read or write cursors it's
	 * reset to maximum of current buffer
	 * @param size the new maximum size of the buffer
	 */
	void resize(size_t size) noexcept;

	/**
	 * Truncates the current buffer without doing any changes to the buffer or memory allocation
	 */
	void truncate() noexcept;

	/**
	 * Erases n elements from the top of the buffer
	 * @param n the count of elements to remove
	 */
	void erase(size_t n) noexcept;

	/**
	 * Returns a pointer to the beginning of the internal buffer
	 * @return pointer to internal buffer
	 */
	[[nodiscard]] uint8_t *data() noexcept;

	/**
	 * Flushes all data that's been read from the buffer and resets
	 * the cursor to the appropriate position
	 */
	void flush() noexcept;

	/**
	 * Returns a pointer of the appropriate type at the requested position from
	 * current read cursor
	 * @return requested pointer
	 */
	template<typename T>
	[[nodiscard]] const std::remove_pointer_t<T> *output_ptr(size_t pos) noexcept
	{
		return reinterpret_cast<const T>(m_buffer.data() + m_read_cursor + pos);
	}

	/**
	 * Returns a pointer of the appropriate type at the requested position from
	 * current write cursor
	 * @return requested pointer
	 */
	template<typename T>
	[[nodiscard]] T input_ptr(size_t pos) noexcept
	{
		return reinterpret_cast<T>(m_buffer.data() + m_write_cursor + pos);
	}

  private:
	std::vector<uint8_t> m_buffer{};
	size_t m_write_cursor{0};
	size_t m_read_cursor{0};
};

#endif
