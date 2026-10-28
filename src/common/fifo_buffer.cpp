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

#define HERCULES_CORE

#include "common/nullpo.h"
#include "common/showmsg.h"

#include <cstring>

#include "fifo_buffer.h"

fifo_buffer::fifo_buffer(size_t base_size) noexcept
{
	resize(base_size);
}

size_t fifo_buffer::capacity() const noexcept
{
	return m_buffer.size();
}

size_t fifo_buffer::free_space() const noexcept
{
	return m_buffer.size() - m_write_cursor;
}

size_t fifo_buffer::read_cursor() const noexcept
{
	return m_read_cursor;
}

size_t fifo_buffer::write_cursor() const noexcept
{
	return m_write_cursor;
}

bool fifo_buffer::empty() const noexcept
{
	return m_buffer.empty();
}

size_t fifo_buffer::readable() const noexcept
{
  return m_write_cursor - m_read_cursor;
}

void fifo_buffer::advance_write(size_t n) noexcept
{
	Assert_chk(m_write_cursor + n <= m_buffer.size());

	m_write_cursor += n;
}

void fifo_buffer::advance_read(size_t n) noexcept
{
	Assert_chk(m_read_cursor + n <= m_write_cursor);

	m_read_cursor += n;
}

void fifo_buffer::resize(size_t size) noexcept
{
	if (size < m_buffer.size()) {
		m_buffer.resize(size);
		m_buffer.shrink_to_fit();

		if (m_read_cursor > m_buffer.size())
			m_read_cursor = m_buffer.size();

		if (m_write_cursor > m_buffer.size())
			m_write_cursor = m_buffer.size();
	} else {
		m_buffer.resize(size, 0x0);
	}
}

void fifo_buffer::truncate() noexcept
{
	m_read_cursor  = 0;
	m_write_cursor = 0;
}

void fifo_buffer::erase(size_t n) noexcept
{
	Assert_chk(n <= m_write_cursor);

	if (n > m_write_cursor)
		n = m_write_cursor;

	if (n == m_write_cursor) {
		truncate();
		return;
	}

	memmove(m_buffer.data(), m_buffer.data() + n, m_write_cursor - n);
	m_write_cursor -= n;
	if (m_read_cursor >= n)
		m_read_cursor -= n;
	else
		m_read_cursor = 0;
}

uint8_t *fifo_buffer::data() noexcept
{
	return m_buffer.data();
}

void fifo_buffer::flush() noexcept
{
	if (m_read_cursor < m_write_cursor) {
		memmove(m_buffer.data(), m_buffer.data() + m_read_cursor, m_write_cursor - m_read_cursor);
		m_write_cursor = m_write_cursor - m_read_cursor;
		m_read_cursor  = 0;
    return;
	}

  truncate();
}
